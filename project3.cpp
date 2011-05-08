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

bool solid;

int width = 500;
int height = 500;

// Eye coords.
int ex, ey, ez;
// Delta from the eye to the reference.
GLdouble dx, dy, dz;

double rotOffset[3];

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
    rotOffset[2] = 0;
}

void init(void) {
    reset();
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
}

void makeSphereRing(float y, float r, int sphereR) {
    glPushMatrix();
    int n = (int) (PI * r / sphereR / 2);
    float angle = 360.0 / n;
    glTranslatef(0,y,0);
    for (int i = 0; i < n; i++) {
        glRotatef(angle, 0.0, 1.0, 0.0);
        glPushMatrix();
        glTranslatef(r,0,0);
        if (solid) {
            glutSolidSphere(sphereR, 15, 15);
        } else {
            glutWireSphere(sphereR, 15, 15);
        }
        glPopMatrix();
    }
    glPopMatrix();
}

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

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    gluLookAt(ex, ey, ez, ex + dx, ey - dy, ez + dz, 0.0, 1.0, 0.0);
    
    // Make the enclosing sphere.
    glColor3f(1.0, 0.0, 1.0);
    if (solid) {
        glutSolidSphere(50, 100, 100);
    } else {
        glutWireSphere(50, 100, 100);
    }
    // Make the sphere of spheres touching the outer sphere.
    glColor3f(1.0, 1.0, 0.0);
    makeSphereSphere(47, 5);
    // glLoadIdentity();
    // gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // glScalef(1.0, 2.0, 1.0);      /* modeling transformation */ 
    // glRotatef(spin_y, 1.0, 0.0, 0.0);
    // glRotatef(spin_x, 0.0, 1.0, 0.0);
    glColor3f(0.0, 1.0, 0.0);
    // glTranslatef(5,0,0);
    // glutSolidSphere(1, 100, 100);
    // glTranslatef(5,0,0);
    // glutSolidSphere(1, 100, 100);
    makeSphereSphere(10, 1);
    glPopMatrix();
    
    glutSwapBuffers();
    glFlush();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, w / (GLdouble) h, 1, 120);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void mouse( int button, int state, int x, int y ) {
    glutPostRedisplay();
}

void motion( int x, int y ) {
    // Cap x and y at the width and height of the screen.
    // Don't judge me because of my nested ternary operators.
    x = x < 0 ? 0 : (x > width ? width : x);
    y = y < 0 ? 0 : (y > height ? height : y);
    dy = y - height / 2.0;
    dz = x - width / 2.0;
    dx = ((height / 2.0 - abs(dy)) + (width / 2.0 - abs(dz))) / 2;
    glutPostRedisplay( );
}

void timer(int v) {
    rotOffset[0] = (rotOffset[0] + 0.25);
    rotOffset[1] = (rotOffset[1] + 359.5);// % 360;
    glutTimerFunc(15, timer, 0);
    glutPostRedisplay();
}

void idle() {
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': ex++; break;
        case 's': ex--; break;
        case 'd': ez++; break;
        case 'a': ez--; break;
        case ' ': ey++; break;
        case 'z': ey--; break;
        case '1': solid = false; break;
        case '2': solid = true; break;
        case 'r': reset(); break;
        case 'q': exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // glutGameModeString("640x480:16@60");
    // glutEnterGameMode();
    glutInitWindowSize(width, height); 
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
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
