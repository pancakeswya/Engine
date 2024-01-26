#include "engine.h"
#include "base/defs.h"
#include "view/application.h"
#include "view/window.h"

namespace engine {

Result Run() {
  Application app;
  if (!app.IsInitialized()) {
    return kFailure;
  }
  Window window(defs::kWindowWidth, defs::kWindowHeight, defs::kWindowTitle);

  if (!window.IsInitialized()) {
    return kFailure;
  }
  return window.Poll();
}

} // namespace engine