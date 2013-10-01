#include </home/mariia/Desktop/object_repres/boxing.h>
#include <list>
#include <iterator>
#include <vector>
#include </home/mariia/Desktop/object_repres/prototype.h>
#include <stdio.h>
#include <string>


using namespace std;
struct Friend{ // друг
    struct Birthday{ // день рождения
        int day, year, month; // день год месяц рождения
    }BirthDay;
    string name; // имя
    string family; // фамалия
};

int main()
{
	int obj = -64732867;
	int arr[] = {127, 2, 3, 4, 5, 6, 7, 8, -1};
	void * p = &obj;
	void * p2 = &arr;
	void * res;
	void* arr2[] = {p, p2};
	list <void *> ll;
	res = generic_box_simple(p, 32);
	obj =  get_word(res,1);	
	printf("%i\n", obj);
	res = generic_box_unboxed_array(p2, 9, 16);
	word_t obj2 = get_word(res, 1);
	printf("%lu\n", obj2);
	printf("%d\n",(short)get_word(res,9));
	ll.push_front(p);
	ll.push_front(p2);
	res = generic_box_boxed_array(ll);
	printf("%p\n", p);
	printf("%p\n", p2);
	printf("%p\n", (void *)get_word(res, 1));
	printf("%p\n", (void *)get_word(res, 2));	 	 
	Friend f;
	f.name = "Oleg";
	f.BirthDay.day = 20;
	getchar();
	return 0;
}
