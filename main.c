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
  float delta;
} time;
time T;

typedef struct {
  int w, s, a, d;
  int m;
  int q, e;
} keys;
keys K;

typedef struct {
  float x, y, z;
  float a;
  float l;
} camera;
camera C;

typedef struct {
  int r, g, b;
} col;
col colors[3];

typedef struct {
  float x, y, z;
} fvec3d;

typedef struct {
  int ind[3];
} ftriangle;

typedef struct {
  fvec3d *vecArr[3];
  float dist;
  float prod;
} ptriangle; // pending triangle
typedef ptriangle *pptriangle;

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
mat4 rotXMat;
mat4 rotCamYMat;
mat4 rotCamXMat;

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

void updateRotYMat(float a, mat4 c) {
  c[0][0]=cos(a);
  c[2][2]=cos(a);
  c[0][2]=sin(a);
  c[1][1]=1.0;
  c[2][0]=-sin(a);
}
void updateRotXMat(float a, mat4 c) {
  c[0][0]=1.0;
  c[1][1]=cos(a);
  c[2][2]=cos(a);
  c[1][2]=-sin(a);
  c[2][1]=sin(a);
}

void updateRotCamYMat(float a, mat4 c) {
  c[0][0] = cos(a);
  c[2][2] = cos(a);
  c[1][1] = 1.0;
  c[2][0] = sin(a);
  c[0][2] = -sin(a);
}

void updateRotCamXMat(float a, mat4 c) {
  c[0][0] = 1.0;
  c[1][1] = cos(a);
  c[2][2] = cos(a);
  c[1][2] = -sin(a);
  c[2][1] = sin(a);
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

void moveVec(fvec3d *vec) { // from player position
  (*vec).x -= C.x;
  (*vec).y -= C.y;
  (*vec).z -= C.z;
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

float norm(fvec3d *v) {
  float norm = sqrt(square(v->x) + square(v->y) + square(v->z));
  v->x = v->x/norm; v->y = v->y/norm; v->z = v->z/norm;
  return norm;
}

void computeNormal(fvec3d *a, fvec3d *b, fvec3d *n) {
  n->x = a->y*b->z - a->z*b->y;
  n->y = a->z*b->x - a->x*b->z;
  n->z = a->x*b->y - a->y*b->x;
  norm(n);
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

void filledTriangle(fvec3d *v0, fvec3d *v1, fvec3d *v2, float u) {
    u = -u;
    glBegin(GL_TRIANGLES);
    glColor3f( u, u, u ); 
    glVertex2i(v0->x * pixelScale + 2, v0->y * pixelScale + 2);
    glVertex2i(v1->x * pixelScale + 2, v1->y * pixelScale + 2);
    glVertex2i(v2->x * pixelScale + 2, v2->y * pixelScale + 2);
    glEnd();
}

void movePlayer() {
  fvec3d mv = {0.0,0.0,0.0};

  // angles
  if (K.d == 1 && K.m == 1) {
    C.a += 0.02;
  }
  if (K.a == 1 && K.m == 1) {
    C.a -= 0.02;
  }
  if (K.w == 1 && K.m == 1) {
    C.l += 0.02;
  }
  if (K.s == 1 && K.m == 1) {
    C.l -= 0.02;
  }

  if (K.w == 1 && K.m == 0) {
    mv.z += 0.1;
  }
  if (K.s == 1 && K.m == 0) {
    mv.z -= 0.1;
  }
  if (K.d == 1 && K.m == 0) {
    mv.x += 0.1;
  }
  if (K.a == 1 && K.m == 0) {
    mv.x -= 0.1;
  }
  if (K.q == 1) {
    mv.y -= 0.1;
  }
  if (K.e == 1) {
    mv.y += 0.1;
  }

  updateRotXMat(C.l, rotXMat);
  updateRotYMat(C.a, rotYMat);
  updateRotCamYMat(C.a, rotCamYMat);
  updateRotCamXMat(-C.l, rotCamXMat);

  // apply movement
  fvec3d o;
  multMatVec(&mv, &o, &rotCamXMat);
  mv=o;
  multMatVec(&mv, &o, &rotCamYMat);
  mv=o;
  C.x += mv.x;
  C.y += mv.y;
  C.z += mv.z;
}

float dotProduct(fvec3d *v0, fvec3d *v1) {
  return v0->x * v1->x + v0->y * v1->y + v0->z * v1->z;
}

void drawCartesian() {
  fvec3d orig = {0.0,0.0,0.0};
  fvec3d x = {0.3,0.0,0.0};
  fvec3d y = {0.0,0.3,0.0};
  fvec3d z = {0.0,0.0,0.3};
  fvec3d *arr[3] = {&x,&y,&z};
  colors[0] = (col){255,0,0};
  colors[1] = (col){0,255,0};
  colors[2] = (col){0,100,255};

  moveVec(&orig);
  fvec3d o;
  multMatVec(&orig, &o, &rotYMat);
  orig=o;
  multMatVec(&orig, &o, &rotXMat);
  orig=o;
  multMatVec(&orig, &o, &projMat);
  scaleVec(&o);
  orig=o;

  for (int i=0; i<3; i++) {
    moveVec(arr[i]);
    fvec3d o;
    multMatVec(arr[i], &o, &rotYMat);
    *arr[i]=o;
    multMatVec(arr[i], &o, &rotXMat);
    *arr[i]=o;
    multMatVec(arr[i], &o, &projMat);
    scaleVec(&o);
    *arr[i]=o;
    drawLines(orig, *arr[i], colors[i].r, colors[i].g, colors[i].b);
  }
}

void drawNormal(fvec3d v0, fvec3d v0o, fvec3d n) {
  n.x = n.x / 4 + v0o.x;
  n.y = n.y / 4 + v0o.y;
  n.z = n.z / 4 + v0o.z;
  moveVec(&n);
  fvec3d o;
  multMatVec(&n, &o, &rotYMat);
  n=o;
  multMatVec(&n, &o, &rotXMat);
  n=o;
  multMatVec(&n, &o, &projMat);
  scaleVec(&o);
  n=o;
  drawLines(v0, n, 0, 255, 0);
}

fvec3d getCenterTri(fvec3d *v0, fvec3d *v1, fvec3d *v2) {
  fvec3d c;
  c.x = (v0->x + v1->x + v2->x) / 3;
  c.y = (v0->y + v1->y + v2->y) / 3;
  c.z = (v0->z + v1->z + v2->z) / 3;
  return c;
}

float getDist(fvec3d *v0, fvec3d *v1) {
  return sqrt(square(v0->x - v1->x) + square(v0->y - v1->y) + square(v0->z - v1->z));
}

// for debugging
void printDists(ptriangle **a, unsigned int lenght) {
  for (int i = 0; i < lenght; i++) {
    printf("dist at elem %i : %f\n", i, a[i]->dist);
  }
}

void basicSortTriangles(ptriangle **a, unsigned int lenght) { // takes a pointer to first elem of arry
  for (int i = 0; i < lenght-1; i++) {
    int swap_j = i;
    for (int j = i+1; j < lenght; j++) {
      if (a[j]->dist < a[swap_j]->dist) {
        swap_j = j;
      }
    }
    pptriangle swap = a[i];
    a[i] = a[swap_j];
    a[swap_j] = swap;
  }
}

void swapPTri(ptriangle **a, int i, int j) {
  pptriangle tmp = a[i];
  a[i] = a[j];
  a[j] = tmp;
}

int partition(ptriangle **a, int start, int end) {
  float pivot = a[end]->dist;
  int i = start-1;
  for (int j=start; j<end; j++) {
    if (a[j]->dist < pivot) {
      i++;
      swapPTri(a, i, j);
    }
  }
  swapPTri(a, i+1, end);
  return i+1;
}

void quicksortPTri(ptriangle **a, int start, int end) {
  if (start < end) {
    int ind_pivot = partition(a, start, end);
    quicksortPTri(a, start, ind_pivot-1);
    quicksortPTri(a, ind_pivot+1, end);
  }
}


void draw3D() {
  int counter = 0;
  ptriangle *arrTri[Cube.numFaces]; // array of pointer to triangle
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
    multMatVec(v, &o, &rotXMat);
    *v=o;
    multMatVec(v, &o, &projMat);
    scaleVec(&o);
    *v=o;
  }
  for (int i = 0; i<Cube.numFaces; i++) {
    int ind[3];
    fvec3d *v0 = &vertexes[Cube.faces[i].ind[0]];
    fvec3d *v1 = &vertexes[Cube.faces[i].ind[1]];
    fvec3d *v2 = &vertexes[Cube.faces[i].ind[2]];
    fvec3d *v0o = &Cube.vertexes[Cube.faces[i].ind[0]];
    fvec3d *v1o = &Cube.vertexes[Cube.faces[i].ind[1]];
    fvec3d *v2o = &Cube.vertexes[Cube.faces[i].ind[2]];
    fvec3d a, b; 
    a.x = v1o->x - v0o->x;
    a.y = v1o->y - v0o->y;
    a.z = v1o->z - v0o->z;
    b.x = v2o->x - v0o->x;
    b.y = v2o->y - v0o->y;
    b.z = v2o->z - v0o->z;
    fvec3d center = getCenterTri(v0o, v1o, v2o);
    fvec3d c;
    // c.x = v0o->x - C.x;
    // c.y = v0o->y - C.y;
    // c.z = v0o->z - C.z;
    c.x = center.x - C.x;
    c.y = center.y - C.y;
    c.z = center.z - C.z;
    norm(&c);
    fvec3d n; computeNormal(&a, &b, &n);
    // drawNormal(*v0, *v0o, n);
    float prod = dotProduct(&c, &n);
    if (prod >= -1 && prod <= 0) {
      ptriangle *ptri = malloc(sizeof(ptriangle));
      ptri->vecArr[0]=v0;
      ptri->vecArr[1]=v1;
      ptri->vecArr[2]=v2;
      ptri->dist = getDist(&c, &center);
      ptri->prod = prod;
      arrTri[counter] = ptri;
      // drawTriangle(v0, v1, v2);
      counter += 1;
    }
  }
  // baiscSortTriangles(arrTri, counter);
  quicksortPTri(arrTri, 0, counter-1);
  for (int i = 0; i < counter; i++) {
    ptriangle *face = arrTri[i];
    fvec3d *v0 = face->vecArr[0];
    fvec3d *v1 = face->vecArr[1];
    fvec3d *v2 = face->vecArr[2];
    filledTriangle(v0, v1, v2, face->prod);
    free((pptriangle)arrTri[i]);
  }
  free(vertexes);
}

void display() {
  int x, y;
  if (T.fr1 - T.fr2 >= 20) { // getting ±24fps
    clearBackground();
    movePlayer();
    drawCartesian();
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
  C.x = 0.3;
  C.z = -5.0;
  C.y = 0.2;
  C.a=0.0;
  C.l=0.0;
  createProjMat(90.0, 0.01, 100.0, projMat);
  fillMatZeros(rotXMat);
  fillMatZeros(rotYMat);
  fillMatZeros(rotCamYMat);
  fillMatZeros(rotCamXMat);
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
  if (key =='q') {
    K.q = 1;
  }
  if (key =='e') {
    K.e = 1;
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
  if (key =='q') {
    K.q = 0;
  }
  if (key =='e') {
    K.e = 0;
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
