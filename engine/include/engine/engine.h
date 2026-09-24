#pragma once
#include "graphics/renderer.hpp"
#include "graphics/platform.hpp"

namespace OLIA_ENGINE
{
    class Engine
    {
    private:
        Renderer *renderer;
        Platform *platform;

    public:
        Engine(PLTContext plt_context);
        ~Engine();
    };
} // namespace OLIA_ENGINE
