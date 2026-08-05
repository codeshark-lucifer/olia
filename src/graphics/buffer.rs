use ash::vk;
use super::vk_context::VkContext;

pub struct Buffer {
    pub handle: vk::Buffer,
    pub memory: vk::DeviceMemory,
    pub size: vk::DeviceSize,
}

impl Buffer {
    pub fn new(
        vk_ctx: &VkContext,
        size: vk::DeviceSize,
        usage: vk::BufferUsageFlags,
        properties: vk::MemoryPropertyFlags,
    ) -> Self {
        let buffer_info = vk::BufferCreateInfo::default()
            .size(size)
            .usage(usage)
            .sharing_mode(vk::SharingMode::EXCLUSIVE);

        let handle = unsafe {
            vk_ctx
                .device
                .create_buffer(&buffer_info, None)
                .expect("Failed to create buffer")
        };

        let mem_requirements = unsafe { vk_ctx.device.get_buffer_memory_requirements(handle) };
        let memory_type = find_memory_type(
            vk_ctx,
            mem_requirements.memory_type_bits,
            properties,
        );

        let alloc_info = vk::MemoryAllocateInfo::default()
            .allocation_size(mem_requirements.size)
            .memory_type_index(memory_type);

        let memory = unsafe {
            vk_ctx
                .device
                .allocate_memory(&alloc_info, None)
                .expect("Failed to allocate buffer memory")
        };

        unsafe {
            vk_ctx
                .device
                .bind_buffer_memory(handle, memory, 0)
                .expect("Failed to bind buffer memory");
        }

        Self {
            handle,
            memory,
            size,
        }
    }

    /// Uploads CPU data into host-visible mapped memory
    pub fn update_data<T: Copy>(&self, vk_ctx: &VkContext, data: &[T]) {
        let data_size = (std::mem::size_of::<T>() * data.len()) as vk::DeviceSize;
        if data_size == 0 {
            return;
        }

        unsafe {
            let data_ptr = vk_ctx
                .device
                .map_memory(self.memory, 0, data_size, vk::MemoryMapFlags::empty())
                .expect("Failed to map buffer memory");
            std::ptr::copy_nonoverlapping(data.as_ptr(), data_ptr as *mut T, data.len());
            vk_ctx.device.unmap_memory(self.memory);
        }
    }

    pub fn destroy(&self, vk_ctx: &VkContext) {
        unsafe {
            vk_ctx.device.destroy_buffer(self.handle, None);
            vk_ctx.device.free_memory(self.memory, None);
        }
    }
}

/// Utility function to find compatible GPU memory type index
pub fn find_memory_type(
    vk_ctx: &VkContext,
    type_filter: u32,
    properties: vk::MemoryPropertyFlags,
) -> u32 {
    let mem_properties = unsafe {
        vk_ctx
            .instance
            .get_physical_device_memory_properties(vk_ctx.physical_device)
    };

    for i in 0..mem_properties.memory_type_count {
        if (type_filter & (1 << i)) != 0
            && (mem_properties.memory_types[i as usize].property_flags & properties) == properties
        {
            return i;
        }
    }

    panic!("Failed to find suitable memory type!");
}