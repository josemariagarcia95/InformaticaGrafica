/* transform.cpp
 *
 * Informatica Grafica / Computer Graphics.
 * Spring 2017.
 *
 * Practice 3.
 * Transformations in OpenGL.
 *
 * Jose Pascual Molina Masso.
 * Escuela Superior de Ingenieria Informatica de Albacete.
 */
//NOTA MENTAL: LAS FUNCIONES SENO, COSENO, ETC. TRABAJAN CON RADIANES. HAY UNA CONSTANTE PI MAS ABAJO
/****************/
/* Header files */
/****************/

/* General */
#include <iostream> 
#include <stdio.h>
#include <stdlib.h>

/* Microsoft Visual C++ */
#include <windows.h>

/* OpenGL */
#include "GL/glew.h"  /* OpenGL Extension Wrangler*/
#include "GL/glut.h"  /* OpenGL Utility Toolkit */

/* GLM */
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective

/* Zalman Trimon 3D display */
#include "trimon.h"

#include "matrices.h"

#define PI 3.14159265358979323846264338327950288   /* pi */

/*******************/
/* Data structures */
/*******************/

/* Window size */
int width;   // Width
int height;  // Height

/* View menu options */
bool viewAxes = true, viewHUD = true;

/* Navigation modes */
enum navMode_type { ZOOM, ORBIT } navMode;
				 
float dist = 10.0;  // Distance to object
float orbitX = 0.0, 
      orbitY = 0.0; // Rotation angles, in degrees,
					//   around X and Y axes
float speed = 1.0;  // Navigation speed

/* Store pointer coordinates and
   the state of mouse left button */
int lastX = 0, lastY = 0;
bool isLeftButtonPressed = false;

/* Model menu options */
enum whichModel_type { PYRAMIDS, PLANETS } whichModel; 

GLfloat angle = 0.0;  // Angulo de rotacion de cada objeto
					  // respecto a su eje correspondiente

GLfloat angle_Earth, angle_Moon = 0.0;
GLfloat degree = PI / 180.0f;
GLfloat pyramids_factor = 2.0f;
GLfloat planets_factor = 2.0f;

/* Array of characters */
char buffer[256];

/* Pyramid vertex-array and buffer objects */
GLuint pyramid_vao, pyramid_vbo;

/* Macro used in glVertexAttribPointer() calls, from the red book */
#define BUFFER_OFFSET(bytes) ((GLubyte *) NULL + (bytes))


/***********************/
/* Function prototypes */
/***********************/

void init(void);
void output(GLfloat x, GLfloat y, char* text);
void drawAxes(GLfloat length);
void drawPyramid(void);
void drawPyramids(void);
void drawPlanets(void);
void drawModel(void);
void drawHUD(void);
void redraw(void);
void stereoRedraw(void);
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void idle(void);
void viewSelect(int value);
void modelSelect(int value);
void drawmodeSelect(int value);
void pointsizeSelect(int value);
void linewidthSelect(int value);
void navmodeSelect(int value);
void speedSelect(int value);
void windowSelect(int value);
void trimonSelect(int value);
void popupmenu(int value);
void changeAnimation(int value);
void changeSpeedAnimation(int value);

//*******************************************************************

void
initPyramid(void)
{
	// A square pyramid, statically allocated
	static const GLfloat pyramid_positions[] =
	{
		0.5f, 0.0f,  0.5f, //0	 // Bottom (2 triangles)
		-0.5f, 0.0f,  0.5f, //1
		-0.5f, 0.0f, -0.5f, //2
		-0.5f, 0.0f, -0.5f, //2
		0.5f, 0.0f, -0.5f, //3
		0.5f, 0.0f,  0.5f, //0
		0.5f, 0.0f,  0.5f,	//0 // Front face (1 triangle)
		-0.5f, 0.0f,  0.5f, //1
		0.0f, 0.75f,  0.0f, //4 
		0.5f, 0.0f,  0.5f,	//0 // Right face (1 triangle)
		0.5f, 0.0f, -0.5f, //3	
		0.0f, 0.75f,  0.0f, //4  
		-0.5f, 0.0f,  0.5f, //1 // Left face (1 triangle)
		-0.5f, 0.0f, -0.5f, //2
		0.0f, 0.75f,  0.0f, //4 
		-0.5f, 0.0f, -0.5f, //2 // Back face (1 triangle)
		0.5f, 0.0f, -0.5f, //3	
		0.0f, 0.75f,  0.0f //4 
	};

	// Set up the vertex-array object (VAO)
	glGenVertexArrays(1, &pyramid_vao);  // Reserve a name for the object
	glBindVertexArray(pyramid_vao);  // Make the object current

									 // The vertex-related data of a VAO is stored in a vertex-buffer object (VBO) 
	glGenBuffers(1, &pyramid_vbo);  // Reserve a name for the buffer object
	glBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);  // Bind it to the GL_ARRAY_BUFFER target
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_positions), pyramid_positions, GL_STATIC_DRAW);
	// Allocate space for it (sizeof(positions)) and copy data

	// Connect data to vertex shader, the entrance to OpenGL pipeline
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
}

//*******************************************************************

void
init(void)
{  
  /* Set black as background colour, RGB(0,0,0) */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  /* Enable depth buffering */
  glEnable(GL_DEPTH_TEST);

  /* Set modelview matrix as the active one */
  glMatrixMode(GL_MODELVIEW);

  /* Set up the geometry */
  initPyramid();

  /* Draw triangles as lines, user can change this in the menu */
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  navMode = ORBIT;
  whichModel = PYRAMIDS;
}

//*******************************************************************

void
output(GLfloat x, GLfloat y, char* text)
{
  char *p;

  glRasterPos2f(x, y);  
  for (p = text; *p; p++)
    glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *p);
}

//*******************************************************************

void
drawAxes(GLfloat length)
{
  /* Draw three reference axes */
  glLineWidth(1.0);
  glBegin(GL_LINES);
	
  /* X axis, red */
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(length, 0.0, 0.0);

  /* Y axis, green */
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, length, 0.0);

  /* Z axis, blue */
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, length);
  glEnd(); 
}

//*******************************************************************

void
drawPyramid(void)
{
	// Nonindexed drawing
	glBindVertexArray(pyramid_vao);  // Make the object current
	glDrawArrays(GL_TRIANGLES, 0, 18);  // count = 18 vertex positions
}

//*******************************************************************

void
drawPyramids(void)
{
	glm::mat4 viewMat, modelviewMat;
	glGetFloatv(GL_MODELVIEW_MATRIX, &viewMat[0][0]);

	/* Right pyramid, scaled to twice its original size in each axis,
	  and rotated "angle" degrees about the x-axis */
	glm::mat4 pyr1Mat, scale2, rotateX, translateX;
	translateX = Matrices::translate(1.5f, 0.0f, 0.0f);
	scale2 = Matrices::scale(2.0f, 2.0f, 2.0f);
	rotateX = Matrices::rotateX(angle);
	pyr1Mat *= scale2 * rotateX * translateX;

	modelviewMat = viewMat * pyr1Mat;
	glLoadMatrixf(&modelviewMat[0][0]);
	drawPyramid();

	/* Left pyramid, scaled by factor 1.5 and
	  rotated "angle" degrees about the y-axis */

	glm::mat4 pyr2Mat, scale1_5, rotateY, translate_mX;
	translate_mX = Matrices::translate(-2.0f, 0.0f, 0.0f);
	scale1_5 = Matrices::scale(1.5f, 1.5f, 1.5f);
	rotateY = Matrices::rotateY(angle);
	pyr2Mat *= translate_mX * rotateY * scale1_5;

	modelviewMat = viewMat * pyr2Mat;
	glLoadMatrixf(&modelviewMat[0][0]);
	drawPyramid();

	/* Back pyramid, not scaled but rotated
	  "angle" degrees about the z-axis */

	glm::mat4 pyr3Mat, rotateZ, translate_Z;

	translate_Z = Matrices::translate(0.0f, 0.0f, -2.5f);
	rotateZ = Matrices::rotateZ(angle);
	pyr3Mat *= rotateZ * translate_Z;

	modelviewMat = viewMat * pyr3Mat;
	glLoadMatrixf(&modelviewMat[0][0]);
	drawPyramid();

	glLoadMatrixf(&viewMat[0][0]);
}
	
//*******************************************************************

void
drawPlanets(void)
{
	glm::mat4 viewMat, modelviewMat;
	glGetFloatv(GL_MODELVIEW_MATRIX, &viewMat[0][0]);

	glm::mat4 sunMat, rotateY;

	rotateY = Matrices::rotateY(angle);
	sunMat *= rotateY;

	modelviewMat = viewMat * sunMat;
	glLoadMatrixf(&modelviewMat[0][0]);
	glutSolidSphere(1.0, 10, 10);

	/* The Earth */
	//Rotate_T - Movement around the sun
	//Rotate_R - Movement of rotation
	glm::mat4 earthMat, scale0_5, translate_E, rotate_T, rotate_R;
	scale0_5 = Matrices::scale(0.5f, 0.5f, 0.5f);
	translate_E = Matrices::translate(4.0f, 0.0f, 0.0f);
	rotate_T = Matrices::rotateY(angle_Earth);
	rotate_R = Matrices::rotateY(angle_Earth);
	earthMat *= rotate_T * translate_E * rotate_R * scale0_5;

 	modelviewMat = viewMat * earthMat;
 	glLoadMatrixf(&modelviewMat[0][0]);
	glutSolidSphere(1.0, 10, 10);

	/* The Moon */

	glm::mat4 moonMat, scale0_25, translate_M, rotate_TE, rotate_MR;
	scale0_25 = Matrices::scale(0.5f, 0.5f, 0.5f);
	translate_M = Matrices::translate(2.2f, 0.0f, 0.0f);
	rotate_TE = Matrices::rotateY(angle_Moon);
	rotate_MR = Matrices::rotateY(angle_Moon);
	moonMat *= rotate_TE * translate_M * rotate_MR * scale0_25;

	modelviewMat = viewMat * earthMat * moonMat;
	glLoadMatrixf(&modelviewMat[0][0]);
	glutSolidSphere(1.0, 10, 10);

  glLoadMatrixf(&viewMat[0][0]);
}
	
//*******************************************************************

void
drawModel(void)
{
  glColor3f(1.0, 1.0, 1.0);

  /* Which model to draw */
  switch (whichModel) {
    case PYRAMIDS:
      drawPyramids();
      break;
    case PLANETS:
      drawPlanets();
  }
}

//*******************************************************************

void
drawHUD(void)
{
  /* Set projection and modelview matrices to draw 2D text */
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, width-1, 0, height-1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
    
  glColor3f(1.0, 1.0, 1.0);  // White

  switch (navMode) {
    case ZOOM:
		sprintf_s(buffer, "Left button and drag: ZOOM");
		output(10, 30, buffer);
		break;
	case ORBIT:
		sprintf_s(buffer, "Left button and drag: ORBIT");
		//sprintf_s(buffer, "Left button and drag: ORBIT");
		output(10, 30, buffer);
  }
  sprintf_s(buffer, "Keys 4,8,2 and 6 to orbit, + and - to zoom in-out, 0 to restore");
  output(10, 15, buffer);

  /* Restore previous projection and modelview matrices */
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

//*******************************************************************

void
redraw(void)
{
  /* Clear frame and depth buffer bits */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* Navigation transformations */
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -dist);
  glRotatef(-orbitX, 1.0, 0.0, 0.0);
  glRotatef(-orbitY, 0.0, 1.0, 0.0);

  /* Draw reference axes */
  if (viewAxes)
    drawAxes(5.0);  

  /* Draw model */
  drawModel();

  /* Draw on-screen information */
  if (viewHUD)
    drawHUD();

  /* Swap back and front buffers */
  glutSwapBuffers();
}

//*******************************************************************

void
stereoRedraw(void)
{
  GLfloat eyesep;  // Eye separation, interpupillary or interocular distance
  GLfloat offset;
  int i;

  eyesep = dist/15.0;

  /* Draw stencial pattern for the interleaved stereo mode */
  trimonPreStereoImg(width, height);

  /* Clear frame and depth buffer bits */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  offset = -eyesep/2.0;  /* Left eye at (-eyesep/2,0,dist) */
  for (i=0; i<2; i++) {
    if (i == 0) {
      /* Left eye, even rows */
      trimonPreLeftImg();
	} else {
      /* Right eye, odd rows */
      trimonPreRightImg();
	}

    /* Set viewpoint */
    glLoadIdentity();
    gluLookAt(offset, 0.0, dist,  /* Left or right eye */
              0.0, 0.0, 0.0,      /* Look at point (0,0,0) */
              0.0, 1.0, 0.0       /* Up vector pointing in the positive Y direction */
             );

    /* Navigation transformations */
    glRotatef(-orbitX, 1.0, 0.0, 0.0);
    glRotatef(-orbitY, 0.0, 1.0, 0.0);

    /* Draw reference axes */
    if (viewAxes)
      drawAxes(5.0);  

    /* Draw model */
    drawModel();

    offset += eyesep;  /* Right eye at (+eyesep/2,0,dist) */
  }

  /* Masks are no longer needed */
  trimonPostStereoImg();
  
  /* Draw on-screen information */
  if (viewHUD)
    drawHUD();

  /* Swap back and front buffers */
  glutSwapBuffers();
}

//*******************************************************************

void
reshape(int w, int h)
{
  /* Viewport is same size as the application window */
  glViewport(0, 0, (GLsizei) w, (GLsizei) h);

  /* Set perspective projection parameters */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(40.0,         /* FOV, field of view
                                  (vertical), in degrees */
                 (float) w/h,  /* Aspect ratio width:height */
                 0.1, 10000.0  /* Front face (Z near, hither) and rear face
                                  (Z far, yon) of the viewing volume */
                );
 
  /* Restore modelview matrix as the active one */
  glMatrixMode(GL_MODELVIEW);

  /* Save current window size */
  width = w;
  height = h;
}

//*******************************************************************

void 
keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case '+':
      dist -= 0.1 * speed;    // Zoom-in
      break;
    case '-':
      dist += 0.1 * speed;    // Zoom-out
      break;
    case '4':
      orbitY += 1.0 * speed;  // Orbit to the right
      break;
    case '6':
      orbitY -= 1.0 * speed;  // Orbit to the left
      break;
    case '8':
      orbitX += 1.0 * speed;  // Orbit downwards
      break;
    case '2':
      orbitX -= 1.0 * speed;  // Orbit upwards
      break;
	case '0':
	  dist = 10.0;   // Restore initial values
	  orbitX = 0.0;
	  orbitY = 0.0;
	  break;
    case 27:         // Escape key
      exit(0);
  }
  glutPostRedisplay();
}

//*******************************************************************

void
mouse(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON) {  // If the left button...
    if (state == GLUT_DOWN) {        // ...is pressed
      isLeftButtonPressed = true;
      lastX = x;
      lastY = y;
    }
    else {
      if (state == GLUT_UP) {        // ...is released
        isLeftButtonPressed = false;
		switch (navMode) {
		  case ZOOM:
		    dist += (y - lastY)*speed/10.0;
			if (dist < 1.0) dist = 1.0;
			break;
		  case ORBIT:
            orbitX += (lastY - y)*speed;
            orbitY += (lastX - x)*speed;
		}
        glutPostRedisplay();
      }
    }
  }
}

//*******************************************************************

void
motion(int x, int y)
{
  if (isLeftButtonPressed) {     // If the left button is pressed
    switch (navMode) {
      case ZOOM:
	    dist += (y - lastY)*speed/10.0;
	    if (dist < 1.0) dist = 1.0;
	    break;
      case ORBIT:
        orbitX += (lastY - y)*speed;
        orbitY += (lastX - x)*speed;
    }
    lastX = x;
    lastY = y;
    glutPostRedisplay();
  }
}

//*******************************************************************

void 
idle(void)
{	
	if (whichModel == PYRAMIDS) {
		angle += degree / pyramids_factor;
		drawPyramids();
	}
	else {
		angle += degree / planets_factor;
		angle_Earth += degree / (planets_factor * 2.0f);
		angle_Moon += degree / (planets_factor * 3.0f);
		if (angle_Moon > 2 * PI) angle_Moon = 0.0f;
		if (angle_Earth > 2 * PI) angle_Earth = 0.0f;
		drawPlanets();
	}
	if (angle > 2 * PI) angle = 0.0f;
	glutPostRedisplay();
}

//*******************************************************************

void
viewSelect(int value)
{
  if (value == 1) viewAxes = !viewAxes;
  if (value == 2) viewHUD = !viewHUD;
  glutPostRedisplay();
}

//*******************************************************************

void
modelSelect(int value)
{
  switch (value) {
    case 1:
      whichModel = PYRAMIDS;
	  break;
	case 2:
      whichModel = PLANETS;
  }
  glutPostRedisplay();
}

//*******************************************************************

void
drawmodeSelect(int value)
{
	switch (value) {
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 3:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glutPostRedisplay();
}

//*******************************************************************

void
pointsizeSelect(int value)
{
	switch (value) {
	case 1:
		glPointSize(1.0f);  // Default
		break;
	case 2:
		glPointSize(3.0f);
		break;
	case 3:
		glPointSize(5.0f);
	}
	glutPostRedisplay();
}

//*******************************************************************

void
linewidthSelect(int value)
{
	switch (value) {
	case 1:
		glLineWidth(1.0f);  // Default
		break;
	case 2:
		glLineWidth(3.0f);
		break;
	case 3:
		glLineWidth(5.0f);
	}
	glutPostRedisplay();
}

//*******************************************************************

void
navmodeSelect(int value)
{
  switch (value) {
    case 1:
      navMode = ZOOM;   // Zoom in-out
	  break;
	case 2:
	  navMode = ORBIT;  // Orbit around the model
	  break;
	case 3:
	  dist = 10.0;      // Restore initial values
	  orbitX = 0.0;
	  orbitY = 0.0;
  }
  glutPostRedisplay();
}

//*******************************************************************

void
speedSelect(int value)
{
  switch (value) {
    case 1:
      speed = 0.1;   // Slow navigation
	  break;
	case 2:
	  speed = 1.0;   // Normal
	  break;
	case 3:
	  speed = 10.0;  // Fast
  }
}

//*******************************************************************
void
windowSelect(int value)
{
  if (value == 1) glutFullScreen();
  if (value == 2) glutReshapeWindow(640, 480);
  glutPostRedisplay();
}

//*******************************************************************

void
trimonSelect(int value)
{
  if (value == 1) {
      glClearColor(0.7, 0.7, 0.7, 0.0);
      trimonEnableStereo();
      glutDisplayFunc(stereoRedraw);
	  glutFullScreen();
  }
  if (value == 2) {
      glClearColor(0.0, 0.0, 0.0, 0.0);
      trimonDisableStereo();
      glutDisplayFunc(redraw);
  }
  glutPostRedisplay();
}

//*******************************************************************

void
popupmenu(int value)
{
  if (value == 666)
    exit(0);
}
//*******************************************************************

void
changeAnimation(int value)
{
	if (value == 1) glutIdleFunc(NULL);
	if (value == 2) glutIdleFunc(idle);
	glutPostRedisplay();
}
//*******************************************************************

void
changeSpeedAnimation(int value)
{
	if (whichModel == PYRAMIDS) {
		pyramids_factor = (float)value;
	}
	else {
		planets_factor = (float)value;
	}
	glutPostRedisplay();
}
//*******************************************************************

void
main(int argc, char* argv[])
{
  /* Ids for the submenus of the pop-up menu */
	int viewMenu, modelMenu, drawmodeMenu, pointsizeMenu, linewidthMenu,
		navmodeMenu, speedMenu, windowMenu, trimonMenu, animation, animation_speed;

  /* Process the command line and negotiates the start
	 of an OpenGL session with the window system */
  glutInit(&argc, argv);
  
  /* Set the OpenGL display mode for the window that will be
     created next: doble buffer, RGBA colour model, depth buffer and stencil buffer */
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);

  /* Create a window for OpenGL rendering: selects the appropriate visual
     and colourmap, and create an OpenGL context associated to this window */
  glutInitWindowSize(640, 480);
  glutCreateWindow("transform");

  /* Initialize GLEW */
  GLenum err = glewInit();
  if (GLEW_OK != err) {
	  std::cerr << "Error: " << glewGetString(err) << std::endl;
  }

  /* Initialize the OpenGL context */
  init();
  
  /* Create a pop-up menu with several submenus */

  viewMenu = glutCreateMenu(viewSelect);
  glutAddMenuEntry("Axes", 1);
  glutAddMenuEntry("Info", 2);

  animation = glutCreateMenu(changeAnimation);
  glutAddMenuEntry("Stop", 1);
  glutAddMenuEntry("Resume", 2);

  animation_speed = glutCreateMenu(changeSpeedAnimation);
  glutAddMenuEntry("Slow", 3);
  glutAddMenuEntry("Normal", 2);
  glutAddMenuEntry("Fast", 1);

  modelMenu = glutCreateMenu(modelSelect);
  glutAddMenuEntry("Pyramids", 1);
  glutAddMenuEntry("Planets", 2);

  drawmodeMenu = glutCreateMenu(drawmodeSelect);
  glutAddMenuEntry("Points", 1);
  glutAddMenuEntry("Lines", 2);
  glutAddMenuEntry("Filled", 3);

  pointsizeMenu = glutCreateMenu(pointsizeSelect);
  glutAddMenuEntry("Small", 1);
  glutAddMenuEntry("Medium", 2);
  glutAddMenuEntry("Big", 3);

  linewidthMenu = glutCreateMenu(linewidthSelect);
  glutAddMenuEntry("Thin", 1);
  glutAddMenuEntry("Medium", 2);
  glutAddMenuEntry("Thick", 3);

  navmodeMenu = glutCreateMenu(navmodeSelect);
  glutAddMenuEntry("Zoom", 1);
  glutAddMenuEntry("Orbit", 2);
  glutAddMenuEntry("Restore", 3);

  speedMenu = glutCreateMenu(speedSelect);
  glutAddMenuEntry("Slow", 1);
  glutAddMenuEntry("Normal", 2);
  glutAddMenuEntry("Fast", 3);

  windowMenu = glutCreateMenu(windowSelect);
  glutAddMenuEntry("Full screen", 1);
  glutAddMenuEntry("Restore", 2);

  trimonMenu = glutCreateMenu(trimonSelect);
  glutAddMenuEntry("Enable stereo", 1);
  glutAddMenuEntry("Disable estereo", 2);

  glutCreateMenu(popupmenu);
  glutAddSubMenu("View", viewMenu);
  glutAddSubMenu("Model", modelMenu);
  glutAddSubMenu("Navigation", navmodeMenu);
  glutAddSubMenu("Speed", speedMenu);
  glutAddSubMenu("Animation speed", animation_speed);
  glutAddSubMenu("Animation", animation);
  glutAddSubMenu("Window", windowMenu);
  glutAddSubMenu("Trimon", trimonMenu);
  glutAddMenuEntry("Exit", 666);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  /* Register event handler functions */

  /* Called each time the window needs to be redisplayed */ 
  glutDisplayFunc(redraw);

  /* Called each time the window is moved or resized */
  glutReshapeFunc(reshape);

  /* Called when a key that generates an ASCII code is pressed */
  glutKeyboardFunc(keyboard);
  
  /* Called when a mouse button is pressed or released */
  glutMouseFunc(mouse);

  /* Called when the pointer is moved within window limits
     while one or more mouse buttons are pressed */ 
  glutMotionFunc(motion);

  /* Called when there is nothing else to do */
  glutIdleFunc(idle);

  /* Event loop */
  glutMainLoop();
}
