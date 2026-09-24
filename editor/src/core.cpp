#include <engine/engine.h>
#include <graphics/platform.hpp>

#include <cstdio>
#include <cstdlib>

int main()
{
    OLIA_ENGINE::PLTContext plt_context{};
    plt_context.width = 956;
    plt_context.height = 540;
    plt_context.title = "olia - engine";
    plt_context.fullscreen = false;
    plt_context.resizeable = true;
    
    OLIA_ENGINE::Engine engine(plt_context);

    return EXIT_SUCCESS;
}