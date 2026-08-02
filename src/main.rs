mod core;
mod utils;

use core::ecs::{Entity, World};
use core::window::Window;

use utils::mathf::{Mat4, Vec3};

// --- COMPONENTS ---

#[allow(dead_code)]
#[derive(Debug)]
struct Transform {
    position: Vec3,
    rotation: Vec3,
    scale: Vec3,
}

#[derive(Debug)]
struct Velocity {
    direction: Vec3,
}

struct View {
    width: i32,
    height: i32,
}

struct Camera {
    view: View,
    fov: f32,
    near: f32,
    far: f32,
}

impl Camera {
    pub fn new(width: i32, height: i32) -> Self {
        Self {
            view: View { width, height },
            fov: 45.0,
            near: 0.1,
            far: 1000.0,
        }
    }

    pub fn get_matrices(&self, trans: &Transform) -> (Mat4, Mat4) {
        let target = trans.position + Vec3::FORWARD;
        let view = Mat4::look_at(trans.position, target, Vec3::UP);

        let aspect = self.view.width as f32 / self.view.height as f32;
        let proj = Mat4::perspective(self.fov.to_radians(), aspect, self.near, self.far);

        (view, proj)
    }
}

// --- HELPER FUNCTIONS ---

// Fixed: Changed `world: &World` to `world: &mut World`
fn create_entity(world: &mut World) -> Entity {
    let entity = world.entities.spawn();

    world.add_component(
        entity,
        Transform {
            position: Vec3::ZERO,
            rotation: Vec3::ZERO,
            scale: Vec3::ONE,
        },
    );

    entity
}

// --- MAIN GAME ---

fn main() {
    let (mut window, mut events) = Window::new("olia", 956, 540);
    let mut world = World::new();

    // Fixed: Pass `&mut world`
    let ent_camera = create_entity(&mut world);
    let ent_player = create_entity(&mut world);

    // Attach Components
    world.add_component(ent_camera, Camera::new(956, 540));
    world.add_component(
        ent_player,
        Velocity {
            direction: Vec3::new(0.01, 0.0, 0.0),
        },
    );

    // Compute View, Projection, and MVP Matrix
    // Fixed: Renamed `mvp` to `_mvp` to suppress unused variable warning
    let _mvp = {
        let cameras = world.pool::<Camera>().unwrap();
        let transforms = world.pool::<Transform>().unwrap();

        if let (Some(camera), Some(trans)) = (cameras.get(ent_camera), transforms.get(ent_camera))
        {
            let (view, proj) = camera.get_matrices(trans);
            let model = Mat4::translate(Vec3::new(1.0, 0.0, 0.0))
                * Mat4::rotate(45.0f32.to_radians(), Vec3::UP);

            proj * view * model
        } else {
            Mat4::identity()
        }
    };

    println!("Initial MVP Matrix calculated successfully!");

    // Main Game Loop
    while window.process_events(&mut events) {
        window.clear(0.1f32, 0.1f32, 0.1f32, 1.0f32);

        // MOVEMENT SYSTEM
        {
            let mut velocities = world.pool_mut::<Velocity>();
            let mut transforms = world.pool_mut::<Transform>();

            for (entity, vel) in velocities.iter_mut() {
                if let Some(trans) = transforms.get_mut(*entity) {
                    trans.position += vel.direction;
                }
            }
        }

        window.swap_window();
    }
}