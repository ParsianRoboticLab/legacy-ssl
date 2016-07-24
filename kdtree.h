
#ifndef KDTREE_H
#define KDTREE_H

#include "geom.h"
#include "obstacle.h"
#include <string.h>

#define FSTALC_TEM template <class item_t>
#define FSTALC_FUN fast_allocator<item_t>

FSTALC_TEM
class fast_allocator{
  item_t *free_list;
  int num_alloc,num_free;
public:
  fast_allocator() {free_list=NULL; num_alloc=num_free=0;}
  ~fast_allocator();

  item_t *alloc();
  void free(item_t *item);
  void freelist(item_t *item);
  int count_allocated() {return(num_alloc);}
  int count_free() {return(num_free);}
};

class KDTree{
  struct node{
    Vector2D minv,maxv; // bounding box of subtree
    state *states;      // list of states stored at this node
    int num_states;     // number of states of this subtree
    union{
      node *child[2]; // children of this tree
      node *next;
    };
  };

  node *root;
  int leaf_size,max_depth;
  int tests;
  fast_allocator<node> anode;

protected:
  inline bool inside(Vector2D &minv,Vector2D &maxv,state &s);
  inline float box_distance(Vector2D &minv,Vector2D &maxv,Vector2D &p);
  void split(node *t,int split_dim);
  state *nearest(node *t,state *best,float &best_dist,Vector2D &x);
  void clear(node *t);
public:
  KDTree() {root=NULL; leaf_size=max_depth=0;}
  state getRoot();
  bool setdim(Vector2D &minv,Vector2D &maxv,int nleaf_size,int nmax_depth);
  bool add(state *s);
  void clear();
  state *nearest(float &dist,Vector2D &x);
};

FSTALC_TEM
FSTALC_FUN::~fast_allocator()
{
  item_t *p,*q;

  p = free_list;
  while( (q = p) ){
    p = p->next;
    q->next = NULL;
    delete(q);
  }

  free_list = NULL;
  num_alloc = num_free = 0;
}

FSTALC_TEM
item_t *FSTALC_FUN::alloc()
{
  item_t *p;

  if(free_list){
    p = free_list;
    free_list = p->next;
    p->next = NULL;
    num_free--;
  }else{
    p = new item_t;
  }
  num_alloc++;

  return(p);
}

FSTALC_TEM
void FSTALC_FUN::free(item_t *item)
{
  item->next = free_list;
  free_list = item;
  num_free++;
  num_alloc--;
}

FSTALC_TEM
void FSTALC_FUN::freelist(item_t *item)
{
  item_t *n;

  while(item){
    n = item->next;
    item->next = free_list;
    free_list = item;
    item = n;

    num_free++;
    num_alloc--;
  }
}



template <class data>
        inline void mzero(data &d)
{
    memset(&d,0,sizeof(d));
}

template <class data>
        inline void mzero(data *d,int n)
{
    memset(d,0,sizeof(data)*n);
}

#endif // KDTREE_H
