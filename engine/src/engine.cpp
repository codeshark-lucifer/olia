#include "engine/engine.h"
#include "ecs/world.h"
#include "core/built-in.hpp"

ECS::World *g_world = nullptr;
namespace OLIA_ENGINE
{
    Engine::Engine(PLTContext plt_context)
    {
        g_world = new ECS::World();
        platform = new Platform(plt_context);
        renderer = new Renderer(platform->window_ptr());

        ECS::Entity entity = g_world->create_entity();
        Mesh mesh{};
        mesh.vertices = {
            // Bottom-left
            {
                .position = {-0.5f, -0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, 0.0f}},

            // Bottom-right
            {
                .position = {0.5f, -0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {1.0f, 0.0f}},

            // Top-right
            {
                .position = {0.5f, 0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {1.0f, 1.0f}},

            // Top-left
            {
                .position = {-0.5f, 0.5f, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, 1.0f}}};

        mesh.indices = {0, 1, 2, 2, 3, 0};

        MeshFilter m_filter{};
        m_filter.mesh = std::make_shared<Mesh>(mesh);

        MeshRenderer m_render{};
        g_world->add_component<MeshFilter>(entity, std::move(m_filter));
        g_world->add_component<MeshRenderer>(entity, std::move(m_render));

        Transform mesh_trans{};
        g_world->add_component<Transform>(entity, std::move(mesh_trans));

        ECS::Entity camera_ent = g_world->create_entity();

        Transform camera_trans{};
        camera_trans.position.z = 5.0f;
        g_world->add_component<Transform>(camera_ent, std::move(camera_trans));

        Camera camera_cmp;
        g_world->add_component<Camera>(camera_ent, std::move(camera_cmp));

        while (platform->PollEvent())
        {
            renderer->DrawFrame();
        }
    }

    Engine::~Engine()
    {
        if (renderer)
            delete renderer;
        if (platform)
            delete platform;
        if (g_world)
            delete g_world;
    }
} // namespace OLIA_ENGIEN
