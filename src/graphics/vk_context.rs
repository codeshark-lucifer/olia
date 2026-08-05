use ash::vk::{self, Handle};
use std::ffi::{CStr, CString};

pub struct VKContext {
    pub entry: ash::Entry,
    pub instance: ash::Instance,

    pub surface: vk::SurfaceKHR,
    pub surface_loader: ash::khr::surface::Instance,

    pub physical_device: vk::PhysicalDevice,
    pub device: ash::Device,

    pub queue: vk::Queue,
    pub queue_family_index: u32,
}

pub type VkContext = VKContext;

impl VKContext {
    pub fn new(window: &sdl3::video::Window) -> Self {
        let entry = unsafe { ash::Entry::load().expect("Failed to load Vulkan entry point") };

        // Instance Extensions
        let instance_extension = window.vulkan_instance_extensions().unwrap();
        let c_extensions: Vec<CString> = instance_extension
            .iter()
            .map(|ext| CString::new(ext.as_bytes()).unwrap())
            .collect();
        let extension_pointers: Vec<*const std::os::raw::c_char> =
            c_extensions.iter().map(|ext| ext.as_ptr()).collect();

        let app_info = vk::ApplicationInfo::default()
            .application_name(CStr::from_bytes_with_nul(b"Olia Engine\0").unwrap())
            .api_version(vk::API_VERSION_1_3);

        let instance_info = vk::InstanceCreateInfo::default()
            .application_info(&app_info)
            .enabled_extension_names(&extension_pointers);

        let instance = unsafe { entry.create_instance(&instance_info, None).unwrap() };

        // Surface
        let surface_handle = unsafe {
            window
                .vulkan_create_surface(instance.handle().as_raw() as _)
                .expect("Failed to create Vulkan surface from SDL3")
        };
        let surface = vk::SurfaceKHR::from_raw(surface_handle as u64);
        let surface_loader = ash::khr::surface::Instance::new(&entry, &instance);

        // Physical Device & Compatible Queue Family
        let physical_device = unsafe { instance.enumerate_physical_devices() }
            .unwrap()
            .into_iter()
            .next()
            .expect("No GPU found!");

        let queue_family_properties =
            unsafe { instance.get_physical_device_queue_family_properties(physical_device) };
        let mut queue_family_index = 0u32;
        for (i, props) in queue_family_properties.iter().enumerate() {
            let supports_graphics = props.queue_flags.contains(vk::QueueFlags::GRAPHICS);
            let supports_surface = unsafe {
                surface_loader
                    .get_physical_device_surface_support(physical_device, i as u32, surface)
                    .unwrap_or(false)
            };

            if supports_graphics && supports_surface {
                queue_family_index = i as u32;
                break;
            }
        }

        let queue_priorities = [1.0f32];
        let queue_create_info = [vk::DeviceQueueCreateInfo::default()
            .queue_family_index(queue_family_index)
            .queue_priorities(&queue_priorities)];

        let device_extensions = [ash::khr::swapchain::NAME.as_ptr()];

        let device_info = vk::DeviceCreateInfo::default()
            .queue_create_infos(&queue_create_info)
            .enabled_extension_names(&device_extensions);

        let device = unsafe {
            instance
                .create_device(physical_device, &device_info, None)
                .unwrap()
        };
        let queue = unsafe { device.get_device_queue(queue_family_index, 0) };

        Self {
            entry,
            instance,
            surface,
            surface_loader,
            physical_device,
            device,
            queue,
            queue_family_index,
        }
    }
}
