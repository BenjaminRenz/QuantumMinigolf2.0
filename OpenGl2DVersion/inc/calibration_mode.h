#ifndef CALIBRATION_MODE_H_INCLUDED
#define CALIBRATION_MODE_H_INCLUDED
#include "filereader.h"
#define GLEW_STATIC
#include "glew.h"
#include "glfw3.h"
#include "linmath.h"
#include "map_camera_plane.h"
#include "simulation.h" //Could be removed still uses simresxy
#include "image_analyse.h"

#include "gl_utils.h" //for compile shader


#include <stdlib.h>
#include <stdio.h>

GLFWwindow* MainWindow;
mat3x3 CalibData;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void drop_file_callback(GLFWwindow* window, int count, const char** paths);
void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void windows_size_callback(GLFWwindow* window, int width, int height);
void glfw_error_callback(int error, const char* description);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

void enter_calibration_mode();
#endif // CALIBRATION_MODE_H_INCLUDED
