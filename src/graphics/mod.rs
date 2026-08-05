pub mod buffer;
pub mod descriptor;
pub mod pipeline;
pub mod swapchain;
pub mod types;
pub mod vk_context;

pub use buffer::*;
pub use descriptor::*;
pub use pipeline::*;
pub use swapchain::*;
pub use types::*;
pub use vk_context::*;

use ash::vk;
use crate::components::{Camera, Transform};
use crate::core::ecs::World;
use crate::utils::mathf::{Mat4, Vec3, Vec4};

#[allow(dead_code)]
pub struct Renderer {
    pub vk_ctx: VkContext,
    pub swapchain: Swapchain,
    pub pipeline: Pipeline,
    pub descriptors: DescriptorSystem,

    pub uniform_buffers: Vec<Buffer>,
    pub storage_buffers: Vec<Buffer>,
    pub vertex_buffer: Buffer,
    pub vertex_count: u32,

    pub command_pool: vk::CommandPool,
    pub command_buffer: vk::CommandBuffer,

    pub image_available_semaphore: vk::Semaphore,
    pub render_finished_semaphore: vk::Semaphore,
    pub in_flight_fence: vk::Fence,
}

impl Renderer {
    pub fn new(window: &sdl3::video::Window, width: u32, height: u32) -> Self {
        let vk_ctx = VkContext::new(window);
        let swapchain = Swapchain::new(&vk_ctx, width, height);

        let frame_count = swapchain.images.len() as u32;

        let ubo_binding = vk::DescriptorSetLayoutBinding::default()
            .binding(0)
            .descriptor_type(vk::DescriptorType::UNIFORM_BUFFER)
            .descriptor_count(1)
            .stage_flags(vk::ShaderStageFlags::VERTEX | vk::ShaderStageFlags::FRAGMENT);

        let ssbo_binding = vk::DescriptorSetLayoutBinding::default()
            .binding(1)
            .descriptor_type(vk::DescriptorType::STORAGE_BUFFER)
            .descriptor_count(1)
            .stage_flags(vk::ShaderStageFlags::VERTEX | vk::ShaderStageFlags::FRAGMENT);

        let layout_bindings = [ubo_binding, ssbo_binding];

        let pool_sizes = [
            vk::DescriptorPoolSize {
                ty: vk::DescriptorType::UNIFORM_BUFFER,
                descriptor_count: frame_count,
            },
            vk::DescriptorPoolSize {
                ty: vk::DescriptorType::STORAGE_BUFFER,
                descriptor_count: frame_count,
            },
        ];

        let descriptors =
            DescriptorSystem::new(&vk_ctx, &layout_bindings, frame_count, &pool_sizes);

        let push_constant_range = vk::PushConstantRange::default()
            .stage_flags(vk::ShaderStageFlags::VERTEX | vk::ShaderStageFlags::FRAGMENT)
            .offset(0)
            .size(std::mem::size_of::<PushConstants>() as u32);

        let vert_spv = include_bytes!("../../assets/shaders/compiled/shader.vert.spv");
        let frag_spv = include_bytes!("../../assets/shaders/compiled/shader.frag.spv");

        let binding_descriptions = [Vertex::get_binding_description()];
        let attribute_descriptions = Vertex::get_attribute_descriptions();

        let pipeline = Pipeline::new(
            &vk_ctx.device,
            swapchain.render_pass,
            vert_spv,
            frag_spv,
            &binding_descriptions,
            &attribute_descriptions,
            &[descriptors.layout],
            &[push_constant_range],
        );

        let ubo_size = std::mem::size_of::<CameraUbo>() as vk::DeviceSize;
        let ssbo_size = (std::mem::size_of::<ObjectDataSsbo>() * 100) as vk::DeviceSize;

        let mut uniform_buffers = Vec::new();
        let mut storage_buffers = Vec::new();

        for i in 0..frame_count as usize {
            let ubo = Buffer::new(
                &vk_ctx,
                ubo_size,
                vk::BufferUsageFlags::UNIFORM_BUFFER,
                vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT,
            );
            descriptors.update_buffer_descriptor(
                &vk_ctx,
                i,
                0,
                &ubo,
                vk::DescriptorType::UNIFORM_BUFFER,
            );
            uniform_buffers.push(ubo);

            let ssbo = Buffer::new(
                &vk_ctx,
                ssbo_size,
                vk::BufferUsageFlags::STORAGE_BUFFER,
                vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT,
            );
            descriptors.update_buffer_descriptor(
                &vk_ctx,
                i,
                1,
                &ssbo,
                vk::DescriptorType::STORAGE_BUFFER,
            );
            storage_buffers.push(ssbo);
        }

        let initial_vertex_capacity = (1024 * std::mem::size_of::<Vertex>()) as vk::DeviceSize;
        let vertex_buffer = Buffer::new(
            &vk_ctx,
            initial_vertex_capacity,
            vk::BufferUsageFlags::VERTEX_BUFFER,
            vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT,
        );

        let command_pool = unsafe {
            let pool_info = vk::CommandPoolCreateInfo::default()
                .queue_family_index(vk_ctx.queue_family_index)
                .flags(vk::CommandPoolCreateFlags::RESET_COMMAND_BUFFER);
            vk_ctx.device.create_command_pool(&pool_info, None).unwrap()
        };

        let command_buffer = unsafe {
            let alloc_info = vk::CommandBufferAllocateInfo::default()
                .command_pool(command_pool)
                .level(vk::CommandBufferLevel::PRIMARY)
                .command_buffer_count(1);
            vk_ctx.device.allocate_command_buffers(&alloc_info).unwrap()[0]
        };

        let semaphore_info = vk::SemaphoreCreateInfo::default();
        let fence_info = vk::FenceCreateInfo::default().flags(vk::FenceCreateFlags::SIGNALED);

        let image_available_semaphore = unsafe {
            vk_ctx
                .device
                .create_semaphore(&semaphore_info, None)
                .unwrap()
        };
        let render_finished_semaphore = unsafe {
            vk_ctx
                .device
                .create_semaphore(&semaphore_info, None)
                .unwrap()
        };
        let in_flight_fence = unsafe { vk_ctx.device.create_fence(&fence_info, None).unwrap() };

        Self {
            vk_ctx,
            swapchain,
            pipeline,
            descriptors,
            uniform_buffers,
            storage_buffers,
            vertex_buffer,
            vertex_count: 0,
            command_pool,
            command_buffer,
            image_available_semaphore,
            render_finished_semaphore,
            in_flight_fence,
        }
    }

    pub fn update_vertices(&mut self, vertices: &[Vertex]) {
        if vertices.is_empty() {
            self.vertex_count = 0;
            return;
        }

        let required_size = (vertices.len() * std::mem::size_of::<Vertex>()) as vk::DeviceSize;
        if required_size > self.vertex_buffer.size {
            self.vertex_buffer.destroy(&self.vk_ctx);
            self.vertex_buffer = Buffer::new(
                &self.vk_ctx,
                required_size,
                vk::BufferUsageFlags::VERTEX_BUFFER,
                vk::MemoryPropertyFlags::HOST_VISIBLE | vk::MemoryPropertyFlags::HOST_COHERENT,
            );
        }

        self.vertex_buffer.update_data(&self.vk_ctx, vertices);
        self.vertex_count = vertices.len() as u32;
    }

    pub fn draw_frame(&mut self, world: &World, width: u32, height: u32) {
        unsafe {
            self.vk_ctx
                .device
                .wait_for_fences(&[self.in_flight_fence], true, u64::MAX)
                .unwrap();
            self.vk_ctx
                .device
                .reset_fences(&[self.in_flight_fence])
                .unwrap();

            let (image_index, _) = self
                .swapchain
                .loader
                .acquire_next_image(
                    self.swapchain.handle,
                    u64::MAX,
                    self.image_available_semaphore,
                    vk::Fence::null(),
                )
                .unwrap();

            let img_idx = image_index as usize;

            // 1. QUERY CAMERA MATRICES FROM ECS WORLD
            let (view_matrix, proj_matrix) = if let (Some(cameras), Some(transforms)) =
                (world.pool::<Camera>(), world.pool::<Transform>())
            {
                let mut found_matrices = None;
                for (entity, camera) in cameras.iter() {
                    if let Some(transform) = transforms.get(*entity) {
                        found_matrices = Some(camera.get_matrices(transform));
                        break;
                    }
                }
                found_matrices.unwrap_or_else(|| {
                    let aspect = width as f32 / height as f32;
                    (
                        Mat4::look_at(Vec3::new(0.0, 0.0, 3.0), Vec3::ZERO, Vec3::UP),
                        Mat4::perspective(45.0f32.to_radians(), aspect, 0.1, 100.0),
                    )
                })
            } else {
                let aspect = width as f32 / height as f32;
                (
                    Mat4::look_at(Vec3::new(0.0, 0.0, 3.0), Vec3::ZERO, Vec3::UP),
                    Mat4::perspective(45.0f32.to_radians(), aspect, 0.1, 100.0),
                )
            };

            let camera_data = CameraUbo {
                view: view_matrix,
                projection: proj_matrix,
            };
            self.uniform_buffers[img_idx].update_data(&self.vk_ctx, &[camera_data]);

            // 2. QUERY OBJECT MODEL MATRICES FROM ECS WORLD
            let mut objects_data = Vec::new();
            let mut primary_model_matrix = Mat4::identity();

            if let (Some(transforms), Some(cameras)) =
                (world.pool::<Transform>(), world.pool::<Camera>())
            {
                for (entity, transform) in transforms.iter() {
                    if cameras.get(*entity).is_some() {
                        continue;
                    }
                    let model_matrix = transform.get_matrix();
                    if objects_data.is_empty() {
                        primary_model_matrix = model_matrix;
                    }
                    objects_data.push(ObjectDataSsbo {
                        world_matrix: model_matrix,
                        color_override: Vec4::new(1.0, 1.0, 1.0, 1.0),
                    });
                }
            }

            if objects_data.is_empty() {
                objects_data.push(ObjectDataSsbo {
                    world_matrix: Mat4::identity(),
                    color_override: Vec4::new(1.0, 1.0, 1.0, 1.0),
                });
            }

            self.storage_buffers[img_idx].update_data(&self.vk_ctx, &objects_data);

            // 3. RECORD COMMANDS
            self.vk_ctx
                .device
                .reset_command_buffer(self.command_buffer, vk::CommandBufferResetFlags::empty())
                .unwrap();

            let begin_info = vk::CommandBufferBeginInfo::default();
            self.vk_ctx
                .device
                .begin_command_buffer(self.command_buffer, &begin_info)
                .unwrap();

            let clear_values = [
                vk::ClearValue {
                    color: vk::ClearColorValue {
                        float32: [0.1, 0.1, 0.1, 1.0],
                    },
                },
                vk::ClearValue {
                    depth_stencil: vk::ClearDepthStencilValue {
                        depth: 1.0,
                        stencil: 0,
                    },
                },
            ];

            let render_pass_info = vk::RenderPassBeginInfo::default()
                .render_pass(self.swapchain.render_pass)
                .framebuffer(self.swapchain.framebuffers[img_idx])
                .render_area(vk::Rect2D {
                    offset: vk::Offset2D { x: 0, y: 0 },
                    extent: vk::Extent2D { width, height },
                })
                .clear_values(&clear_values);

            self.vk_ctx.device.cmd_begin_render_pass(
                self.command_buffer,
                &render_pass_info,
                vk::SubpassContents::INLINE,
            );

            self.vk_ctx.device.cmd_bind_pipeline(
                self.command_buffer,
                vk::PipelineBindPoint::GRAPHICS,
                self.pipeline.handle,
            );

            // Flipped viewport height (Vulkan 1.1 / VK_KHR_maintenance1 standard)
            let viewport = vk::Viewport::default()
                .x(0.0)
                .y(height as f32)
                .width(width as f32)
                .height(-(height as f32))
                .min_depth(0.0)
                .max_depth(1.0);
            self.vk_ctx
                .device
                .cmd_set_viewport(self.command_buffer, 0, &[viewport]);

            let scissor = vk::Rect2D::default()
                .offset(vk::Offset2D { x: 0, y: 0 })
                .extent(vk::Extent2D { width, height });
            self.vk_ctx
                .device
                .cmd_set_scissor(self.command_buffer, 0, &[scissor]);

            self.vk_ctx.device.cmd_bind_descriptor_sets(
                self.command_buffer,
                vk::PipelineBindPoint::GRAPHICS,
                self.pipeline.layout,
                0,
                &[self.descriptors.sets[img_idx]],
                &[],
            );

            let push_data = PushConstants {
                model_matrix: primary_model_matrix,
                tint_color: Vec4::new(1.0, 1.0, 1.0, 1.0),
            };

            let push_bytes = std::slice::from_raw_parts(
                &push_data as *const _ as *const u8,
                std::mem::size_of::<PushConstants>(),
            );

            self.vk_ctx.device.cmd_push_constants(
                self.command_buffer,
                self.pipeline.layout,
                vk::ShaderStageFlags::VERTEX | vk::ShaderStageFlags::FRAGMENT,
                0,
                push_bytes,
            );

            if self.vertex_count > 0 {
                let vertex_buffers = [self.vertex_buffer.handle];
                let offsets = [0];
                self.vk_ctx.device.cmd_bind_vertex_buffers(
                    self.command_buffer,
                    0,
                    &vertex_buffers,
                    &offsets,
                );
                self.vk_ctx
                    .device
                    .cmd_draw(self.command_buffer, self.vertex_count, 1, 0, 0);
            } else {
                self.vk_ctx.device.cmd_draw(self.command_buffer, 3, 1, 0, 0);
            }

            self.vk_ctx.device.cmd_end_render_pass(self.command_buffer);
            self.vk_ctx
                .device
                .end_command_buffer(self.command_buffer)
                .unwrap();

            let wait_semaphores = [self.image_available_semaphore];
            let wait_stages = [vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT];
            let signal_semaphores = [self.render_finished_semaphore];
            let command_buffers = [self.command_buffer];

            let submit_info = vk::SubmitInfo::default()
                .wait_semaphores(&wait_semaphores)
                .wait_dst_stage_mask(&wait_stages)
                .command_buffers(&command_buffers)
                .signal_semaphores(&signal_semaphores);

            self.vk_ctx
                .device
                .queue_submit(self.vk_ctx.queue, &[submit_info], self.in_flight_fence)
                .unwrap();

            let swapchains = [self.swapchain.handle];
            let image_indices = [image_index];
            let present_info = vk::PresentInfoKHR::default()
                .wait_semaphores(&signal_semaphores)
                .swapchains(&swapchains)
                .image_indices(&image_indices);

            self.swapchain
                .loader
                .queue_present(self.vk_ctx.queue, &present_info)
                .unwrap();
        }
    }
}