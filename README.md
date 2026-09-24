get extensions
validation layers {"VK_LAYER_KHRONOS_validation"}
create appinfo
create instance info 
setup debug info for print debug
create instance 
create debug messenger
create surface

pick physical device

destroy surface KHR
destroy debug messenger EXT
destroy instance.

## TODO

- [x] Create a Vulkan instance.
- [x] Create the debug messenger.
- [x] Create the window surface.
- [x] Select a physical device.
- [x] Create the logical device and queues.
- [x] Create the swapchain.
- [x] Create swapchain image views.
- [x] Create a render pass or dynamic rendering setup.
- [x] Create graphics pipeline and shaders.
- [x] Create framebuffers or rendering targets.
- [x] Create command pools and command buffers.
- [x] Add synchronization objects.
- [x] Implement the draw frame loop.
- [x] Handle swapchain recreation on resize.
- [x] Add cleanup for all rendering resources.