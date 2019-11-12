#ifndef GL_UTILS_H_INCLUDED
#define GL_UTILS_H_INCLUDED

#define GLEW_STATIC
#include "glew.h"
#include "glfw3.h"
#include <stdio.h>

GLuint CompileShaderFromFile(char* FilePath, GLuint shaderType);

#endif // GL_UTILS_H_INCLUDED
