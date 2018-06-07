#pragma once

#if defined(_WIN32)
#define USE_SDL_PLATFORM
#define USE_SDL_WINDOW
#define USE_SDL_INPUT
#define USE_VULKAN_GRAPHICS
#else
#error Platform not configured.
#endif

