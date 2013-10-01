# include <stdio.h>
# include "/home/mary/урур/prototype.h"
//#include <prototype.h>

void  test_model4() {  // создаем массив unboxed, заполняем элементами
    int arr[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    void * arr_gc = create_unboxed_array(10);
    int i;
    for(i = 0 ; i<10 ; i++) {
        set_word(arr_gc,i,(word_t)(&arr[i]));
        printf ("num_spec_arr: %i\n",i);
        printf ("elem_spec_arr: %i\n", (int)get_word((word_t *)arr_gc,i));
    }
    printf ("model: %u\n", ((BLOCK_TAG *)arr_gc)->model);
    printf ("size : %i\n",(int)((BLOCK_TAG *)arr_gc)->size);
    return;
}
void  test_model3() {  // создаем массив boxed, заполняем элементами
    int arr[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    void * arr_gc2 = create_generic_object(10,0);
    int i;
    for(i = 0 ; i<10 ; i++) {
        set_word(arr_gc2,i, (word_t)(&arr[i]));
        printf ("num_spec_arr: %i\n",i);
        printf ("elem_spec_arr: %i\n",(int) get_word(arr_gc2,i));
    }
    void * arr_gc = create_boxed_array(2);
    set_word(arr_gc,1,(word_t)arr[i]);
    set_ptr(arr_gc,2, arr_gc2);
    printf ("model: %u\n", ((BLOCK_TAG *)arr_gc)->model);
    printf ("size : %i\n",(int)((BLOCK_TAG *)arr_gc)->size);
    return;
}
void  test_model2() {  // создаем объект без указателей
    int arr[] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    void * arr_gc = create_generic_object(10,0);
    int i;
    for(i = 0 ; i<10 ; i++) {
        set_word(arr_gc,i, (word_t)arr[i]);
        printf ("num_spec_arr: %i\n",i);
        printf ("elem_spec_arr: %i\n",(int) get_word(arr_gc,i));
    }
    i = 0;
    printf ("model: %u\n", ((BLOCK_TAG *)arr_gc)->model);
    printf ("size : %i\n",(int)((BLOCK_TAG *)arr_gc)->size);
    return;
}
void test_model1() {  // создаем структуру со смешанными данными, храним смещение указателей.
   int arr[] = {11, 12, 13};
   void * ptr = &arr[1];
    float num1 = 1.0980;
    POINTER_DESCR p1 = { .offset = 0, .boxed = 1 };
    POINTER_DESCR p2 = { .offset = 0 + sizeof(num1), .boxed = 0 };
    void * arr_gc = create_generic_object(4,2,p1,p2);
        set_word(arr_gc, 1, arr[0]);
        set_ptr(arr_gc, 2, ptr);
        set_ptr(arr_gc, 3, &num1);
        set_word(arr_gc, 4, arr[1]);
    printf ("model: %u\n", ((BLOCK_TAG *)arr_gc)->model);
    printf ("size : %i\n",(int)((BLOCK_TAG *)arr_gc)->size);
    return;
}
void test_model0() {  // создаем структуру со смешанными данными, храним битовые массивы указателей.
   int arr[] = {11, 12, 13};
   void * ptr = &arr[1];
   float num1 = 1.0980;
   POINTER_DESCR p1 = { .offset = 0, .boxed = 1 };
   POINTER_DESCR p2 = { .offset = 0 + sizeof(num1), .boxed = 0 };
   void * arr_gc = create_bitmap(4,2,p1,p2);
        set_word(arr_gc, 1, arr[0]);
        set_ptr(arr_gc, 2, ptr);
        set_ptr(arr_gc, 3, &num1);
        set_word(arr_gc, 4, arr[1]);
   printf ("model: %u\n", ((BLOCK_TAG *)arr_gc)->model);
   printf ("size : %i\n",(int)((BLOCK_TAG *)arr_gc)->size);
   return;
}

void  create_name(char * name, void * res) {
  size_t len = strlen(name);
  size_t i = 1;
  for (; i < len + 1; i++) {
    set_word(res, i, name[i-1]);
  }
  return ;
}


size_t start_letter(void * obj, char  let) {
    if (get_word(obj, 1) == (word_t)let)
    return 1;
  return 0;
}

size_t get_num_name(void * list_client_allworkers, size_t id, char let) {
	void * ls_i = ((void *)get_word(list_client_allworkers, id));
	size_t count = get_size(ls_i);
	size_t i = 1;
	size_t j = 0;
	void * tmp;
		  for (; i < count + 1; i++) {
			  tmp =get_word(ls_i, i);
		if ( start_letter(tmp, let) == 1) {
		  j++;
		}
		  }
	return j;
}


void  add_workers(void * list,int count, ...) { // добавление пары: id, рабочий
  va_list args;
  va_start (args, count);
  size_t i = 1;
  size_t j = 1;
  size_t len = get_size(list);
  if (count != 0) // если имен введено не нуль 
  {
    for ( ; j < len + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
      if ( get_word(list, j) == 0)
      break;
    }
    if( j == len) {
      printf("свободных мест нет");
      return;
    }
    else {
    for ( ; i < (size_t)(count+1); i++) {  // записываем id и имя работника(сколько их в аргументах)
      char * namels = va_arg(args, char *);
      void * name = create_generic_object(strlen(namels),0);
      create_name(namels,name);
      set_word(list, j, (j+1)/2 );
      set_ptr(list, j+1, name);
      j+=2;
     }
    }
  }

  return;
}


size_t paste_name(void * list_cl, void * list, size_t n, size_t j, char let) {
	void * ls_i = (void *)get_word(list,n);
	size_t count = get_size(ls_i);
	size_t i = 1;
	for (; i < count + 1; i++) {
	  if (start_letter( (void *)get_word(ls_i, i), let) == 1) {
		set_word(list_cl, j, get_word(ls_i, i));
		j++;
	  }
	}
	return j;
}





void * select_client(void * list_client_allworkers, char ch, size_t count, ...)  {  // вывести всех клиентов count работников на букву 'a'
  va_list args;
  va_start (args, count);
  void * list_cl;
  size_t all_num = 0;
  size_t i = 1;
  size_t past = 1;
  void * indx = create_unboxed_array(count);
  for (; i < count + 1; i++) {
	all_num += get_num_name(list_client_allworkers, i, ch); //возвращает количество имен на такую букву(ch) в списке клиентов рабочего с таким id 
    set_word(indx, count, i);
  }
  list_cl = create_boxed_array(all_num);
  i = 1;
  for (; i < count + 1; i++) {
    past = paste_name(list_cl, list_client_allworkers, i, past, ch);
  }
  return list_cl;
}

void  add_client(void * list, size_t count, ...) {  // добавление клиента в список рабочего
    va_list args;
    va_start (args, count);
    size_t i = 1;
    size_t j = 1;
    size_t len = get_size(list);
    if (count != 0) // если имен введено не нуль
    {
      for (; j < len + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
          if ( get_word(list, j) == 0)
      break;
      }
      if( j == len) {
        return;
      }
      else {
      for (; i < count + 1; i++) {
        char * namels = va_arg(args, char *);
        void * name = create_generic_object(strlen(namels),0);
        create_name(namels,name);
        set_ptr(list, j, name);
        j++;
       }
      }
    }
}

void print_list_workers(void * list) {
    size_t len = get_size(list);
    size_t len2;
    void * nam1;
    size_t i = 2;
    size_t j ;
    word_t tmp;
    for (; i < (size_t)(len+1); i += 2) {
        tmp = get_word(list, i-1);
        printf("работник: " "%i ", (int)tmp);
        nam1 = (void *)get_word(list, i);
        if (nam1 != 0){
            len2 = get_size(nam1);
            for (j = 1; j < len2+1; j++) {
                tmp = get_word(nam1, j);
                printf("%c", tmp, tmp);
            }
            printf("\n");
        }
    }
}
void print_list_clients(void * list) {
    size_t len = get_size(list);
    size_t len2;
    void * nam1;
    size_t i = 1;
    size_t j;
    word_t tmp;
    for (; i< len+1; i++) {
        nam1 = (void *)get_word(list, i);
        if (nam1 != 0){
            len2 = get_size(nam1);
        for (j = 1; j < len2+1; j++) {
            tmp = get_word(nam1, j);
            printf("%c", tmp, tmp);
        }
        printf("\n");
    }
    }
}

int main () {
size_t ind[] = {1, 2, 3};
  
void * list_workers1;
void * list_workers2;

void * list_clients1 = create_boxed_array(15);
void * list_clients2 = create_boxed_array(15);
void * list_clients3 = create_boxed_array(15);


POINTER_DESCR p1 = { .offset = 0, .boxed = 1};
POINTER_DESCR p2 = { .offset = sizeof(p1) + sizeof(ind[0]), .boxed = 1};
POINTER_DESCR p3 = { .offset = sizeof(p1) + sizeof(ind[0]) + sizeof(p2) + sizeof(ind[1]), .boxed = 1};

list_workers1 = create_generic_object(6, 3, p1, p2, p3);
list_workers2 = create_bitmap(6, 3, p1, p2, p3);

add_workers(list_workers1, 3, "lissi", "kttttl","susan");
add_workers(list_workers2, 3, "lis", "jake","san");

add_client(list_clients1, 5, "otto", "aan", "alays", "nata","deppy");
add_client(list_clients2, 3, "grey", "stenny", "alis");
add_client(list_clients3, 6, "mary", "max", "katty", "alya", "asssa","ark");

void * list_client_allworkers = create_unboxed_array(3);

set_ptr(list_client_allworkers, 1, list_clients1);
set_ptr(list_client_allworkers, 2, list_clients2);
set_ptr(list_client_allworkers, 3, list_clients3);

char letter = 'm';
printf("список работников 1:" "\n");
print_list_workers(list_workers2);
printf("список работников 2:" "\n");
print_list_workers(list_workers1);
printf("список клиентов 1:" "\n");
print_list_clients(list_clients1);
printf("список клиентов 2:" "\n ");
print_list_clients(list_clients2);
printf("список клиентов 3:" "\n");
print_list_clients(list_clients3);
printf("результат селекта:");
printf("клиенты на букву " "%c\n", letter);
void * result = select_client(list_client_allworkers, letter, 3);
print_list_clients(result);
return 1;
}
