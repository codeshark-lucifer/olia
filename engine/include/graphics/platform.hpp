#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <SDL3/SDL.h>

namespace OLIA_ENGINE
{
    struct PLTContext
    {
        uint32_t width = 0;
        uint32_t height = 0;
        const char *title = nullptr;
        bool fullscreen = false;
        bool resizeable = false;
    };

    class Platform
    {
    public:
        Platform(PLTContext);
        ~Platform();

        SDL_Window *window_ptr() const { return m_window; }
        bool PollEvent();

    private:
        SDL_Window *m_window;
    };
} // namespace OLIA_EGNINE
