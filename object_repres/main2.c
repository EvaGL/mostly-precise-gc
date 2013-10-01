#include</home/mariia/Desktop/object_repres/prototype.h>
#include</home/mariia/Desktop/object_repres/boxing.h>
#include<stdio.h>
#include<list>
#include<iterator>

using namespace std;

int main () {
int a = 2;
int arr[] = {1, 2, 3, 4, 5};
int brr[] = {1, 2, 3, 4, 5};
void * p1 = &a;
void * p2 = &arr;
void * p3 = &brr;
void * o1 = generic_box_simple(p1);
void * o2 = generic_box_unboxed_array(p2,2);
list<void *> ll;
ll.push_front(p2);
ll.push_front(p3);
list<POINTER_DESCR> ll2;
//void * o3 = generic_box_boxed_array(ll,2);
POINTER_DESCR d1 = { 0, 0 };
ll2.push_front(d1);
d1 = {sizeof(word_t), 1};
ll2.push_front(d1);
list<POINTER_DESCR>::iterator it = ll2.begin();
printf("первый: " "%i %i ",(int)((POINTER_DESCR )*it).offset, ((POINTER_DESCR )*it).boxed);
it++;
printf("второй: " "%i %i ",(int)((POINTER_DESCR )*it).offset, ((POINTER_DESCR )*it).boxed);
printf("lolololololo");
}

