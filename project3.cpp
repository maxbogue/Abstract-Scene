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

#define PI 3.14159265

using namespace std;

// Not actually sure what these should default to..
int width = 1024;
int height = 768;

bool solid;             // Whether shapes are solid or wireframe.

double ex, ey, ez;         // Eye coords.
double dx, dy, dz;      // Delta from the eye to the reference.

double rotOffset[2];    // The rotations of all spheres.
double cubeEdgeScale;   // How scaled the cube edges are.
bool cubeEdgeGrow;      // Whether the cube edges are grown (growing) or not.

// Sets all global values to their defaults.
void reset() {
    solid = false;
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

// Initialize globals and GL.
void init(void) {
    reset();
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
}

/**
 * Constructs a series of spheres positioned in a ring.
 * @param y     The height of the ring.
 * @param r     The radius of the ring.
 * @param sr    The radius of each sphere.
 */
void makeSphereRing(float y, float r, int sr) {
    glPushMatrix();
    int n = (int) (PI * r / sr / 2);
    float angle = 360.0 / n;
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
void makeSphereSphere(float r, int sr) {
    int n = (int) (r / sr / 2);
    float yStep = 2 * r / n;
    float y = yStep / 2 - r;
    for (int i = 0; i < n; i++) {
        glPushMatrix();
        glRotatef(rotOffset[i % 2], 0.0, 1.0, 0.0);
        float x = sqrt(r*r - y*y);
        makeSphereRing(y, x, sr);
        y += yStep;
        glPopMatrix();
    }
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
        float s = cubeEdgeScale * 2.0 * dist / size + 1;
        glScalef(x ? 1 : s, y ? 1 : s, z ? 1 : s);
        if (solid) {
            glutSolidCube(size);
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

// General construction of the world; called for each viewport.
void makeWorld() {
    glPushMatrix();
    
    // Make the enclosing sphere.
    glColor3f(10/255.0, 85/255.0, 175/255.0);
    if (solid) {
        glutSolidSphere(50, 100, 100);
    } else {
        glutWireSphere(50, 100, 100);
    }
    
    // Make the sphere of spheres touching the outer sphere.
    glColor3f(0/255.0, 204/255.0, 255/255.0);
    makeSphereSphere(47, 5);
    
    // Make the inner sphere of spheres.
    glColor3f(204/255.0, 255/255.0, 0/255.0);
    makeSphereSphere(10, 1);
    
    // Make the cubes and torus.
    glColor3f(255/255.0, 0/255.0, 204/255.0);
    makeCubes(15, 1);
    makeTorus();
    
    glPopMatrix();
}

// The main display function.
void display(void) {
    
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
    double norm = sqrt(dx*dx + dy*dy + dz*dz);
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
