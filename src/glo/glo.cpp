#include "glo.h"
#include <gl/glew.h>
#include <plog/Log.h>

namespace glo {

void InitiazlieGlew() {
  glewInit();
  PLOG_INFO << "GLEW_VERSION: " << glewGetString(GLEW_VERSION);
}

} // namespace glo