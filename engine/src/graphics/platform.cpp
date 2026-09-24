#include "graphics/platform.hpp"
#include <stdexcept>
#include <string>

namespace OLIA_ENGINE
{
    Platform::Platform(PLTContext context)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
            throw std::runtime_error("Failed to initialize SDL.");

        SDL_WindowFlags flags = 0;
        if (context.resizeable)
            flags |= SDL_WINDOW_RESIZABLE;
        if (context.fullscreen)
            flags |= SDL_WINDOW_FULLSCREEN;
        flags |= SDL_WINDOW_VULKAN;
        m_window = SDL_CreateWindow(context.title, context.width, context.height, flags);
        if (!m_window)
            throw std::runtime_error("Failed to create SDL_Window");
    }

    Platform::~Platform()
    {
        SDL_DestroyWindow(m_window);
    }

    bool Platform::PollEvent()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                return false;

            default:
                break;
            }
        }
        return true;
    }
} // namespace OLIA_ENGINE
