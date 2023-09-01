#include <GLUT/glut.h>

#include <OpenGL/OpenGL.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int m;
} keys;
keys K;

typedef struct {
  float x, y, z;
  float a;
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
  int ind[3];
} ftriangle;

typedef struct {
  int x, y;
} pos;

typedef struct {
  ftriangle *faces;
  int numFaces;
  fvec3d *vertexes;
  int numVertex;
} mesh;

mesh Cube;

typedef float mat4[4][4];

mat4 projMat;
mat4 rotYMat;

typedef struct {
  float coefs[4][4];
} mat4x4;

void fillMatZeros(mat4 coefs) {
  for (int i = 0; i<4; i++) {
    for (int j = 0; j<4; j++) {
      coefs[i][j]=0.0;
    }
  }
}

void createProjMat(float fov, float zn, float zf, mat4 coefs) {
  fillMatZeros(coefs);
  // fov in degrees
  float a = (float)SH / (float)SW;
  float f = 1.0 / tan(fov * M_PI / 360.0);
  float q = zf / (float)(zf - zn);
  coefs[0][0] = a * f;
  coefs[1][1] = f;
  coefs[2][2] = q;
  coefs[3][2] = -zn * q;
  coefs[2][3] = 1;
}

void updateRotYMat(float a, mat4 coefs) {
  coefs[0][0]=cos(a);
  coefs[2][2]=cos(a);
  coefs[0][2]=sin(a);
  coefs[1][1]=1.0;
  coefs[2][0]=-sin(a);
}

void readObj() {
  FILE *fp = fopen("twocubes.obj", "r");
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
    faces[j].ind[0]=ind[0]-1;
    faces[j].ind[1]=ind[1]-1;
    faces[j].ind[2]=ind[2]-1;
  }
  Cube.faces = faces;
  Cube.numFaces=numFaces;
  Cube.vertexes=vertexes;
  Cube.numVertex=numVertex;
  printf("------ end reading file ------\n");
  fclose(fp);
}

void movePlayer() {
  if (K.d == 1 && K.m == 1) {
    P.a += 0.02;
  }
  if (K.a == 1 && K.m == 1) {
    P.a -= 0.02;
  }
  float dx=sin(P.a)*0.1, dz=cos(P.a)*0.1;
  if (K.w == 1 && K.m == 0) {
    P.x += dx;
    P.z += dz;
  }
  if (K.s == 1 && K.m == 0) {
    P.x -= dx; 
    P.z -= dz;
  }
  if (K.d == 1 && K.m == 0) {
    P.x += dz; P.z -= dx;
  }
  if (K.a == 1 && K.m == 0) {
    P.x -= dz; P.z += dx;
  }
}

void moveVec(fvec3d *vec) { // from player position
  (*vec).x -= P.x;
  (*vec).y -= P.y;
  (*vec).z -= P.z;
}

void multMatVec(fvec3d *i, fvec3d *o, mat4 *mat) {
  o->x = i->x * (*mat)[0][0] + i->y * (*mat)[1][0] + i->z * (*mat)[2][0] + (*mat)[3][0];
  o->y = i->x * (*mat)[0][1] + i->y * (*mat)[1][1] + i->z * (*mat)[2][1] + (*mat)[3][1];
  o->z = i->x * (*mat)[0][2] + i->y * (*mat)[1][2] + i->z * (*mat)[2][2] + (*mat)[3][2];
  float w =
      i->x * (*mat)[0][3] + i->y * (*mat)[1][3] + i->z * (*mat)[2][3] + (*mat)[3][3];
  if (w != 0.0) {
    *o = (fvec3d){o->x / w, o->y / w, o->z};
  }
}

void scaleVec(fvec3d *v) { // we dont take in and out since we can modify this vector directly
  v->x = ((v->x + 1.0) * SW2);
  v->y = ((v->y + 1.0) * SH2);
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
  updateRotYMat(P.a, rotYMat);
  fvec3d *vertexes = malloc(sizeof(fvec3d)*Cube.numVertex);// cube copy (to not alter original cooridinates)
  for (int i = 0; i<Cube.numVertex; i++) {
    vertexes[i] = Cube.vertexes[i]; // have to assign each array element, the pointer to an array only points to its first element
  }
  for (int i = 0; i<Cube.numVertex; i++) {
    fvec3d *v = &(vertexes[i]);
    moveVec(v);
    fvec3d o;
    multMatVec(v, &o, &rotYMat);
    *v=o;
    multMatVec(v, &o, &projMat);
    scaleVec(&o);
    *v=o;
  }
  for (int i = 0; i<Cube.numFaces; i++) {
    int ind[3];
    ind[0] = Cube.faces[i].ind[0];
    ind[1] = Cube.faces[i].ind[1];
    ind[2] = Cube.faces[i].ind[2];
    drawTriangle(&vertexes[ind[0]], &vertexes[ind[1]], &vertexes[ind[2]]);
  }
  free(vertexes);
}

void display() {
  int x, y;
  if (T.fr1 - T.fr2 >= 20) { // getting ±24fps
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
  P.x = 0.3;
  P.z = -5.0;
  P.y = 0.2;
  P.a=0.0;
  createProjMat(90.0, 0.01, 100.0, projMat);
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
  if (key == 'm') {
    K.m = 1;
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
  if (key == 'm') {
    K.m = 0;
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
