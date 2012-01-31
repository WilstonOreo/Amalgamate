#include "infinitepanorama/GUI.hpp"

int lbutton, posx, panPos;

struct PanoramaElement
{
	void draw()
	{
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_QUADS);
		glVertex2f( posX+panPos+0, 0 );
		glVertex2f( posX+panPos+100, 0 );
		glVertex2f( posX+panPos+100, 100 );
		glVertex2f( posX+panPos+0, 100 );
		glEnd();
	}
//	Image image, seamImage;
	int posX, posY;
};

deque<PanoramaElement> panoramaImages;



void main_reshape(int width, int height)
{
    glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(0,width,height,0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

void main_display()
{
	glClearColor(0.0,0.0,0.0,0.0);
	glClear(GL_COLOR_BUFFER_BIT); 

	panoramaImages.clear();

	for (int i = 0; i < 5; i++)
	{
		PanoramaElement e;
	//	e.image.resize( Geometry(200,200,0,0) );
	//	e.image.draw( DrawableRectangle(100,100,0,0));
		e.posX = 150*i; e.posY = 0;
		e.draw();
		panoramaImages.push_front(e);
	}

	glutSwapBuffers();
}

void main_mouse(int btn, int state, int x, int y) 
{
	if (btn == GLUT_LEFT) {
		if (state == GLUT_UP) {
			lbutton = 0;
		}
		if (state == GLUT_DOWN) {
			lbutton = 1;
			posx = x;
		}
	}
	main_display();
}

void main_mouseMotion(int x, int y)
{
	if (lbutton)
	{ 	
		int diff = x-posx;
		posx = x;
		panPos += diff;	
	}
	
	main_display();
}

void main_keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27: exit(0); break;

	}
	main_display(); 
}



