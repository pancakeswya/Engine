#ifndef ENGINE_WINDOW_PLUGIN_H_
#define ENGINE_WINDOW_PLUGIN_H_

#include "engine/window/instance.h"
#include "engine/window/window.h"

extern "C" {

extern engine::Instance* GetInstance();
extern void DestroyInstance(engine::Instance* instance);

extern engine::Window* CreateWindow(int width, int height, const std::string& title);
extern void DestroyWindow(engine::Window* window);

} // extern "C"

#endif // ENGINE_WINDOW_PLUGIN_H_
