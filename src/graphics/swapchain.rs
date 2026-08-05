use ash::vk;
use super::buffer::find_memory_type;
use super::vk_context::VkContext;

#[allow(dead_code)]
pub struct Swapchain {
    pub loader: ash::khr::swapchain::Device,
    pub handle: vk::SwapchainKHR,
    pub format: vk::SurfaceFormatKHR,
    pub extent: vk::Extent2D,
    pub images: Vec<vk::Image>,
    pub image_views: Vec<vk::ImageView>,
    pub depth_image: vk::Image,
    pub depth_image_memory: vk::DeviceMemory,
    pub depth_image_view: vk::ImageView,
    pub depth_format: vk::Format,
    pub render_pass: vk::RenderPass,
    pub framebuffers: Vec<vk::Framebuffer>,
}

impl Swapchain {
    pub fn new(vk_ctx: &VkContext, width: u32, height: u32) -> Self {
        let loader = ash::khr::swapchain::Device::new(&vk_ctx.instance, &vk_ctx.device);

        let capabilities = unsafe {
            vk_ctx
                .surface_loader
                .get_physical_device_surface_capabilities(vk_ctx.physical_device, vk_ctx.surface)
                .unwrap()
        };
        let formats = unsafe {
            vk_ctx
                .surface_loader
                .get_physical_device_surface_formats(vk_ctx.physical_device, vk_ctx.surface)
                .unwrap()
        };
        let format = formats[0];
        let extent = vk::Extent2D { width, height };

        // 1. Swapchain Handle
        let info = vk::SwapchainCreateInfoKHR::default()
            .surface(vk_ctx.surface)
            .min_image_count(2.max(capabilities.min_image_count))
            .image_format(format.format)
            .image_color_space(format.color_space)
            .image_extent(extent)
            .image_array_layers(1)
            .image_usage(vk::ImageUsageFlags::COLOR_ATTACHMENT)
            .image_sharing_mode(vk::SharingMode::EXCLUSIVE)
            .pre_transform(capabilities.current_transform)
            .composite_alpha(vk::CompositeAlphaFlagsKHR::OPAQUE)
            .present_mode(vk::PresentModeKHR::FIFO)
            .clipped(true);

        let handle = unsafe { loader.create_swapchain(&info, None).unwrap() };
        let images = unsafe { loader.get_swapchain_images(handle).unwrap() };

        // 2. Image Views
        let image_views: Vec<vk::ImageView> = images
            .iter()
            .map(|&image| {
                let view_info = vk::ImageViewCreateInfo::default()
                    .image(image)
                    .view_type(vk::ImageViewType::TYPE_2D)
                    .format(format.format)
                    .subresource_range(vk::ImageSubresourceRange {
                        aspect_mask: vk::ImageAspectFlags::COLOR,
                        base_mip_level: 0,
                        level_count: 1,
                        base_array_layer: 0,
                        layer_count: 1,
                    });
                unsafe { vk_ctx.device.create_image_view(&view_info, None).unwrap() }
            })
            .collect();

        // 3. Depth Buffer (Image, Memory, View)
        let depth_format = vk::Format::D32_SFLOAT;

        let depth_image_info = vk::ImageCreateInfo::default()
            .image_type(vk::ImageType::TYPE_2D)
            .extent(vk::Extent3D {
                width,
                height,
                depth: 1,
            })
            .mip_levels(1)
            .array_layers(1)
            .format(depth_format)
            .tiling(vk::ImageTiling::OPTIMAL)
            .initial_layout(vk::ImageLayout::UNDEFINED)
            .usage(vk::ImageUsageFlags::DEPTH_STENCIL_ATTACHMENT)
            .samples(vk::SampleCountFlags::TYPE_1)
            .sharing_mode(vk::SharingMode::EXCLUSIVE);

        let depth_image = unsafe {
            vk_ctx
                .device
                .create_image(&depth_image_info, None)
                .expect("Failed to create depth image")
        };

        let mem_requirements = unsafe { vk_ctx.device.get_image_memory_requirements(depth_image) };
        let memory_type = find_memory_type(
            vk_ctx,
            mem_requirements.memory_type_bits,
            vk::MemoryPropertyFlags::DEVICE_LOCAL,
        );

        let alloc_info = vk::MemoryAllocateInfo::default()
            .allocation_size(mem_requirements.size)
            .memory_type_index(memory_type);

        let depth_image_memory = unsafe {
            vk_ctx
                .device
                .allocate_memory(&alloc_info, None)
                .expect("Failed to allocate depth image memory")
        };

        unsafe {
            vk_ctx
                .device
                .bind_image_memory(depth_image, depth_image_memory, 0)
                .expect("Failed to bind depth image memory");
        }

        let depth_view_info = vk::ImageViewCreateInfo::default()
            .image(depth_image)
            .view_type(vk::ImageViewType::TYPE_2D)
            .format(depth_format)
            .subresource_range(vk::ImageSubresourceRange {
                aspect_mask: vk::ImageAspectFlags::DEPTH,
                base_mip_level: 0,
                level_count: 1,
                base_array_layer: 0,
                layer_count: 1,
            });

        let depth_image_view = unsafe {
            vk_ctx
                .device
                .create_image_view(&depth_view_info, None)
                .expect("Failed to create depth image view")
        };

        // 4. Render Pass
        let color_attachment = vk::AttachmentDescription::default()
            .format(format.format)
            .samples(vk::SampleCountFlags::TYPE_1)
            .load_op(vk::AttachmentLoadOp::CLEAR)
            .store_op(vk::AttachmentStoreOp::STORE)
            .initial_layout(vk::ImageLayout::UNDEFINED)
            .final_layout(vk::ImageLayout::PRESENT_SRC_KHR);

        let color_attachment_ref = vk::AttachmentReference::default()
            .attachment(0)
            .layout(vk::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

        let depth_attachment = vk::AttachmentDescription::default()
            .format(depth_format)
            .samples(vk::SampleCountFlags::TYPE_1)
            .load_op(vk::AttachmentLoadOp::CLEAR)
            .store_op(vk::AttachmentStoreOp::DONT_CARE)
            .stencil_load_op(vk::AttachmentLoadOp::DONT_CARE)
            .stencil_store_op(vk::AttachmentStoreOp::DONT_CARE)
            .initial_layout(vk::ImageLayout::UNDEFINED)
            .final_layout(vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        let depth_attachment_ref = vk::AttachmentReference::default()
            .attachment(1)
            .layout(vk::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

        let subpass = vk::SubpassDescription::default()
            .pipeline_bind_point(vk::PipelineBindPoint::GRAPHICS)
            .color_attachments(std::slice::from_ref(&color_attachment_ref))
            .depth_stencil_attachment(&depth_attachment_ref);

        let subpass_dependency = vk::SubpassDependency::default()
            .src_subpass(vk::SUBPASS_EXTERNAL)
            .dst_subpass(0)
            .src_stage_mask(
                vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT
                    | vk::PipelineStageFlags::EARLY_FRAGMENT_TESTS,
            )
            .src_access_mask(vk::AccessFlags::empty())
            .dst_stage_mask(
                vk::PipelineStageFlags::COLOR_ATTACHMENT_OUTPUT
                    | vk::PipelineStageFlags::EARLY_FRAGMENT_TESTS,
            )
            .dst_access_mask(
                vk::AccessFlags::COLOR_ATTACHMENT_WRITE
                    | vk::AccessFlags::DEPTH_STENCIL_ATTACHMENT_WRITE,
            );

        let attachments = [color_attachment, depth_attachment];
        let render_pass_info = vk::RenderPassCreateInfo::default()
            .attachments(&attachments)
            .subpasses(std::slice::from_ref(&subpass))
            .dependencies(std::slice::from_ref(&subpass_dependency));

        let render_pass =
            unsafe { vk_ctx.device.create_render_pass(&render_pass_info, None).unwrap() };

        // 5. Framebuffers
        let framebuffers: Vec<vk::Framebuffer> = image_views
            .iter()
            .map(|&view| {
                let framebuffer_attachments = [view, depth_image_view];
                let framebuffer_info = vk::FramebufferCreateInfo::default()
                    .render_pass(render_pass)
                    .attachments(&framebuffer_attachments)
                    .width(width)
                    .height(height)
                    .layers(1);
                unsafe { vk_ctx.device.create_framebuffer(&framebuffer_info, None).unwrap() }
            })
            .collect();

        Self {
            loader,
            handle,
            format,
            extent,
            images,
            image_views,
            depth_image,
            depth_image_memory,
            depth_image_view,
            depth_format,
            render_pass,
            framebuffers,
        }
    }

    #[allow(dead_code)]
    pub fn destroy(&self, vk_ctx: &VkContext) {
        unsafe {
            for &framebuffer in &self.framebuffers {
                vk_ctx.device.destroy_framebuffer(framebuffer, None);
            }
            vk_ctx.device.destroy_render_pass(self.render_pass, None);

            vk_ctx.device.destroy_image_view(self.depth_image_view, None);
            vk_ctx.device.destroy_image(self.depth_image, None);
            vk_ctx.device.free_memory(self.depth_image_memory, None);

            for &image_view in &self.image_views {
                vk_ctx.device.destroy_image_view(image_view, None);
            }
            self.loader.destroy_swapchain(self.handle, None);
        }
    }
}