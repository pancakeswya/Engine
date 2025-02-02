#ifndef ENGINE_WINDOW_PLUGIN_H_
#define ENGINE_WINDOW_PLUGIN_H_

#include "engine/plugin_api.h"

#include "engine/window/instance.h"
#include "engine/window/window.h"

extern "C" {

extern ENGINE_API engine::Instance* PluginCreateInstance();
extern ENGINE_API void PluginDestroyInstance(engine::Instance* instance);

extern ENGINE_API engine::Window* PluginCreateWindow(int width, int height, const std::string& title);
extern ENGINE_API void PluginDestroyWindow(engine::Window* window);

} // extern "C"

#endif // ENGINE_WINDOW_PLUGIN_H_
