use olia::prelude::*;

const VERTEX_CODE: &str = r#"
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aUV;

    uniform mat4 u_MVP;

    out vec3 v_Normal;

    void main() {
        gl_Position = u_MVP * vec4(aPos, 1.0);
        v_Normal = aNormal;
    }
"#;

const FRAGMENT_CODE: &str = r#"
    #version 330 core
    out vec4 FragColor;

    in vec3 v_Normal;

    uniform vec3 u_Color;

    void main() {
        vec3 lightDir = normalize(vec3(0.5, 1.0, 0.8));
        float diff = max(dot(v_Normal, lightDir), 0.2); // 0.2 ambient
        FragColor = vec4(u_Color * diff, 1.0);
    }
"#;

#[allow(dead_code)]
fn create_entity(world: &mut World) -> Entity {
    let entity = world.entities.spawn();
    let transform = Transform::default();
    world.add_component(entity, transform);
    entity
}

fn create_camera(world: &mut World, width: i32, height: i32) -> Entity {
    let camera = world.entities.spawn();
    let mut transform = Transform::default();
    transform.position = Vec3::new(0.0, 0.0, 3.5);

    world.add_component(camera, transform);
    world.add_component(camera, Camera::new(width, height));
    camera
}

fn create_cube_mesh() -> Mesh {
    let vertices = vec![
        // Front Face
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::BACK,
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::BACK,
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::BACK,
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::BACK,
            uv: Vec2::new(0.0, 1.0),
        },
        // Back Face
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::FORWARD,
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::FORWARD,
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::FORWARD,
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::FORWARD,
            uv: Vec2::new(0.0, 0.0),
        },
        // Top Face
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::UP,
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::UP,
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::UP,
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::UP,
            uv: Vec2::new(1.0, 1.0),
        },
        // Bottom Face
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::DOWN,
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::DOWN,
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::DOWN,
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::DOWN,
            uv: Vec2::new(0.0, 1.0),
        },
        // Right Face
        Vertex {
            position: Vec3::new(0.5, -0.5, -0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, -0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, 0.5, 0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
        Vertex {
            position: Vec3::new(0.5, -0.5, 0.5),
            normal: Vec3::new(1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        // Left Face
        Vertex {
            position: Vec3::new(-0.5, -0.5, -0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, -0.5, 0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 0.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, 0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(1.0, 1.0),
        },
        Vertex {
            position: Vec3::new(-0.5, 0.5, -0.5),
            normal: Vec3::new(-1.0, 0.0, 0.0),
            uv: Vec2::new(0.0, 1.0),
        },
    ];

    let indices = vec![
        0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 8, 9, 10, 10, 11, 8, 12, 13, 14, 14, 15, 12, 16, 17,
        18, 18, 19, 16, 20, 21, 22, 22, 23, 20,
    ];

    Mesh { vertices, indices }
}

struct Context {
    world: World,
    shader: Shader,
    default_camera: Entity,
}

impl Context {
    pub fn new(width: u32, height: u32) -> Self {
        let mut world = World::new();
        let shader =
            Shader::from_source(VERTEX_CODE, FRAGMENT_CODE).expect("Failed to compile shader");
        let default_camera = create_camera(&mut world, width as i32, height as i32);

        Context {
            world,
            shader,
            default_camera,
        }
    }
}

struct Spinner {
    speed: Vec3,
}

fn main() {
    let (mut window, mut events) = Window::new("olia", 956, 540, false, false);
    let mut context = Context::new(window.width, window.height);

    unsafe {
        gl::Enable(gl::DEPTH_TEST);
    }

    // Pass &mut context
    setup(&mut context);

    while window.process_events(&mut events) {
        window.clear(0.1, 0.1, 0.1, 1.0);
        update(&mut context);
        render(&mut context);
        window.swap_window();
    }
}

// Accept &mut Context so entities can be spawned
fn setup(context: &mut Context) {
    let cube_mesh: Mesh = create_cube_mesh();
    let cube = context.world.entities.spawn();
    context.world.add_component(cube, Transform::default());
    context
        .world
        .add_component(cube, MeshRenderer::new(&cube_mesh));
    context.world.add_component(
        cube,
        Spinner {
            speed: Vec3 {
                x: 0.001,
                y: 0.001,
                z: 0.001,
            },
        },
    );
}

fn update(context: &Context) {
    // 1. Check if the Spinner pool exists (read-only)
    let Some(spinners) = context.world.pool::<Spinner>() else {
        return;
    };

    // 2. Fetch the Transform pool mutably
    let mut transforms = context.world.pool_mut::<Transform>();

    // 3. Iterate over spinners and mutate transforms
    for (entity, spinner) in spinners.iter() {
        if let Some(trans) = transforms.get_mut(*entity) {
            trans.rotation.y += spinner.speed.y; // Now works! `trans` is `&mut Transform`
        }
    }
}

fn render(context: &mut Context) {
    let Some(cameras) = context.world.pool::<Camera>() else {
        return;
    };
    let Some(transforms) = context.world.pool::<Transform>() else {
        return;
    };
    let Some(renderers) = context.world.pool::<MeshRenderer>() else {
        return;
    };

    if let (Some(camera), Some(cam_trans)) = (
        cameras.get(context.default_camera),
        transforms.get(context.default_camera),
    ) {
        let (view, proj) = camera.get_matrices(cam_trans);

        context.shader.use_program();
        context
            .shader
            .set_vec3("u_Color", &Vec3::new(0.2, 0.7, 1.0));

        for (entity, renderer) in renderers.iter() {
            if let Some(trans) = transforms.get(*entity) {
                let model = trans.get_matrix();
                let mvp = proj * view * model;

                context.shader.set_mat4("u_MVP", &mvp);
                renderer.draw();
            }
        }
    }
}
