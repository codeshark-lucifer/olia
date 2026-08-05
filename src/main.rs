use olia::context::{Context, ContextDesc};
use olia::prelude::*;

fn main() {
    let mut ctx = Context::new(ContextDesc {
        title: "olia".to_string(),
        width: 956,
        height: 540,
        resizable: false,
        fullscreen: false,
    });

    let camera_ent = ctx.world.entities.spawn();
    ctx.world.add_component(
        camera_ent,
        Camera::new(ctx.platform.width as i32, ctx.platform.height as i32),
    );
    ctx.world.add_component(
        camera_ent,
        Transform::new(
            Vec3::new(0.0, 0.0, 3.5), //cam position slightly further
            Vec3::ZERO,
            Vec3::ONE,
        ),
    );

    let object_ent = ctx.world.entities.spawn();
    ctx.world.add_component(
        object_ent,
        Transform::new(Vec3::ZERO, Vec3::ZERO, Vec3::ONE),
    );

    let cube_vertices = create_cube_vertices();
    ctx.renderer.update_vertices(&cube_vertices);

    while ctx.run() {
        let mut transforms = ctx.world.pool_mut::<Transform>();

        if let Some(trans) = transforms.get_mut(object_ent) {
            trans.rotation.x += 0.005;
            trans.rotation.y += 0.01;
        }
    }
}

fn create_cube_vertices() -> Vec<Vertex> {
    vec![
        // -------------------------------------------------------------
        // Front Face (+Z)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 0.0, 1.0),
            uv: Vec2::new(1.0, 0.0),
        },
        // -------------------------------------------------------------
        // Back Face (-Z)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 0.0, -1.0),
            uv: Vec2::new(1.0, 0.0),
        },
        // -------------------------------------------------------------
        // Left Face (-X)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
        // -------------------------------------------------------------
        // Right Face (+X)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
        // -------------------------------------------------------------
        // Top Face (+Y)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::new(0.0, 1.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
        // -------------------------------------------------------------
        // Bottom Face (-Y)
        // -------------------------------------------------------------
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::new(0.0, -1.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
    ]
}
