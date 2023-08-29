#include <GLUT/glut.h>

#include <OpenGL/OpenGL.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define res 1
#define SW 160 * res // Screen width
#define SH 120 * res
#define SW2 (SW / 2)
#define SH2 (SH / 2)
#define pixelScale 4 / res
#define GLSW (SW * pixelScale) // OpenGL window width
#define GLSH (SH * pixelScale)
#define square(x) (x) * (x)

typedef struct {
  int fr1, fr2;
} time;
time T;

typedef struct {
  int w, s, a, d;
} keys;
keys K;

typedef struct {
  float x, y, z;
  int a;
  int l;
} player;
player P;

typedef struct {
  float cos[360];
  float sin[360];
} math;
math M;

typedef struct {
  float x, y, z;
} fvec3d;

typedef struct {
  fvec3d *v[3];
} ftriangle; // triangle

typedef struct {
  int x, y;
} pos;

typedef struct {
  ftriangle *faces;
  int numFaces;
} mesh;

mesh Cube;

typedef float mat4[4][4];

mat4 projMat;

void movePlayer() {
  if (K.w == 1) {
    P.z += 0.1;
  }
  if (K.s == 1) {
    P.z -= 0.1;
  }
  if (K.d == 1) {
    P.x += 0.1;
  }
  if (K.a == 1) {
    P.x -= 0.1;
  }
}

void moveVec(fvec3d *vec) { // from player position
  (*vec).x -= P.x;
  (*vec).y -= P.y;
  (*vec).z -= P.z;
}

void multMatVec(fvec3d *i, fvec3d *o, mat4 mat) {
  o->x = i->x * mat[0][0] + i->y * mat[1][0] + i->z * mat[2][0] + mat[3][0];
  o->y = i->x * mat[0][1] + i->y * mat[1][1] + i->z * mat[2][1] + mat[3][1];
  o->z = i->x * mat[0][2] + i->y * mat[1][2] + i->z * mat[2][2] + mat[3][2];
  float w =
      i->x * mat[0][3] + i->y * mat[1][3] + i->z * mat[2][3] + mat[3][3];
  if (w != 0.0) {
    *o = (fvec3d){o->x / w, o->y / w, o->z};
  }
}

void scaleVec(fvec3d *v) { // we dont take in and out since we can modify this vector directly
  v->x = ((v->x + 1.0) * SW2);
  v->y = ((v->y + 1.0) * SH2);
}

void readObj() {
  FILE *fp = fopen("cube.obj", "r");
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  if (fp == NULL) {
    printf("Error opening cube.obj");
    return;
  }
  int numVertex, numFaces;

  fscanf(fp, "%i", &numVertex);
  fvec3d *vertexes = malloc(sizeof(fvec3d) * numVertex);

  for (int i = 0; i < numVertex; i++) {
    fscanf(fp, "%f %f %f", &vertexes[i].x, &vertexes[i].y, &vertexes[i].z);
  }

  fscanf(fp, "%i", &numFaces);
  ftriangle *faces = malloc(sizeof(ftriangle) * numFaces);

  int ind[3];
  for (int j = 0; j < numFaces; j++) {
    fscanf(fp, "%i %i %i", &ind[0], &ind[1], &ind[2]);
    faces[j].v[0] = &vertexes[ind[0] - 1];
    faces[j].v[1] = &vertexes[ind[1] - 1];
    faces[j].v[2] = &vertexes[ind[2] - 1];
  }
  Cube.faces = faces;
  Cube.numFaces=numFaces;
  printf("------ end reading file ------\n");
  fclose(fp);
}

void pixel(int x, int y, int r, int g, int b) {
  glColor3ub(r, g, b);
  glBegin(GL_POINTS);
  glVertex2i(x * pixelScale + 2, y * pixelScale + 2);
  glEnd();
}

void clearBackground() {
  int x, y;
  for (x = 0; x < SW; x++) {
    for (y = 0; y < SH; y++) {
      pixel(x, y, 0, 60, 130);
    }
  }
}

void drawLines(fvec3d v1, fvec3d v2, int r, int g, int b) {
  int x1=v1.x, x2=v2.x, y1=v1.y, y2=v2.y;
  glColor3ub(r, g, b);
  glBegin(GL_LINES);
  glVertex2i(x1 * pixelScale + 2, y1 * pixelScale + 2);
  glVertex2i(x2 * pixelScale + 2, y2 * pixelScale + 2);
  glEnd();
}

void drawTriangle(fvec3d *v0, fvec3d *v1, fvec3d *v2) {
  drawLines(*v0, *v1, 255, 0, 0);
  drawLines(*v1, *v2, 255, 0, 0);
  drawLines(*v2, *v0, 255, 0, 0);
}

void draw3D() {
  fvec3d o0, o1, o2;
  for (int i = 0; i < Cube.numFaces; i++) {
    ftriangle face = Cube.faces[i];
    fvec3d v0=*face.v[0], v1=*face.v[1], v2=*face.v[2];
    moveVec(&v0); moveVec(&v1); moveVec(&v2);
    multMatVec(&v0, &o0, projMat);
    multMatVec(&v1, &o1, projMat);
    multMatVec(&v2, &o2, projMat);
    scaleVec(&o0);
    scaleVec(&o1);
    scaleVec(&o2);
    drawTriangle(&o0, &o1, &o2);
  }
}

void display() {
  int x, y;
  if (T.fr1 - T.fr2 >= 42) { // getting ±24fps
    clearBackground();
    movePlayer();
    draw3D();
    T.fr2 = T.fr1;
    glutSwapBuffers();
    glutReshapeWindow(GLSW, GLSH); // prevents resizing
  }
  T.fr1 = glutGet(GLUT_ELAPSED_TIME);
  glutPostRedisplay(); // marks window as needing to be redisplayed
}

void init() {
  readObj();
  int dg;
  float u;
  for (dg = 0; dg < 360; dg++) {
    u = dg * M_PI / 180.0;
    M.cos[dg] = cos(u);
    M.sin[dg] = sin(u);
  }
  float a = (float)SH / (float)SW;
  float f = 1.0 / tan(90.0 * M_PI / 360.0);
  float zf = 100.0;
  float zn = 0.01;
  float q = zf / (float)(zf - zn);
  // P.x=70; P.z=-110; P.y=20;
  P.x = 0.3;
  P.z = -5.0;
  P.y = 0.2;
  projMat[0][0] = a * f;
  projMat[1][1] = f;
  projMat[2][2] = q;
  projMat[3][2] = -zn * q;
  projMat[2][3] = 1;
}

void keysDown(unsigned char key, int x, int y) {
  if (key == 'w') {
    K.w = 1;
  }
  if (key == 's') {
    K.s = 1;
  }
  if (key == 'a') {
    K.a = 1;
  }
  if (key == 'd') {
    K.d = 1;
  }
}

void keysUp(unsigned char key, int x, int y) {
  if (key == 'w') {
    K.w = 0;
  }
  if (key == 's') {
    K.s = 0;
  }
  if (key == 'a') {
    K.a = 0;
  }
  if (key == 'd') {
    K.d = 0;
  }
}

int main(int argc, char *argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(GLSW / 2, GLSH / 2);
  glutInitWindowSize(GLSW, GLSH);
  glutCreateWindow("Potato <3");
  glPointSize(pixelScale);
  gluOrtho2D(0, GLSW, GLSH, 0); // left, right, bottom, top
  init();
  glutDisplayFunc(display);
  glutKeyboardFunc(keysDown);
  glutKeyboardUpFunc(keysUp);
  glutMainLoop();
  return EXIT_SUCCESS;
}
