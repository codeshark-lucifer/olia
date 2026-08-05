use sdl3::event::{Event, WindowEvent};
use sdl3::keyboard::Keycode;

#[allow(dead_code)]
pub struct Platform {
    pub sdl: sdl3::Sdl,
    pub window: sdl3::video::Window,
    pub width: u32,
    pub height: u32,
    pub running: bool,
}

impl Platform {
    pub fn new(
        title: &str,
        width: u32,
        height: u32,
        resizable: bool,
        fullscreen: bool,
    ) -> (Self, sdl3::EventPump) {
        let sdl = sdl3::init().expect("Failed to initialize SDL3.");
        let video = sdl.video().expect("Failed to get video subsystem");

        let mut builder = video.window(title, width, height);
        if resizable {
            builder.resizable();
        }
        if fullscreen {
            builder.fullscreen();
        }

        let window = builder.vulkan().build().expect("Failed to create SDL3 window");
        let event_pump = sdl.event_pump().expect("Failed to get event pump");

        let win = Self {
            sdl,
            window,
            width,
            height,
            running: true,
        };
        (win, event_pump)
    }

    pub fn process_events(&mut self, events: &mut sdl3::EventPump) -> bool {
        for event in events.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => self.running = false,
                Event::Window {
                    win_event: WindowEvent::Resized(w, h),
                    ..
                } => {
                    self.width = w as u32;
                    self.height = h as u32;
                }
                _ => {}
            }
        }
        self.running
    }
}
