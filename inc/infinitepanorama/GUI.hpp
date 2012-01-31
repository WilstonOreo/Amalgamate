#pragma once 
#include <GL/glut.h>
#include <deque>

using namespace std;

static int mainWindowHandle;

void main_reshape(int width, int height);
void main_display();

void main_mouse(int btn, int state, int x, int y);
void main_mouseMotion(int x, int y);
void main_keyboard(unsigned char key, int x, int y);



