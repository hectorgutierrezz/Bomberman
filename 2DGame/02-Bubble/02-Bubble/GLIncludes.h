#ifndef GL_INCLUDES_H
#define GL_INCLUDES_H

#ifdef __EMSCRIPTEN__
#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>
#else
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#endif
