#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"

#include "glslprogram.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Jack Schofield

// title of these windows:

const char *WINDOWTITLE = { "CS450 Final Project -- schofija" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the escape key:

const int ESCAPE = { 0x1b };

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };

// size of the 3d box:

const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };

// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta",
	(char*)"White",
	(char*)"Black"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };

// Bezier curves:
	const int NUMPOINTS = 30;
	const int NUMCURVES = 2;

	struct Point
	{
		float x0, y0, z0;       // initial coordinates
		float x, y, z;        // animated coordinates
	};

	struct Curve
	{
		float r, g, b;
		Point p0, p1, p2, p3;
	};

// Shader constants:
const float uKa	= .3;
const float uKd	= .3;
const float uKs	= .3;

const float uColorR = .25;
const float uColorG = .25;
const float uColorB = .25;

const float uSpecR = 1.;
const float uSpecG = 1.;
const float uSpecB = 1.;
const float uShininess		= 1.;

// Keyboard case 
const float CASE_THICKNESS = 0.2;
const float CASE_LENGTH = 4.6;
const float CASE_WIDTH = 12.;
const float CASE_HEIGHT = 0.75;


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong
//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// non-constant global variables:

GLuint	AxesList;				// list to hold the axes
GLuint	BoxList;				// object display list

GLuint  keylist;				// List for keyboard keys
GLuint	caselist;				// List for keyboard case			

int		ActiveButton;			// current button that is down
int		AxesOn;					// != 0 means to draw the axes
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
float	Time;					// timer in the range [0.,1.)
int		WhichColor;				// index into Colors[ ]
int		WhichProjection;		// ORTHO or PERSP
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

//Bezier curves
	Curve Curves[NUMCURVES];		// if you are creating a pattern of curves
	Curve key;						// if you are not

//Shaders
	GLSLProgram *Pattern;

//Key Animation
	bool keyPressed = false;
	bool keyReleased = false;

	int keyi;	//Indexing for main set of keys
	int keyj;	//Indexing for main set of keys

	int keys;	//Indexing for special keys (keys outside of main set)

//Textures
	GLuint keytextures[46];		//array of textures for each keycap [standard keycap size]
	GLuint keytextures2[20];	//second array for special keys
	int width, height = 64;
	int width2 = 96;	//For 1.25x, 1.5x keys
	int width3 = 128;	//For 2x keys and longer
	int level = 0;
	int ncomps = 3;
	int border = 0;

//OsuSphere Globals (for debugging purposes)
int		NumLngs, NumLats;
struct point* Pts;
int widthtest = 1024;
int heighttest = 512;

bool drawtext = false;


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoShadowMenu();
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );

void	Keyboard( unsigned char, int, int );
void	KbdUp ( unsigned char, int, int );

void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);

void	drawkey( struct Curve& ); // function to draw a key

void	dtkey(int, int, float, GLuint*, int, int, int); //Parameters: collumn, row, width, texture array, index, keyi, keyj				
		
void	OsuSphere(float, int, int);


// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// init all the global variables used by Display( ):

	Reset( );

	// create the display structures that will not change:

	InitLists( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	const int MS_IN_THE_ANIMATION_CYCLE = 10000;	// milliseconds in the animation loop
	int ms = glutGet(GLUT_ELAPSED_TIME);			// milliseconds since the program started
	ms %= MS_IN_THE_ANIMATION_CYCLE;				// milliseconds in the range 0 to MS_IN_THE_ANIMATION_CYCLE-1
	Time = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;        // [ 0., 1. )

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}

	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );

	// erase the background:

	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif

	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );

	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:

	//gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );
	gluLookAt(5., 6., 5.,		5., 0., 0.,		0., 1., 0.);

	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

	// set the fog parameters:
	// (this is really here to do intensity depth cueing)

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// possibly draw the axes:

	if( AxesOn == 0 )
	{
		glColor3fv( &Colors[WhichColor][1] );
		glCallList( AxesList );
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );

	// draw the current object:

	//glCallList( BoxList );
	glPushMatrix();
		Pattern->Use();

			Pattern->SetUniformVariable("uTime", Time);
			Pattern->SetUniformVariable("uKa", uKa);
			Pattern->SetUniformVariable("uKd", uKd);
			Pattern->SetUniformVariable("uKs", uKs);
			Pattern->SetUniformVariable("uColor", uColorR, uColorG, uColorB);
			Pattern->SetUniformVariable("uSpecularColor", uSpecR, uSpecG, uSpecB);
			Pattern->SetUniformVariable("uShininess", uShininess);
			glTranslatef(-1.1, -0.1, -.5);

				glCallList(caselist);

		Pattern->Use(0);
	glPopMatrix();

	// This for loop covers most of the keys
	for(int j = 0; j < 4; j++)
	{
		for(int i = 0; i < 10; i++)
		{
				dtkey(i, j, 1., keytextures, i + j*10, i, j);
		}
	}

		// Tilde
		//dtkey(-1, 0, 1., keytextures, 40);

		// Minus
		//dtkey(8, 0, 1., keytextures, 41);


		//Backspace
		glPushMatrix();
			glTranslatef(9.6, 0., 0.);
			glScalef(2., 1., 1.); //backspace is 2 unit
			glCallList(keylist);
		glPopMatrix();

		//Tab
		glPushMatrix();
			glTranslatef(-.8, 0., 0.8);
			glScalef(1.45, 1., 1.);
			glCallList(keylist);
		glPopMatrix();

		//Caps-lock
		glPushMatrix();
			glTranslatef(-.8, 0., 1.6);
			glScalef(1.9, 1., 1.);
			glCallList(keylist);
		glPopMatrix();

		//Left-shift
		glPushMatrix();
			glTranslatef(-.8, 0., 2.4);
			glScalef(2.3, 1., 1.);
			glCallList(keylist);
		glPopMatrix();

		//Right-shift
		glPushMatrix();
		glTranslatef(.8*10 + 1, 0., 2.4);
		glScalef(2.3, 1., 1.);
			glCallList(keylist);
		glPopMatrix();
		
		// Bottom-row

			for(int i = 0; i < 8; i++)
			{
				glPushMatrix();
					if(i<=3)
					{
						glTranslatef(-.8 + i * .96, 0., 3.2);
					}

					else{ glTranslatef(-.8 + i * .96 + 3.9, 0., 3.2); }
					
					if(i == 3)
					{
						glScalef(6.3, 1., 1.);
					}
					else{ glScalef(1.2, 1., 1.); }
					glCallList(keylist);
				glPopMatrix();
			}		

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	if(drawtext)
	{
		// draw some gratuitous text that just rotates on top of the scene:

		glDisable( GL_DEPTH_TEST );
		glColor3f( 0., 1., 1. );
		DoRasterString( 0., 1., 0., (char *)"Text That Moves" );

		// draw some gratuitous text that is fixed on the screen:
		//
		// the projection matrix is reset to define a scene whose
		// world coordinate system goes from 0-100 in each axis
		//
		// this is called "percent units", and is just a convenience
		//
		// the modelview matrix is reset to identity as we don't
		// want to transform these coordinates

		glDisable( GL_DEPTH_TEST );
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );
		gluOrtho2D( 0., 100.,     0., 100. );
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity( );
		glColor3f( 1., 1., 1. );
		DoRasterString( 5., 5., 0., (char *)"Text That Doesn't" );
	}

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	WhichColor = id - RED;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDebugMenu( int id )
{
	DebugOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoProjectMenu( int id )
{
	WhichProjection = id;
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:
	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:
	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(int) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Colors",        colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );

	glutKeyboardFunc( Keyboard );
	glutKeyboardUpFunc( KbdUp );
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF); //Repeat inputs off (not constantly repeating inputs if a key is held down)

	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// do this *after* opening the window and init'ing glew:

	Pattern = new GLSLProgram();
	bool valid = Pattern->Create("pattern.vert", "pattern.frag");
	if (!valid)
	{
		fprintf(stderr, "Shader cannot be created!\n");
		DoMainMenu(QUIT);
	}
	else
	{
		fprintf(stderr, "Shader created.\n");
	}
	Pattern->SetVerbose(false);

	unsigned char* tkey[46]{};	//main keys
	unsigned char* tkey2[20]{}; //special keys

	//Main keys
	tkey[0] = BmpToTexture("textures/1.bmp", &width, &height);
	tkey[1] = BmpToTexture("textures/2.bmp", &width, &height);
	tkey[2] = BmpToTexture("textures/3.bmp", &width, &height);
	tkey[3] = BmpToTexture("textures/4.bmp", &width, &height);
	tkey[4] = BmpToTexture("textures/5.bmp", &width, &height);
	tkey[5] = BmpToTexture("textures/6.bmp", &width, &height);
	tkey[6] = BmpToTexture("textures/7.bmp", &width, &height);
	tkey[7] = BmpToTexture("textures/8.bmp", &width, &height);
	tkey[8] = BmpToTexture("textures/9.bmp", &width, &height);
	tkey[9] = BmpToTexture("textures/0.bmp", &width, &height);
	tkey[10] = BmpToTexture("textures/q.bmp", &width, &height);
	tkey[11] = BmpToTexture("textures/w.bmp", &width, &height);
	tkey[12] = BmpToTexture("textures/e.bmp", &width, &height);
	tkey[13] = BmpToTexture("textures/r.bmp", &width, &height);
	tkey[14] = BmpToTexture("textures/t.bmp", &width, &height);
	tkey[15] = BmpToTexture("textures/y.bmp", &width, &height);
	tkey[16] = BmpToTexture("textures/u.bmp", &width, &height);
	tkey[17] = BmpToTexture("textures/i.bmp", &width, &height);
	tkey[18] = BmpToTexture("textures/o.bmp", &width, &height);
	tkey[19] = BmpToTexture("textures/p.bmp", &width, &height);
	tkey[20] = BmpToTexture("textures/a.bmp", &width, &height);
	tkey[21] = BmpToTexture("textures/s.bmp", &width, &height);
	tkey[22] = BmpToTexture("textures/d.bmp", &width, &height);
	tkey[23] = BmpToTexture("textures/f.bmp", &width, &height);
	tkey[24] = BmpToTexture("textures/g.bmp", &width, &height);
	tkey[25] = BmpToTexture("textures/h.bmp", &width, &height);
	tkey[26] = BmpToTexture("textures/j.bmp", &width, &height);
	tkey[27] = BmpToTexture("textures/k.bmp", &width, &height);
	tkey[28] = BmpToTexture("textures/l.bmp", &width, &height);
	tkey[29] = BmpToTexture("textures/semicolon.bmp", &width, &height);
	tkey[30] = BmpToTexture("textures/z.bmp", &width, &height);
	tkey[31] = BmpToTexture("textures/x.bmp", &width, &height);
	tkey[32] = BmpToTexture("textures/c.bmp", &width, &height);
	tkey[33] = BmpToTexture("textures/v.bmp", &width, &height);
	tkey[34] = BmpToTexture("textures/b.bmp", &width, &height);
	tkey[35] = BmpToTexture("textures/n.bmp", &width, &height);
	tkey[36] = BmpToTexture("textures/m.bmp", &width, &height);
	tkey[37] = BmpToTexture("textures/comma.bmp", &width, &height);
	tkey[38] = BmpToTexture("textures/period.bmp", &width, &height);
	tkey[39] = BmpToTexture("textures/questionmark.bmp", &width, &height);
	tkey[40] = BmpToTexture("textures/s/tilde.bmp", &width, &height);
	tkey[41] = BmpToTexture("textures/s/-.bmp", &width, &height);
	tkey[42] = BmpToTexture("textures/s/+.bmp", &width, &height);
	tkey[43] = BmpToTexture("textures/s/openbracket.bmp", &width, &height);
	tkey[44] = BmpToTexture("textures/s/closedbracket.bmp", &width, &height);
	tkey[45] = BmpToTexture("textures/s/quote.bmp", &width, &height);

	//Extra keys
	tkey2[0] = BmpToTexture("textures/s/tilde.bmp", &width, &height);
	tkey2[1] = BmpToTexture("textures/s/tab.bmp", &width3, &height);
	tkey2[2] = BmpToTexture("textures/s/caps.bmp", &width3, &height);
	tkey2[3] = BmpToTexture("textures/s/shift.bmp", &width3, &height);



	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for(int i = 0; i<46; i++)
	{
	glGenTextures(1, &keytextures[i]);

		glBindTexture(GL_TEXTURE_2D, keytextures[i]);// make the Tex0 texture current and set its parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tkey[i]);
	}

	//glGenTextures(1, &texkey2);

	//	glBindTexture(GL_TEXTURE_2D, texkey2);// make the Tex0 texture current and set its parameters
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tkey[1]);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow( MainWindow );

	// create the object:

	BoxList = glGenLists( 1 );
	glNewList( BoxList, GL_COMPILE );

		glBegin( GL_QUADS );

			glColor3f( 0., 0., 1. );
				glVertex3f( -dx, -dy,  dz );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f( -dx,  dy,  dz );

				glVertex3f( -dx, -dy, -dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx, -dy, -dz );

			glColor3f( 1., 0., 0. );
				glVertex3f(  dx, -dy,  dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f(  dx,  dy,  dz );

				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f( -dx,  dy, -dz );
				glVertex3f( -dx, -dy, -dz );

			glColor3f( 0., 1., 0. );
				glVertex3f( -dx,  dy,  dz );
				glVertex3f(  dx,  dy,  dz );
				glVertex3f(  dx,  dy, -dz );
				glVertex3f( -dx,  dy, -dz );

				glVertex3f( -dx, -dy,  dz );
				glVertex3f( -dx, -dy, -dz );
				glVertex3f(  dx, -dy, -dz );
				glVertex3f(  dx, -dy,  dz );

		glEnd( );

	glEndList( );

	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1. );
		glLineWidth( 1. );
	glEndList( );

	keylist = glGenLists( 1 );
	glNewList(keylist, GL_COMPILE);
		drawkey( key );
	glEndList();

	caselist = glGenLists( 1 );
	glNewList( caselist, GL_COMPILE );

		glBegin(GL_QUADS);
		//glColor3f(.25, .25, .25);

		glColor3f(.25, .25, .25);
		glNormal3f(-1., 0., 0.);
		glVertex3f(0., 0., 0.);
		glNormal3f(-1., 0., 0.);
		glVertex3f(0., CASE_HEIGHT, 0.);
		glNormal3f(-1., 0., 0.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(-1., 0., 0.);
		glVertex3f(0., 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_THICKNESS, 0., 0.);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_THICKNESS, CASE_HEIGHT, 0.);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_THICKNESS, 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, 0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_THICKNESS, CASE_HEIGHT, 0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_THICKNESS, 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_THICKNESS, 0., CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(-1., 0., 0.);
		glVertex3f(CASE_WIDTH, 0., 0.);
		glNormal3f(-1., 0., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, 0.);
		glNormal3f(-1., 0., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(-1., 0., 0.);
		glVertex3f(CASE_WIDTH, 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., 0.);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, 0.);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH);
		glNormal3f(1., 0., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH);

		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, 0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT,  0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT,  CASE_LENGTH);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, CASE_LENGTH);

		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., CASE_LENGTH);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., CASE_LENGTH);

		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., 0.);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH, 0., CASE_THICKNESS);
		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., CASE_THICKNESS);

		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, 0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, 0.);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, CASE_THICKNESS);
		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, CASE_THICKNESS);

		glNormal3f(0., 0., -1.);
		glVertex3f(0., 0., 0.);
		glNormal3f(0., 0., -1.);
		glVertex3f(0., CASE_HEIGHT, 0.);
		glNormal3f(0., 0., -1.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, 0.);
		glNormal3f(0., 0., -1.);
		glVertex3f(CASE_WIDTH, 0., 0.);

		glNormal3f(0., 0., 1.);
		glVertex3f(0., 0., CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(0., CASE_HEIGHT, CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH, CASE_HEIGHT, CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH, 0., CASE_THICKNESS);

		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., CASE_LENGTH);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH);
		glNormal3f(0., -1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., -1., 0.);
		glVertex3f(0., 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH);
		glNormal3f(0., 1., 0.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 1., 0.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(0., 0., 1.);
		glVertex3f(0., 0., CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH + CASE_THICKNESS);

		glNormal3f(0., 0., -1.);
		glVertex3f(0., 0., CASE_LENGTH);
		glNormal3f(0., 0., -1.);
		glVertex3f(0., CASE_HEIGHT, CASE_LENGTH);
		glNormal3f(0., 0., -1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, CASE_HEIGHT, CASE_LENGTH);
		glNormal3f(0., 0., -1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH);

		//Bottom
		glColor3f(.15, .15, .15);
		glNormal3f(0., 0., 1.);
		glVertex3f(0., 0., 0.);
		glNormal3f(0., 0., 1.);
		glVertex3f(0., 0., CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., CASE_LENGTH + CASE_THICKNESS);
		glNormal3f(0., 0., 1.);
		glVertex3f(CASE_WIDTH + CASE_THICKNESS, 0., 0. );

		glEnd();

	glEndList();
}


// the keyboard callback:
// for keyPRESSES (down)
void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{

	case ESCAPE:
		drawtext = true; // will not return here
		break;			// happy compiler

	case '1':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 0;
		keyj = 0;
		break;
	case '2':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 1;
		keyj = 0;
		break;
	case '3':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 2;
		keyj = 0;
		break;
	case '4':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 3;
		keyj = 0;
		break;
	case '5':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 4;
		keyj = 0;
		break;
	case '6':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 5;
		keyj = 0;
		break;
	case '7':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 6;
		keyj = 0;
		break;
	case '8':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 7;
		keyj = 0;
		break;
	case '9':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 8;
		keyj = 0;
		break;

	case '0':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 9;
		keyj = 0;
		break;
	case 'q':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 0;
		keyj = 1;
		break;
	case 'w':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 1;
		keyj = 1;
		break;
	case 'e':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 2;
		keyj = 1;
		break;
	case 'r':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 3;
		keyj = 1;
		break;
	case 't':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 4;
		keyj = 1;
		break;
	case 'y':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 5;
		keyj = 1;
		break;
	case 'u':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 6;
		keyj = 1;
		break;
	case 'i':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 7;
		keyj = 1;
		break;
	case 'o':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 8;
		keyj = 1;
		break;
	case 'p':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 9;
		keyj = 1;
		break;
	case 'a':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 0;
		keyj = 2;
		break;
	case 's':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 1;
		keyj = 2;
		break;
	case 'd':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 2;
		keyj = 2;
		break;
	case 'f':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 3;
		keyj = 2;
		break;
	case 'g':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 4;
		keyj = 2;
		break;
	case 'h':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 5;
		keyj = 2;
		break;
	case 'j':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 6;
		keyj = 2;
		break;
	case 'k':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 7;
		keyj = 2;
		break;
	case 'l':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 8;
		keyj = 2;
		break;
	case ';':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 9;
		keyj = 2;
		break;
	case 'z':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 0;
		keyj = 3;
		break;
	case 'x':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 1;
		keyj = 3;
		break;
	case 'c':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 2;
		keyj = 3;
		break;
	case 'v':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 3;
		keyj = 3;
		break;
	case 'b':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 4;
		keyj = 3;
		break;
	case 'n':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 5;
		keyj = 3;
		break;
	case 'm':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 6;
		keyj = 3;
		break;
	case ',':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 7;
		keyj = 3;
		break;
	case '.':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 8;
		keyj = 3;
		break;
	case '/':
		printf(" Key pressed \n ");
		keyReleased = false;
		keyPressed = true;
		keyi = 9;
		keyj = 3;
		break;

		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// keyboard callback for key RELEASES
void KbdUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case ESCAPE:
		DoMainMenu(QUIT); // will not return here
		break;			// happy compiler

		case '1':
			printf( " Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 0;
			keyj = 0;
			break;
		case '2':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 1;
			keyj = 0;
			break;
		case '3':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 2;
			keyj = 0;
			break;
		case '4':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 3;
			keyj = 0;
			break;
		case '5':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 4;
			keyj = 0;
			break;
		case '6':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 5;
			keyj = 0;
			break;
		case '7':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 6;
			keyj = 0;
			break;
		case '8':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 7;
			keyj = 0;
			break;
		case '9':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 8;
			keyj = 0;
			break;
		case '0':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 9;
			keyj = 0;
			break;

		case 'q':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 0;
			keyj = 1;
			break;
		case 'w':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 1;
			keyj = 1;
			break;
		case 'e':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 2;
			keyj = 1;
			break;
		case 'r':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 3;
			keyj = 1;
			break;
		case 't':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 4;
			keyj = 1;
			break;
		case 'y':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 5;
			keyj = 1;
			break;
		case 'u':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 6;
			keyj = 1;
			break;
		case 'i':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 7;
			keyj = 1;
			break;
		case 'o':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 8;
			keyj = 1;
			break;
		case 'p':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 9;
			keyj = 1;
			break;
		case 'a':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 0;
			keyj = 2;
			break;
		case 's':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 1;
			keyj = 2;
			break;
		case 'd':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 2;
			keyj = 2;
			break;
		case 'f':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 3;
			keyj = 2;
			break;
		case 'g':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 4;
			keyj = 2;
			break;
		case 'h':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 5;
			keyj = 2;
			break;
		case 'j':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 6;
			keyj = 2;
			break;
		case 'k':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 7;
			keyj = 2;
			break;
		case 'l':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 8;
			keyj = 2;
			break;
		case ';':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 9;
			keyj = 2;
			break;
		case 'z':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 0;
			keyj = 3;
			break;
		case 'x':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 1;
			keyj = 3;
			break;
		case 'c':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 2;
			keyj = 3;
			break;
		case 'v':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 3;
			keyj = 3;
			break;
		case 'b':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 4;
			keyj = 3;
			break;
		case 'n':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 5;
			keyj = 3;
			break;
		case 'm':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 6;
			keyj = 3;
			break;
		case ',':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 7;
			keyj = 3;
			break;
		case '.':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 8;
			keyj = 3;
			break;
		case '/':
			printf(" Key released \n ");
			keyPressed = false;
			keyReleased = true;
			keyi = 9;
			keyj = 3;
			break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%key' (0x%0x)\n", key, key);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();

}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );

	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since the window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

// Function to draw a single keycap on the keyboard
// This function uses a modified version of the provided bezier
// curve drawing program in order to have smooth indents on the keys
void drawkey(struct Curve& k) {

	k.p0.x = 0.;
	k.p0.y = 0.75;
	k.p0.z = 0.;

	k.p1.x = 0.25;
	k.p1.y = 0.65;
	k.p1.z = 0.;

	k.p2.x = 0.5;
	k.p2.y = 0.65;
	k.p2.z =0.;

	k.p3.x = 0.75;
	k.p3.y = 0.75;
	k.p3.z = 0.;

	glLineWidth(3.);
	glColor3f(.75, .75, 0.75);

	//Draw the top of the key
	glBegin(GL_TRIANGLE_STRIP);
		for (int it = 0; it <= NUMPOINTS; it++)
		{
			float t = (float)it / (float)NUMPOINTS;
			float omt = 1.f - t;
			float x = omt * omt * omt * k.p0.x + 3.f * t * omt * omt * k.p1.x + 3.f * t * t * omt * k.p2.x + t * t * t * k.p3.x;
			float y = omt * omt * omt * k.p0.y + 3.f * t * omt * omt * k.p1.y + 3.f * t * t * omt * k.p2.y + t * t * t * k.p3.y;
			float z = omt * omt * omt * k.p0.z + 3.f * t * omt * omt * k.p1.z + 3.f * t * t * omt * k.p2.z + t * t * t * k.p3.z;

			glNormal3f(x, y, z);
			
			glTexCoord2f(x * 4/3, z + 1.);
			glVertex3f(x, y, z);

			glNormal3f(x, y, z + 0.75);
			glTexCoord2f(x * 4/3, z);
			glVertex3f(x, y, z + 0.75);
		}
	glEnd();


	//Draw the back side of the key
	glBegin(GL_TRIANGLE_STRIP);
	for (int it = 0; it <= NUMPOINTS; it++)
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt * omt * omt * k.p0.x + 3.f * t * omt * omt * k.p1.x + 3.f * t * t * omt * k.p2.x + t * t * t * k.p3.x;
		float y = omt * omt * omt * k.p0.y + 3.f * t * omt * omt * k.p1.y + 3.f * t * t * omt * k.p2.y + t * t * t * k.p3.y;
		float z = omt * omt * omt * k.p0.z + 3.f * t * omt * omt * k.p1.z + 3.f * t * t * omt * k.p2.z + t * t * t * k.p3.z;

		glNormal3f(x, y, -1.);
		glVertex3f(x, y, 0);
		glVertex3f(x, 0, 0);

	}
	glEnd();

	//Draw the front side of the key
	glBegin(GL_TRIANGLE_STRIP);
	for (int it = 0; it <= NUMPOINTS; it++)
	{
		float t = (float)it / (float)NUMPOINTS;
		float omt = 1.f - t;
		float x = omt * omt * omt * k.p0.x + 3.f * t * omt * omt * k.p1.x + 3.f * t * t * omt * k.p2.x + t * t * t * k.p3.x;
		float y = omt * omt * omt * k.p0.y + 3.f * t * omt * omt * k.p1.y + 3.f * t * t * omt * k.p2.y + t * t * t * k.p3.y;
		float z = omt * omt * omt * k.p0.z + 3.f * t * omt * omt * k.p1.z + 3.f * t * t * omt * k.p2.z + t * t * t * k.p3.z;

		glNormal3f(x, y, 1.);
		glVertex3f(x, y, .75);
		glVertex3f(x, 0, .75);

	}
	glEnd();

	// Draw the left and right side of the keys
	glBegin(GL_QUADS);

		// Left
		glVertex3f(0., 0., 0.);
		glVertex3f(0., 0., 0.75);
		glVertex3f(0., k.p3.y, 0.75);
		glVertex3f(0., k.p3.y, 0.);

		// Right
		glVertex3f(k.p3.x, 0., 0.);
		glVertex3f(k.p3.x, 0., 0.75);
		glVertex3f(k.p3.x, k.p3.y, 0.75);
		glVertex3f(k.p3.x, k.p3.y, 0.);

	glEnd();
}

void	dtkey(int i, int j, float w, GLuint* kt, int idx, int k1, int k2) {

		glPushMatrix();

		glTranslatef(0.8 * i + (float(j) / 3), 0., 0.8 * j);

		if (keyPressed && k1 == keyi && k2 == keyj)
		{
			//glTranslatef(0., -0.75, 0.); // originale
			glTranslatef(0., -0.5, 0.);
		}

		if (keyReleased && k1 == keyi && k2 == keyj)
		{
			glTranslatef(0., 0., 0.);
		}

		Pattern->Use();
		Pattern->SetUniformVariable("uTime", Time);
		Pattern->SetUniformVariable("uKa", uKa);
		Pattern->SetUniformVariable("uKd", uKd);
		Pattern->SetUniformVariable("uKs", uKs);
		Pattern->SetUniformVariable("uColor", uColorR, uColorG, uColorB);
		Pattern->SetUniformVariable("uSpecularColor", uSpecR, uSpecG, uSpecB);
		Pattern->SetUniformVariable("uShininess", uShininess);

		glActiveTexture(GL_TEXTURE6);                 // use texture unit 5
		glBindTexture(GL_TEXTURE_2D, kt[idx]);
		Pattern->SetUniformVariable("uTexUnit", 6);   // tell your shader program you are using texture unit 5

		glCallList(keylist);

	Pattern->Use(0);

		glPopMatrix();
}

// read a BMP file into a Texture:

#define VERBOSE		false
#define BMP_MAGIC_NUMBER	0x4d42
#ifndef BI_RGB
#define BI_RGB			0
#define BI_RLE8			1
#define BI_RLE4			2
#endif


// bmp file header:
struct bmfh
{
	short bfType;		// BMP_MAGIC_NUMBER = "BM"
	int bfSize;		// size of this file in bytes
	short bfReserved1;
	short bfReserved2;
	int bfOffBytes;		// # bytes to get to the start of the per-pixel data
} FileHeader;

// bmp info header:
struct bmih
{
	int biSize;		// info header size, should be 40
	int biWidth;		// image width
	int biHeight;		// image height
	short biPlanes;		// #color planes, should be 1
	short biBitCount;	// #bits/pixel, should be 1, 4, 8, 16, 24, 32
	int biCompression;	// BI_RGB, BI_RLE4, BI_RLE8
	int biSizeImage;
	int biXPixelsPerMeter;
	int biYPixelsPerMeter;
	int biClrUsed;		// # colors in the palette
	int biClrImportant;
} InfoHeader;

// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp;
#ifdef _WIN32
        errno_t err = fopen_s( &fp, filename, "rb" );
        if( err != 0 )
        {
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
        }
#else
	FILE *fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}
#endif

	FileHeader.bfType = ReadShort( fp );

	// if bfType is not BMP_MAGIC_NUMBER, the file is not a bmp:

	if( VERBOSE ) fprintf( stderr, "FileHeader.bfType = 0x%0x = \"%c%c\"\n",
			FileHeader.bfType, FileHeader.bfType&0xff, (FileHeader.bfType>>8)&0xff );
	if( FileHeader.bfType != BMP_MAGIC_NUMBER )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}

	FileHeader.bfSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfSize = %d\n", FileHeader.bfSize );

	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );

	FileHeader.bfOffBytes = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "FileHeader.bfOffBytes = %d\n", FileHeader.bfOffBytes );

	InfoHeader.biSize = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSize = %d\n", InfoHeader.biSize );
	InfoHeader.biWidth = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biWidth = %d\n", InfoHeader.biWidth );
	InfoHeader.biHeight = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biHeight = %d\n", InfoHeader.biHeight );

	const int nums = InfoHeader.biWidth;
	const int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biPlanes = %d\n", InfoHeader.biPlanes );

	InfoHeader.biBitCount = ReadShort( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biBitCount = %d\n", InfoHeader.biBitCount );

	InfoHeader.biCompression = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biCompression = %d\n", InfoHeader.biCompression );

	InfoHeader.biSizeImage = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biSizeImage = %d\n", InfoHeader.biSizeImage );

	InfoHeader.biXPixelsPerMeter = ReadInt( fp );
	InfoHeader.biYPixelsPerMeter = ReadInt( fp );

	InfoHeader.biClrUsed = ReadInt( fp );
	if( VERBOSE )	fprintf( stderr, "InfoHeader.biClrUsed = %d\n", InfoHeader.biClrUsed );

	InfoHeader.biClrImportant = ReadInt( fp );

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	// pixels will be stored bottom-to-top, left-to-right:
	unsigned char *texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\n" );
		return NULL;
	}

	// extra padding bytes:

	int requiredRowSizeInBytes = 4 * ( ( InfoHeader.biBitCount*InfoHeader.biWidth + 31 ) / 32 );
	if( VERBOSE )	fprintf( stderr, "requiredRowSizeInBytes = %d\n", requiredRowSizeInBytes );

	int myRowSizeInBytes = ( InfoHeader.biBitCount*InfoHeader.biWidth + 7 ) / 8;
	if( VERBOSE )	fprintf( stderr, "myRowSizeInBytes = %d\n", myRowSizeInBytes );

	int oldNumExtra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;
	if( VERBOSE )	fprintf( stderr, "Old NumExtra padding = %d\n", oldNumExtra );

	int numExtra = requiredRowSizeInBytes - myRowSizeInBytes;
	if( VERBOSE )	fprintf( stderr, "New NumExtra padding = %d\n", numExtra );

	// this function does not support compression:

	if( InfoHeader.biCompression != 0 )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	
	// we can handle 24 bits of direct color:
	if( InfoHeader.biBitCount == 24 )
	{
		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	// we can also handle 8 bits of indirect color:
	if( InfoHeader.biBitCount == 8 && InfoHeader.biClrUsed == 256 )
	{
		struct rgba32
		{
			unsigned char r, g, b, a;
		};
		struct rgba32 *colorTable = new struct rgba32[ InfoHeader.biClrUsed ];

		rewind( fp );
		fseek( fp, sizeof(struct bmfh) + InfoHeader.biSize - 2, SEEK_SET );
		for( int c = 0; c < InfoHeader.biClrUsed; c++ )
		{
			colorTable[c].r = fgetc( fp );
			colorTable[c].g = fgetc( fp );
			colorTable[c].b = fgetc( fp );
			colorTable[c].a = fgetc( fp );
			if( VERBOSE )	fprintf( stderr, "%4d:\t0x%02x\t0x%02x\t0x%02x\t0x%02x\n",
				c, colorTable[c].r, colorTable[c].g, colorTable[c].b, colorTable[c].a );
		}

		rewind( fp );
		fseek( fp, FileHeader.bfOffBytes, SEEK_SET );
		int t;
		unsigned char *tp;
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				int index = fgetc( fp );
				*(tp+0) = colorTable[index].r;	// r
				*(tp+1) = colorTable[index].g;	// g
				*(tp+2) = colorTable[index].b;	// b
			}

			for( int e = 0; e < numExtra; e++ )
			{
				fgetc( fp );
			}
		}

		delete[ ] colorTable;
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	const unsigned char b2 = fgetc( fp );
	const unsigned char b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	const unsigned char b0 = fgetc( fp );
	const unsigned char b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}


//************************
// OsuSphere (debugging)
//************************

struct point
{
	float x, y, z;		// coordinates
	float nx, ny, nz;	// surface normal
	float s, t;		// texture coords
};

inline
struct point*
	PtsPointer(int lat, int lng)
{
	if (lat < 0)	lat += (NumLats - 1);
	if (lng < 0)	lng += (NumLngs - 0);
	if (lat > NumLats - 1)	lat -= (NumLats - 1);
	if (lng > NumLngs - 1)	lng -= (NumLngs - 0);
	return &Pts[NumLngs * lat + lng];
}

inline
void
DrawPoint(struct point* p)
{
	glNormal3fv(&p->nx);
	glTexCoord2fv(&p->s);
	glVertex3fv(&p->x);
}

void
OsuSphere(float radius, int slices, int stacks)
{
	// set the globals:

	NumLngs = slices;
	NumLats = stacks;
	if (NumLngs < 3)
		NumLngs = 3;
	if (NumLats < 3)
		NumLats = 3;

	// allocate the point data structure:

	Pts = new struct point[NumLngs * NumLats];

	// fill the Pts structure:

	for (int ilat = 0; ilat < NumLats; ilat++)
	{
		float lat = -M_PI / 2. + M_PI * (float)ilat / (float)(NumLats - 1);	// ilat=0/lat=0. is the south pole
											// ilat=NumLats-1, lat=+M_PI/2. is the north pole
		float xz = cosf(lat);
		float  y = sinf(lat);
		for (int ilng = 0; ilng < NumLngs; ilng++)				// ilng=0, lng=-M_PI and
											// ilng=NumLngs-1, lng=+M_PI are the same meridian
		{
			float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
			float x = xz * cosf(lng);
			float z = -xz * sinf(lng);
			struct point* p = PtsPointer(ilat, ilng);
			p->x = radius * x;
			p->y = radius * y;
			p->z = radius * z;
			p->nx = x;
			p->ny = y;
			p->nz = z;
			p->s = (lng + M_PI) / (2. * M_PI);
			p->t = (lat + M_PI / 2.) / M_PI;
		}
	}

	struct point top, bot;		// top, bottom points

	top.x = 0.;		top.y = radius;	top.z = 0.;
	top.nx = 0.;		top.ny = 1.;		top.nz = 0.;
	top.s = 0.;		top.t = 1.;

	bot.x = 0.;		bot.y = -radius;	bot.z = 0.;
	bot.nx = 0.;		bot.ny = -1.;		bot.nz = 0.;
	bot.s = 0.;		bot.t = 0.;

	// connect the north pole to the latitude NumLats-2:

	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = 0; ilng < NumLngs; ilng++)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		top.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&top);
		struct point* p = PtsPointer(NumLats - 2, ilng);	// ilat=NumLats-1 is the north pole
		DrawPoint(p);
	}
	glEnd();

	// connect the south pole to the latitude 1:

	glBegin(GL_TRIANGLE_STRIP);
	for (int ilng = NumLngs - 1; ilng >= 0; ilng--)
	{
		float lng = -M_PI + 2. * M_PI * (float)ilng / (float)(NumLngs - 1);
		bot.s = (lng + M_PI) / (2. * M_PI);
		DrawPoint(&bot);
		struct point* p = PtsPointer(1, ilng);					// ilat=0 is the south pole
		DrawPoint(p);
	}
	glEnd();

	// connect the horizontal strips:

	for (int ilat = 2; ilat < NumLats - 1; ilat++)
	{
		struct point* p;
		glBegin(GL_TRIANGLE_STRIP);
		for (int ilng = 0; ilng < NumLngs; ilng++)
		{
			p = PtsPointer(ilat, ilng);
			DrawPoint(p);
			p = PtsPointer(ilat - 1, ilng);
			DrawPoint(p);
		}
		glEnd();
	}

	// clean-up:

	delete[] Pts;
	Pts = NULL;
}