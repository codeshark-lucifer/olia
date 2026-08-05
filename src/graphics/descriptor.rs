use ash::vk;
use super::buffer::Buffer;
use super::vk_context::VkContext;

#[allow(dead_code)]
pub struct DescriptorSystem {
    pub layout: vk::DescriptorSetLayout,
    pub pool: vk::DescriptorPool,
    pub sets: Vec<vk::DescriptorSet>,
}

#[allow(dead_code)]
impl DescriptorSystem {
    pub fn new(
        vk_ctx: &VkContext,
        bindings: &[vk::DescriptorSetLayoutBinding],
        max_sets: u32,
        pool_sizes: &[vk::DescriptorPoolSize],
    ) -> Self {
        // 1. Create Descriptor Set Layout
        let layout_info = vk::DescriptorSetLayoutCreateInfo::default().bindings(bindings);
        let layout = unsafe {
            vk_ctx
                .device
                .create_descriptor_set_layout(&layout_info, None)
                .expect("Failed to create descriptor set layout")
        };

        // 2. Create Descriptor Pool
        let pool_info = vk::DescriptorPoolCreateInfo::default()
            .pool_sizes(pool_sizes)
            .max_sets(max_sets);

        let pool = unsafe {
            vk_ctx
                .device
                .create_descriptor_pool(&pool_info, None)
                .expect("Failed to create descriptor pool")
        };

        // 3. Allocate Descriptor Sets
        let layouts = vec![layout; max_sets as usize];
        let alloc_info = vk::DescriptorSetAllocateInfo::default()
            .descriptor_pool(pool)
            .set_layouts(&layouts);

        let sets = unsafe {
            vk_ctx
                .device
                .allocate_descriptor_sets(&alloc_info)
                .expect("Failed to allocate descriptor sets")
        };

        Self { layout, pool, sets }
    }

    /// Connects a UBO or SSBO buffer to a descriptor binding slot
    pub fn update_buffer_descriptor(
        &self,
        vk_ctx: &VkContext,
        set_index: usize,
        binding: u32,
        buffer: &Buffer,
        descriptor_type: vk::DescriptorType,
    ) {
        let buffer_info = [vk::DescriptorBufferInfo {
            buffer: buffer.handle,
            offset: 0,
            range: buffer.size,
        }];

        let write = [vk::WriteDescriptorSet::default()
            .dst_set(self.sets[set_index])
            .dst_binding(binding)
            .descriptor_type(descriptor_type)
            .buffer_info(&buffer_info)];

        unsafe {
            vk_ctx.device.update_descriptor_sets(&write, &[]);
        }
    }

    pub fn destroy(&self, vk_ctx: &VkContext) {
        unsafe {
            vk_ctx.device.destroy_descriptor_pool(self.pool, None);
            vk_ctx
                .device
                .destroy_descriptor_set_layout(self.layout, None);
        }
    }
}