#pragma once

#define ENGINE_NAME "TinyEngine"
#define ENGINE_VERSION_MAJOR 1
#define ENGINE_VERSION_MINOR 0
#define ENGINE_VERSION_BUILD 0

#if defined(_WIN32)
#define USE_SDL_PLATFORM
#define USE_SDL_WINDOW
#define USE_VULKAN_GRAPHICS
#else
#error Platform not configured.
#endif

