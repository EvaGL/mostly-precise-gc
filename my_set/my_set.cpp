#include <cstdio>
#include "gc2.h"
#include <algorithm>

extern bool new_active;
 
using namespace std;
 
typedef unsigned int uint;
 
template <class T>
class cartesian_tree
{
  public:
 
 
  T key;
  uint y;
  int count;
 
  gc_ptr <cartesian_tree<T> > l, r;
 
  cartesian_tree (T _key)
  {
    count = 1;
    key = _key;
    y = ((rand()<<15) + rand());
    l = 0;
    r = 0;
  }
 
  cartesian_tree (void)
  {
    count = 1;
    key = 0;
    y = 0;
    l = 0;
    r = 0;
  }
};
 
template <class T>
gc_ptr <cartesian_tree<T> > merge (gc_ptr <cartesian_tree<T> > u, gc_ptr <cartesian_tree<T> > v)
{
  if (u == 0)
    return v;
 
  if (v == 0)
    return u;
 
  if (u->y >= v->y)
  {
    u->r = merge (u->r, v);
    u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
    return u;
  }
  else
  {
    v->l = merge (u, v->l);
    v->count = (v->l != 0 ? v->l->count : 0) + (v->r != 0 ? v->r->count : 0) + 1;
    return v;
  }
}
 
template <class T>
cartesian_tree<T> split (gc_ptr <cartesian_tree<T> > u, T key)
{
  cartesian_tree<T> res, tmp;
 
  res.l = res.r = 0;
 
  if (u == 0)
    return res;
 
  if (u->key >= key)
  {
    tmp = split (u->l, key);
    u->l = tmp.r;
    u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
    res.l = tmp.l, res.r = u;
//    res.l->sum = tmp.l->sum, res.r->sum = u->sum;
  }
  else
  {
    tmp = split (u->r, key);
    u->r = tmp.l;
    u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
    res.r = tmp.r, res.l = u;
//    res.r->sum = tmp.r->sum, res.l->sum = u->sum;
  }
 
  return res;
}
 
template <class T>
gc_ptr <cartesian_tree<T> > del (gc_ptr <cartesian_tree<T> >u, gc_ptr <cartesian_tree<T> > p, gc_ptr <cartesian_tree<T> > node, T key)
{
  if (u == 0)
    return node;
 
  if (u->key == key)
  {
    gc_ptr <cartesian_tree<T> > d;
    if (p == 0)
      d = node, node = merge (node->l, node->r);
    else
    {
      if (p->r == u)
        d = p->r, p->r = merge(u->l, u->r);
      else
        d = p->l, p->l = merge(u->l, u->r);
    }
//      free(d);
//    printf ("%p\n", d.ptr);
    return node;
  }
 
  if (u->key > key)
    return del(u->l, u, node, key);
  else
    return del(u->r, u, node, key);
  u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
}
 
template <class T>
gc_ptr <cartesian_tree<T> > add (gc_ptr <cartesian_tree<T> > u, gc_ptr <cartesian_tree<T> > v)
{
  if (u == 0)
    return v;
 
  if (u->y > v->y)
  {
    if (u->key < v->key)
    {
      u->r = add(u->r, v);
      u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
    }
    else
    {
      u->l = add(u->l, v);
      u->count = (u->l != 0 ? u->l->count : 0) + (u->r != 0 ? u->r->count : 0) + 1;
    }
    return u;
  }
  else
  {
    cartesian_tree<T> tmp;
    tmp = split (u, v->key);
    v->l = tmp.l, v->r = tmp.r;
    v->count = (v->l != 0 ? v->l->count : 0) + (v->r != 0 ? v->r->count : 0) + v->key;
    return v;
  }
 
  return 0;
}
 
template <class T>
uint greater_count (gc_ptr <cartesian_tree<T> > u, T key)
{
  if (u == 0)
    return 0;
 
  uint res = 0;
 
  if (u->key >= key)
  {
    if (u->r != 0)
      res += u->r->count;
    return res + 1 + greater_count(u->l, key);
  }
  else
    return greater_count(u->r, key);
}
 
template <class T>
T get_min (gc_ptr <cartesian_tree<T> > v)
{
  if (v->l != 0)
    return get_min(v->l);
  return v->key;
}
 
template <class T>
void clear (gc_ptr <cartesian_tree<T> > u)
{
  if (u == 0)
    return;
  clear(u->l), clear(u->r);
//    free(u);
}
 
template <class T>
class myset
{
public:
  gc_ptr <cartesian_tree<T> > node;
 
  myset()
  {
    node = 0;
  }
 
  void insert (T k)
  {
//    new_active = true;
    gc_ptr <cartesian_tree<T> > v = gc_new <cartesian_tree<T>> (1);
    *v = cartesian_tree<T> (k);
//    new_active = false;
    node = add (node, v);
  }
 
  void erase (T k)
  {
    node = del (node, gc_ptr<cartesian_tree<T>> (0), node, k);
  }
 
  uint greater_count (T k)
  {
    return ::greater_count (node, k);
  }
 
  T get_min ()
  {
    return ::get_min(node);
  }
};
 
int main (void)
{
//  srand(time(0));
  int ELEM_COUNT = 100000;
  multiset <int> s;
  myset <int> ms;
  for (int i = 0; i < ELEM_COUNT; i++)
  {
    int k = ((rand()<<15) + rand());
    s.insert(k);
    ms.insert(k);
  }
  for (int j = 0;;j++)
  {
    for (int i = 0; i < 50000; i++)
    {

      if (ms.get_min() != *s.begin())
      {
        printf ("ERROR\n");
        return 0;
      }

      s.erase(s.begin());
      ms.erase(ms.get_min());
    }
    for (int i = 0; i < 50000; i++)
    {
      int k = ((rand()<<15) + rand());
      s.insert(k);
      ms.insert(k);
    }
/*
    int root_cnt = 0;
    for (ptr_list* root = collect(); root != 0; root = root->next)
      root_cnt++;
    printf ("root_cnt = %d\n", root_cnt);
    for (ptr_list* root = collect(); root != 0; root = root->next)
      printf ("%p ", root->ptr);//reinterpret_cast <void*> (*((long *)(root->ptr))));
    printf ("\n");
*/
//    printf ("true root %p\n", ms.node.ptr);	
//    mark_and_sweep();
  }
}
