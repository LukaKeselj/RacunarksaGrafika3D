#include "../Header/Crosshair.h"
#include <cstring>

unsigned char* createCrosshairImage(int size, int* width, int* height) {
    *width = size;
    *height = size;
    
    unsigned char* pixels = new unsigned char[size * size * 4];
    memset(pixels, 0, size * size * 4);
    
    int center = size / 2;
    int thickness = 3;
    int lineLength = size / 3;
    int gap = 6;
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = (y * size + x) * 4;
            
            bool isHorizontalLine = (y >= center - thickness/2 && y <= center + thickness/2) &&
                                   ((x >= center - lineLength && x <= center - gap) ||
                                    (x >= center + gap && x <= center + lineLength));
            
            bool isVerticalLine = (x >= center - thickness/2 && x <= center + thickness/2) &&
                                 ((y >= center - lineLength && y <= center - gap) ||
                                  (y >= center + gap && y <= center + lineLength));
            
            bool isCenterDot = (x >= center - 1 && x <= center + 1) &&
                              (y >= center - 1 && y <= center + 1);
            
            if (isHorizontalLine || isVerticalLine) {
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 50;
                pixels[idx + 2] = 50;
                pixels[idx + 3] = 255;
            } else if (isCenterDot) {
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 50;
                pixels[idx + 2] = 50;
                pixels[idx + 3] = 200;
            } else {
                pixels[idx + 0] = 0;
                pixels[idx + 1] = 0;
                pixels[idx + 2] = 0;
                pixels[idx + 3] = 0;
            }
        }
    }
    
    return pixels;
}

GLFWcursor* createCrosshairCursor() {
    int width, height;
    unsigned char* pixels = createCrosshairImage(64, &width, &height);
    
    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels;
    
    GLFWcursor* cursor = glfwCreateCursor(&image, width / 2, height / 2);
    
    delete[] pixels;
    
    return cursor;
}
