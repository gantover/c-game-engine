#include <stdio.h>
#include <stdlib.h>

#define VECTOR_INIT_CAPACITY 6
#define UNDEFINE -1
#define SUCCESS 0

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

int vectorTotal(vector *v) {
  int result = UNDEFINE;
  if (v) {
    result = v->vectorlist.total;
  }
  return result;
}


int vectorResize(vector *v, int capacity) {
  int status = UNDEFINE;
  if(v) {
    void **items = realloc(v->vectorlist.items, sizeof(void *) * capacity);
    if (items) {
      v->vectorlist.items = items;
      v->vectorlist.capacity = capacity;
      status = SUCCESS;
    }
  }
  return status;
}

int vectorPush(vector *v, void *item) {
  int status = UNDEFINE;
  if(v) {
    if (v->vectorlist.capacity == v->vectorlist.total) {
      status = vectorResize(v, v->vectorlist.capacity * 2);
      if (status != UNDEFINE) {
        v->vectorlist.items[v->vectorlist.total++] = item;
        status = SUCCESS;
      }
    } else {
      v->vectorlist.items[v->vectorlist.total++] = item;
      // here we add the item at total then we increment total;
      status = SUCCESS;
    }
  }
  return status;
}

void *vectorGet(vector *v, int index) {
  void *readData = NULL;
  if (v) {
    if (index >= 0 && index < v->vectorlist.total ) {
      readData = v->vectorlist.items[index];
    }
  }
  return readData;
}

int vectorSet(vector *v, int index, void *item) {
  int status = UNDEFINE;
  if (v) {
    if (index >= 0 && index < v->vectorlist.total) {
      v->vectorlist.items[index] = item;
    }
    status = SUCCESS;
  }
  return status;
}

// int vectorRemove(vector *v, int index) {
//   int status = UNDEFINE;
//   if (v) {
//     if (index >= 0 && index < v->vectorlist.total) {
//       v->vectorlist.items[index] = NULL;
//       for (int i = index+1; i < v->vectorlist.total; i++) {
//         v->vectorlist.items[i-1] = v->vectorlist.items[i];
//         v->vectorlist.items[i] = NULL;
//       }
//       v->vectorlist.items[v->vectorlist.total--] = NULL;
//     }
//   }
//   return status;
// }

int vectorRemove(vector *v, int index)
{
    int  status = UNDEFINE;
    int i = 0;
    if(v)
    {
        if ((index < 0) || (index >= v->vectorlist.total))
            return status;
        free(v->vectorlist.items[index]);
        v->vectorlist.items[index] = NULL;
        for (i = index; (i < v->vectorlist.total - 1); ++i)
        {
            v->vectorlist.items[i] = v->vectorlist.items[i + 1];
            v->vectorlist.items[i + 1] = NULL;
        }
        v->vectorlist.total--;
        if ((v->vectorlist.total > 0) && ((v->vectorlist.total) == (v->vectorlist.capacity / 4)))
        {
            vectorResize(v, v->vectorlist.capacity / 2);
        }
        status = SUCCESS;
    }
    return status;
}

int vectorRemoveFree(vector *v, int index) {
  int status = UNDEFINE;
  if (v) {
    if (index >= 0 && index < v->vectorlist.total) {
      free(v->vectorlist.items[index]);
      v->vectorlist.items[index] = NULL;
      for (int i = index; i < v->vectorlist.total-1; i++) {
        v->vectorlist.items[i] = v->vectorlist.items[i+1];
        // v->vectorlist.items[i+1] = NULL; 
        // seems like there is no need to do that since the value is overwritten anyway
      }
      v->vectorlist.items[--v->vectorlist.total] = NULL;
      // v->vectorlist.total--;
    }
  }
  return status;
}

int vectorFreeWithElem(vector *v) {
  int status = UNDEFINE;
  if (v) {
    for (int i = 0; i < v->vectorlist.total; i++) {
      free(v->vectorlist.items[i]);
    }
    free(v->vectorlist.items);
    status = SUCCESS;
  }
  return status;
}

int vectorFree(vector *v) {
  int status = UNDEFINE;
  if (v) {
    for (int i = 0; i < v->vectorlist.total; i++) {
      free(v->vectorlist.items[i]);
    }
    free(v->vectorlist.items);
    status = SUCCESS;
  }
  return status;
}

void vector_init(vector *v) {
  v->total = vectorTotal;
  v->resize = vectorResize;
  v->push = vectorPush;
  v->get = vectorGet;
  v->rmv = vectorRemove;
  v->rmv_free = vectorRemoveFree;
  v->free = vectorFree;
  v->free_w_elem = vectorFreeWithElem;
  v->vectorlist.total = 0;
  v->vectorlist.capacity = VECTOR_INIT_CAPACITY;
  v->vectorlist.items = malloc(sizeof(void *) * v->vectorlist.capacity);
}
