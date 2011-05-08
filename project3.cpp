#if defined(__APPLE__) && defined(__MACH__)  // for mac
#   include <GLUT/glut.h>
#else // for others
#   include <GL/glut.h>
#endif

#include <iostream>
#include <cmath>

using namespace std;

int width = 500;
int height = 500;

// Eye coords.
int ex = 0, ey = 0, ez = 0;
GLdouble dx = 1, dy = 0, dz = 0;

int rotOffset[3];

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
}

void makeSphereRing(float y, float r, int sphereR, int n) {
    glPushMatrix();
    float angle = 360.0 / n;
    // glTranslatef(r,0,0);
    glRotatef(rotOffset[0], 0.0, 1.0, 0.0);
    for (int i = 0; i < n; i++) {
        glRotatef(angle, 0.0, 1.0, 0.0);
        glPushMatrix();
        glTranslatef(r,0,0);
        glutSolidSphere(sphereR, 100, 100);
        glPopMatrix();
    }
    glPopMatrix();
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    gluLookAt(ex, ey, ez, ex + dx, ey - dy, ez + dz, 0.0, 1.0, 0.0);
    
    glColor3f(1.0, 0.0, 1.0);
    glutSolidSphere(50, 100, 1, 100);
    // glLoadIdentity();
    // gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // glScalef(1.0, 2.0, 1.0);      /* modeling transformation */ 
    // glRotatef(spin_y, 1.0, 0.0, 0.0);
    // glRotatef(spin_x, 0.0, 1.0, 0.0);
    glColor3f(1.0, 1.0, 0.0);
    // glTranslatef(5,0,0);
    // glutSolidSphere(1, 100, 100);
    // glTranslatef(5,0,0);
    // glutSolidSphere(1, 100, 100);
    makeSphereRing(0, 10, 8);
    glPopMatrix();
    
    glutSwapBuffers();
    glFlush();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, width / (GLdouble) height, 1, 100);
    // glFrustum(-100, 100, -100, 100, 1, 100);
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
    rotOffset[0] = (rotOffset[0] + 1) % 360;
    rotOffset[1] = (rotOffset[1] + 2) % 360;
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
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    // glutGameModeString("640x480:16@60");
    // glutEnterGameMode();
    glutInitWindowSize(500, 500); 
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutTimerFunc(10, timer, 0);
    glutIdleFunc(idle);
    glutPassiveMotionFunc( motion );
    glutMouseFunc( mouse );
    glutMainLoop();
    return 0;
}
