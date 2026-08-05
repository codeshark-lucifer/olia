use crate::core::ecs::World;
use crate::core::platform::Platform;
use crate::graphics::Renderer;
use sdl3::EventPump;

#[allow(dead_code)]
pub struct Context {
    pub world: World,
    pub platform: Platform,
    pub events: EventPump,
    pub renderer: Renderer,
}

pub struct ContextDesc {
    pub title: String,
    pub width: u32,
    pub height: u32,
    pub resizable: bool,
    pub fullscreen: bool,
}

impl Context {
    pub fn new(desc: ContextDesc) -> Self {
        let world = World::new();
        let (platform, events) = Platform::new(
            &desc.title,
            desc.width,
            desc.height,
            desc.resizable,
            desc.fullscreen,
        );
        let renderer = Renderer::new(&platform.window, platform.width, platform.height);

        Self {
            world: world,
            platform: platform,
            events: events,
            renderer: renderer,
        }
    }

    pub fn run(&mut self) -> bool {
        if !self.platform.process_events(&mut self.events) {
            return false;
        }

        // draw frame
        self.renderer.draw_frame(&self.world, self.platform.width, self.platform.height);
        true
    }
}
