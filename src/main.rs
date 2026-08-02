mod core;

use core::window::Window;

fn main() {
    let (mut window, mut events) = Window::new("olia", 956, 540);

    // Main Game Loop
    while window.process_events(&mut events) {
        window.clear(0.1f32, 0.1f32, 0.1f32, 1.0f32);

        window.swap_window();
    }
}
