#include <GLUT/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define res 1
#define SW 160*res // Screen width
#define SH 120*res
#define SW2 (SW/2)
#define SH2 (SH/2)
#define pixelScale 4/res
#define GLSW (SW*pixelScale) // OpenGL window width
#define GLSH (SH*pixelScale)
#define square(x) (x)*(x)

typedef struct {
  int fr1, fr2;
} time; time T;

typedef struct {
  int w,s,a,d;
} keys; keys K;

typedef struct {
  int x,y,z;
  int a;
  int l;
} player; player P;

typedef struct {
  float cos[360];
  float sin[360];
} math; math M;

typedef struct {
  float f; // from fov
  float a; // h/w
  int zn;
  int zf;
  int q;
} projection; projection PJ;

typedef int vec3d[3];

void pixel(int x, int y, int r, int g, int b) {
  glColor3ub(r,g,b);
  glBegin(GL_POINTS);
  glVertex2i(x*pixelScale+2, y*pixelScale+2);
  glEnd();
}

void movePlayer() {
  if (K.w==1) {P.z+=1; printf("moving up\n");}
  if (K.s==1) {P.z-=1;}
  if (K.d==1) {P.x+=1;}
  if (K.a==1) {P.x-=1;}
}

void moveVec(vec3d vec) { // from player position
  vec[0]-=P.x;
  vec[1]-=P.y;
  vec[2]-=P.z;
}

void rotateVec(vec3d vec) {
}

void projectVec(vec3d vec) {
  vec[0]=vec[0]*200/vec[2] + SW2;
  vec[1]=vec[1]*200/vec[2] + SH2;
}

void clearBackground() {
  int x,y;
  for (x=0; x<SW; x++) {
    for (y=0; y<SH; y++) {
      pixel(x,y,0,60,130);
    }
  }
}

void draw3D() {
  vec3d vec0 = {40,0,10};
  vec3d vec1 = {40,0,290};
  moveVec(vec0);
  moveVec(vec1);
  projectVec(vec0);
  projectVec(vec1);
  pixel(vec0[0], vec0[1], 255,0,0);
  pixel(vec1[0], vec1[1], 0,255,0);
}

void display() {
  int x,y;
  if(T.fr1-T.fr2>=42){ // getting ±24fps
    clearBackground();
    movePlayer();
    draw3D();
    T.fr2=T.fr1;
    glutSwapBuffers();
    glutReshapeWindow(GLSW,GLSH); // prevents resizing
  }
  T.fr1=glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay(); // marks window as needing to be redisplayed
}

void init() {
  int dg; float u;
  for (dg=0; dg<360; dg++) {
    u = dg*M_PI/180.0;
    M.cos[dg] = cos(u);
    M.sin[dg] = sin(u);
  }
  PJ.a = (float)SH/(float)SW;
  PJ.f = 1.0/tan(90.0*M_PI/360.0);
  PJ.q = PJ.zf/(float)(PJ.zf-PJ.zn);
  P.x=70; P.z=-110; P.y=20;
}

void keysDown(unsigned char key, int x, int y) {
  if (key=='w') {K.w=1;}
  if (key=='s') {K.s=1;}
  if (key=='a') {K.a=1;}
  if (key=='d') {K.d=1;}
}

void keysUp(unsigned char key, int x, int y) {
  if (key=='w') {K.w=0;}
  if (key=='s') {K.s=0;}
  if (key=='a') {K.a=0;}
  if (key=='d') {K.d=0;}
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(GLSW/2, GLSH/2);
  glutInitWindowSize(GLSW, GLSH);
  glutCreateWindow("Potato <3");
  glPointSize(pixelScale);
  gluOrtho2D(0,GLSW,0,GLSH); // left, right, bottom, top
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keysDown);
  glutKeyboardUpFunc(keysUp);
  glutMainLoop();
  return EXIT_SUCCESS;
}
