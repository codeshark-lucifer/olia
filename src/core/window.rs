use sdl3::event::Event;
use sdl3::keyboard::Keycode;
use sdl3::video::GLProfile;

#[allow(dead_code)]
pub struct Window {
    pub title: String,
    pub width: u32,
    pub height: u32,
    running: bool,
    _sdl: sdl3::Sdl,
    window: sdl3::video::Window,
    gl_context: sdl3::video::GLContext,
}

impl Window {
    pub fn new(title: &str, width: u32, height: u32) -> (Self, sdl3::EventPump) {
        let sdl = sdl3::init().unwrap();
        let video = sdl.video().unwrap();

        // OpenGL Configure
        let gl_attr = video.gl_attr();
        gl_attr.set_context_profile(GLProfile::Core);
        gl_attr.set_context_version(3, 3);

        let window = video
            .window(title, width, height)
            .position_centered()
            .opengl()
            .build()
            .unwrap();

        let gl_context = window.gl_create_context().unwrap();
        window.gl_make_current(&gl_context).unwrap();

        // Load OpenGL functions
        gl::load_with(|name| {
            video
                .gl_get_proc_address(name)
                .map_or(std::ptr::null(), |f| f as *const _)
        });

        let event_pump = sdl.event_pump().unwrap();

        let win = Self {
            title: title.to_string(),
            width,
            height,
            running: true,
            _sdl: sdl,
            window,
            gl_context,
        };

        (win, event_pump)
    }

    // 2. Uses &mut self (to modify running) and &mut sdl3::EventPump (borrowing events)
    pub fn process_events(&mut self, events: &mut sdl3::EventPump) -> bool {
        for event in events.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => self.running = false,
                _ => {}
            }
        }

        self.running
    }

    pub fn clear(&self, r:f32, g:f32, b:f32, a: f32) {
        unsafe {
            gl::ClearColor(r, g,b, a);
            gl::Clear(gl::COLOR_BUFFER_BIT);
        }
    }

    pub fn swap_window(&self) {
        self.window.gl_swap_window();
    }
}