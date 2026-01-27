#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

unsigned char* createCrosshairImage(int size, int* width, int* height);
GLFWcursor* createCrosshairCursor();
