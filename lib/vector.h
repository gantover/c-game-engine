#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
  void **items;
  int capacity;
  int total;
} sVectorList;

typedef struct sVector vector;
struct sVector {
  sVectorList vectorlist;
  int (*total) (vector *);
  int (*resize) (vector *, int);
  int (*push) (vector *, void *);
  void *(*get) (vector *, int);
  int (*rmv) (vector *, int);
  int (*rmv_free) (vector *, int);
  int (*free) (vector *);
  int (*free_w_elem) (vector *);
};

void vector_init(vector *v);

#endif // !VECTOR_H
