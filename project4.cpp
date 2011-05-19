// Awesome 3D scene project for CG1.
// Author: Max Bogue
// Date: 5/8/2011

#if defined(__APPLE__) && defined(__MACH__)  // for mac
#   include <GLUT/glut.h>
#else // for others
#   include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <cmath>
#include "tga_loader.cpp"

#define PI 3.14159265

using namespace std;

// Not actually sure what these should default to..
int width = 1024;
int height = 768;

bool solid;             // Whether shapes are solid or wireframe.

GLdouble ex, ey, ez;         // Eye coords.
GLdouble dx, dy, dz;      // Delta from the eye to the reference.

GLdouble rotOffset[2];    // The rotations of all spheres.
GLdouble cubeEdgeScale;   // How scaled the cube edges are.
bool cubeEdgeGrow;      // Whether the cube edges are grown (growing) or not.
bool fogEnabled;
bool lightsEnabled;

// NURB data.
GLUnurbsObj *nurb;
GLfloat nurbCPs[4][4][3];
GLfloat nurbKnots[8] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};

struct Material {
    GLfloat c[3];
    GLfloat a[4];
    GLfloat d[4];
    GLfloat s[4];
    GLfloat e;
};

Material enclosingSphereMat = {
    { 10/255.0, 85/255.0, 175/255.0 },
    { 10/255.0, 85/255.0, 175/255.0, 1.0},
    { 10/255.0, 85/255.0, 175/255.0, 1.0},
    { 10/255.0, 85/255.0, 175/255.0, 1.0},
    50.0,
};

Material outerSpheresMat = {
    { 0/255.0, 204/255.0, 255/255.0 },
    { 0/255.0, 204/255.0, 255/255.0, 1.0},
    { 0/255.0, 204/255.0, 255/255.0, 1.0},
    { 0/255.0, 204/255.0, 255/255.0, 1.0},
    80.0,
};

Material innerSpheresMat = {
    { 204/255.0, 255/255.0, 0/255.0 },
    { 204/255.0, 255/255.0, 0/255.0, 1.0},
    { 204/255.0, 255/255.0, 0/255.0, 1.0},
    { 204/255.0, 255/255.0, 0/255.0, 1.0},
    80.0,
};

Material torusMat = {
    { 255/255.0, 0/255.0, 204/255.0 },
    { 255/255.0, 0/255.0, 204/255.0, 1.0},
    { 255/255.0, 0/255.0, 204/255.0, 1.0},
    { 255/255.0, 0/255.0, 204/255.0, 1.0},
    20.0,
};

int t1w = 512;
int t1h = 256;

GLuint textures[4];


// Sets all global values to their defaults.
void reset() {
    solid = false;
    lightsEnabled = true;
    fogEnabled = false;
    ex = -20;
    ey = 0;
    ez = 0;
    dx = 1;
    dy = 0;
    dz = 0;
    rotOffset[0] = 0;
    rotOffset[1] = 0;
    cubeEdgeScale = 0;
    cubeEdgeGrow = false;
}

// Loads a .tga texture using rgb_tga.
int loadTexture(char * filename, int w, int h) {
    GLubyte *data = rgb_tga(filename, &w, &h);
    // glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB,
                                          GL_UNSIGNED_BYTE, data );

    free( data );
    return 1;
}

// Initializes the NURB control points array.
// Code taken from the nurb.c example and tweaked to my needs.
void initNurbCPs() {
	for (int u = 0; u < 4; u++) {
		for (int v = 0; v < 4; v++) {
			nurbCPs[u][v][0] = 3.0*((GLfloat)u - 1.5);
			nurbCPs[u][v][1] = 3.0*((GLfloat)v - 1.5);

			if ( (u == 1 || u == 2) && (v == 1 || v == 2))
				nurbCPs[u][v][2] = 4.5;
			else
				nurbCPs[u][v][2] = -4.5;
		}
	} 
}

// Initialize globals and GL.
void init(void) {
    reset();
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    
    // AZIZ! LIGHT!
    glEnable( GL_LIGHTING );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE );
    GLfloat light0Position[4] = { 0.0, 1.0, 0.0, 0.0 };
    glLightfv( GL_LIGHT0, GL_POSITION, light0Position );
    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );
    
    // Fog config. 
    glFogi( GL_FOG_MODE, GL_LINEAR );
    GLfloat fogColor[4] = { 0.0, 0.0, 0.0, 0.5 };  // fog color - gray
    glFogfv( GL_FOG_COLOR, fogColor );
    glFogf( GL_FOG_DENSITY, 1.0 );
    glHint( GL_FOG_HINT, GL_NICEST );
    glFogf( GL_FOG_START, 5.0 );
    glFogf( GL_FOG_END, 50.0 );
    
    // NURB initialization.
	nurb = gluNewNurbsRenderer();
	gluNurbsProperty(nurb, GLU_SAMPLING_TOLERANCE, 25.0);
	gluNurbsProperty(nurb, GLU_DISPLAY_MODE, GLU_FILL);
    initNurbCPs();
    

    glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
    // unsigned char* tex1Data = rgb_tga("tex1.tga", &t1w, &t1h);
    glGenTextures(1, textures);
    
    // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    // glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    // glEnable(GL_TEXTURE_GEN_S);
    // glEnable(GL_TEXTURE_GEN_T);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    if (!loadTexture("tex1.tga", 256, 256)) {
        printf("Error loading texture!");
    }
}

/**
 * Constructs a series of spheres positioned in a ring.
 * @param y     The height of the ring.
 * @param r     The radius of the ring.
 * @param sr    The radius of each sphere.
 */
void makeSphereRing(GLfloat y, GLfloat r, int sr) {
    glPushMatrix();
    int n = (int) (PI * r / sr / 2);
    GLfloat angle = 360.0 / n;
    glTranslatef(0,y,0);
    for (int i = 0; i < n; i++) {
        glRotatef(angle, 0.0, 1.0, 0.0);
        glPushMatrix();
        glTranslatef(r,0,0);
        if (solid) {
            glutSolidSphere(sr, 15, 15);
        } else {
            glutWireSphere(sr, 15, 15);
        }
        glPopMatrix();
    }
    glPopMatrix();
}

/**
 * Constructs a series of sphere rings positioned in a sphere.
 * @param r     The radius of the ring.
 * @param sr    The radius of each sphere.
 */
void makeSphereSphere(GLfloat r, int sr) {
    int n = (int) (r / sr / 2);
    GLfloat yStep = 2 * r / n;
    GLfloat y = yStep / 2 - r;
    for (int i = 0; i < n; i++) {
        glPushMatrix();
        glRotatef(rotOffset[i % 2], 0.0, 1.0, 0.0);
        GLfloat x = sqrt(r*r - y*y);
        makeSphereRing(y, x, sr);
        y += yStep;
        glPopMatrix();
    }
}

void makeCube() {
    glScalef(0.5, 0.5, 0.5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glBegin(GL_QUADS);
    	// Front Face
    	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    	// Back Face                                              
    	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    	// Top Face                                               
    	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    	// Bottom Face                                            
    	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    	// Right face                                             
    	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
    	glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
    	glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
    	glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
    	// Left Face                                              
    	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    	glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
    	glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
    	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Constructs a series of cubes positioned such that when stretched out they
 * form the outline of a larger cube.
 * @param dist  The distance in each direction from the origin a cube is.
 * @param size  The size of each cube.
 */
void makeCubes(int dist, int size) {
    
    for (int i = 0; i < 12; i++) {
        glPushMatrix();
        int x, y, z;
        int d = i / 4;
        int r = i % 4;
        x = d == 0 ? dist : (d == 1 ? -dist : 0);
        if (i < 8) {
            y = r == 0 ? dist : (r == 1 ? -dist : 0);
            z = r == 2 ? dist : (r == 3 ? -dist : 0);
        } else {
            y = r < 2 ? dist : -dist;
            z = r % 2 ? -dist : dist;
        }
        glTranslatef(x, y, z);
        GLfloat s = cubeEdgeScale * 2.0 * dist / size + 1;
        glScalef(x ? 1 : s, y ? 1 : s, z ? 1 : s);
        if (solid) {
            
            // glBindTexture(GL_TEXTURE_2D, textures[0]);
            // glutSolidCube(size);
            makeCube();
        } else {
            glutWireCube(size);
        }
        glPopMatrix();
    }
}

// Makes a spinning torus.
void makeTorus() {
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    glRotatef(rotOffset[1] * 4, 0, 1, 0);
    if (solid) {
        glutSolidTorus(0.4, 3.0, 30, 50);
    } else {
        glutWireTorus(0.4, 3.0, 30, 50);
    }
    glPopMatrix();
}

// Makes NURB, harbinger of doom!
void makeNurb() {
    glPushMatrix();
    glTranslatef(0, -46.5, 0);
    glRotatef(270, 1, 0, 0);
    gluBeginSurface(nurb);
    gluNurbsSurface(nurb,
        8, nurbKnots,
        8, nurbKnots,
        4 * 3,
        3,
        &nurbCPs[0][0][0],
        4, 4,
        GL_MAP2_VERTEX_3);
    gluEndSurface(nurb);
    glPopMatrix();
}

void setMaterial(Material m) {
    glColor3f(m.c[0], m.c[1], m.c[2]);
    glMaterialfv( GL_FRONT, GL_AMBIENT,   m.a );
    glMaterialfv( GL_FRONT, GL_DIFFUSE,   m.d );
    glMaterialfv( GL_FRONT, GL_SPECULAR,  m.s );
    glMaterialf ( GL_FRONT, GL_SHININESS, m.e );
    glMaterialfv( GL_BACK, GL_AMBIENT,   m.a );
    glMaterialfv( GL_BACK, GL_DIFFUSE,   m.d );
    glMaterialfv( GL_BACK, GL_SPECULAR,  m.s );
    glMaterialf ( GL_BACK, GL_SHININESS, m.e );
}

// General construction of the world; called for each viewport.
void makeWorld() {
    glPushMatrix();
    
    // Make the enclosing sphere.
    setMaterial(enclosingSphereMat);
    if (solid) {
        glutSolidSphere(50, 100, 100);
    } else {
        glutWireSphere(50, 100, 100);
    }
    
    // Make the sphere of spheres touching the outer sphere.
    setMaterial(outerSpheresMat);
    makeSphereSphere(47, 5);
    
    // Make the inner sphere of spheres.
    setMaterial(innerSpheresMat);
    makeSphereSphere(10, 1);
    
    // Make the cubes and torus.
    setMaterial(torusMat);
    makeCubes(15, 1);
    makeTorus();
    
    glPopMatrix();
}

// The main display function.
void display(void) {
    
    // Settings.
    if (fogEnabled) {
        glEnable(GL_FOG);
    } else {
        glDisable( GL_FOG );
    }
    if (lightsEnabled) {
        glEnable( GL_LIGHTING );
    } else {
        glDisable( GL_LIGHTING );
    }
    
    // Main viewport.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    gluPerspective(60, width / (GLdouble) height, 1, 120);
    glMatrixMode(GL_MODELVIEW);
    glScissor(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    gluLookAt(ex, ey, ez, ex + dx, ey - dy, ez + dz, 0.0, 1.0, 0.0);
    makeWorld();
    
    // This is placed outside of makeWorld because if it's created in the
    // rear view then looking up causes the program to lock.
    makeNurb();
    
    // Upper-right reverse viewport.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(width * 0.75, height * 0.75, width * 0.25, height * 0.25);
    gluPerspective(60, width / (GLdouble) height, 1, 100);
    glMatrixMode(GL_MODELVIEW);
    glScissor(width * 0.75, height * 0.75, width * 0.25, height * 0.25);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(ex, ey, ez, ex - dx, ey + dy, ez - dz, 0.0, 1.0, 0.0);
    makeWorld();
    
    // The crucial end functions.
    glFlush();
    glutSwapBuffers();
    
}

// The scaling of the cube edges happens here.
void scaleCubeEdges(int v) {
    bool cond = false;
    if (cubeEdgeGrow && cubeEdgeScale < 1.0) {
        cubeEdgeScale += 0.02;
        cond = cubeEdgeScale < 1.0;
    } else if (!cubeEdgeGrow && cubeEdgeScale > 0.0) {
        cubeEdgeScale -= 0.02;
        cond = cubeEdgeScale > 0.0;
    }
    if (cond) {
        glutTimerFunc(5, scaleCubeEdges, 0);
    }
}

// Called on mouse click.
void mouse( int button, int state, int x, int y ) {
    if (state == GLUT_UP) {
        cubeEdgeGrow = !cubeEdgeGrow;
        scaleCubeEdges(0);
        glutPostRedisplay();
    }
}

// Called on mouse motion.
void motion( int x, int y ) {
    // Cap x and y at the width and height of the screen.
    // Don't judge me because of my nested ternary operators.
    x = x < 0 ? 0 : (x > width ? width : x);
    y = y < 0 ? 0 : (y > height ? height : y);
    dy = y - height / 2.0;
    dz = x - width / 2.0;
    dx = ((height / 2.0 - abs(dy)) + (width / 2.0 - abs(dz))) / 2;
    GLdouble norm = sqrt(dx*dx + dy*dy + dz*dz);
    dx /= norm;
    dy /= norm;
    dz /= norm;
    glutPostRedisplay( );
}

// Repeating timer.  Rotates the sphere spheres.
void timer(int v) {
    rotOffset[0] = (rotOffset[0] + 0.25);
    rotOffset[1] = (rotOffset[1] + 359.5);
    if (rotOffset[0] >= 360) rotOffset[0] -= 360;
    if (rotOffset[1] >= 360) rotOffset[1] -= 360;
    glutTimerFunc(15, timer, 0);
    glutPostRedisplay();
}

// Called on keypress.
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': ex += dx; ey -= dy; ez += dz; break;
        case 's': ex -= dx; ey += dy; ez -= dz; break;
        case 'a': ex += dz; ez -= dx; break;
        case 'd': ex -= dz; ez += dx; break;
        case ' ': ey++; break;
        case 'z': ey--; break;
        case '1': solid = false; break;
        case '2': solid = true; break;
        case 'r': reset(); break;
        case 'q': exit(0);
        case 'f': fogEnabled = !fogEnabled; break;
        case 'l': lightsEnabled = !lightsEnabled; break;
    }
}

// Called when the window changes sizes.
void reshape(int w, int h) {
    width = w;
    height = h;
}

// Idle function; just refreshes the display.
void idle() {
    glutPostRedisplay();
}

// Mainy McMainerson.
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // Game mode is pretty awesome, I recommend it.
    // glutGameModeString("1920x1200:16@60");
    // glutEnterGameMode();
    glutInitWindowSize(width, height); 
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    // Register functions!  Oh C++ and its pseudo first-order functions...
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutTimerFunc(20, timer, 0);
    glutIdleFunc(idle);
    glutPassiveMotionFunc( motion );
    glutMouseFunc( mouse );
    glutMainLoop();
    return 0;
}
