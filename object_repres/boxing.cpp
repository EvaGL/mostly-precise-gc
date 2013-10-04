#include </home/mariia/Desktop/object_repres/boxing.h>
#include <iterator>
#include <vector>
#include </home/mariia/Desktop/object_repres/prototype.h>
#include <cstdio>

using namespace std;
# define BYTESIZE  8

void * generic_box_simple (void * ptr_to_heap, size_t type) { 
	void * object = create_generic_object(1, 0, 0); 
	word_t value = 0;
	for (size_t j = 0; j < type / BYTESIZE; j++) {
		value |= (*((unsigned char *)ptr_to_heap + j)) << (j * BYTESIZE);
	}
	set_word(object, 1, value);
	return object;
}

void * generic_box_unboxed_array(void * ptr_to_heap, size_t len, size_t type) {
	void * object = create_unboxed_array(len); 
	for (size_t i = 1; i < len + 1; i++) {
		word_t value = 0;
		for (size_t j = 0; j < type / BYTESIZE; j++) {
			value |= (*((unsigned char *)ptr_to_heap + (i-1) * type / BYTESIZE + j)) << (j * BYTESIZE);
		}
		set_word(object, i, value);
	}
	return object;
}

void * generic_box_boxed_array (list<void *> ptr_to_heap)  { 
	size_t i = 1;
	list<void *> ::iterator it = ptr_to_heap.begin(); 
	void * object = create_boxed_array(ptr_to_heap.size()); // память под массив
	for (list<void *> ::iterator it = ptr_to_heap.begin(); it != ptr_to_heap.end(); it++, i++) {
		word_t value = ((word_t)(*it));//ptr_to_heap + i * sizeof(void *));
		set_word(object, i, value);
	}
	return object;
}

void * generic_box_struct (void * ptr_to_heap, size_t len, list <size_t> num_el_inobj,list <size_t> offsets, list<size_t> type,list<size_t>mod, size_t all_size, size_t num_ptr) { 
  // где len - количество элементов в структуре, num_el_inobj - количество элементов во каждом объекте структуры( например массиве), offsets - смещение каждого объекта( важно понимать, что для массивов 
  // и вложенных структур нам нужно их полное смещение, type - размер типа для объектов( в битах), нужен для вычисления смещения в куче, для записи каждого объекта массива, например, mod - говорит
  // в аll_size  размер вложенных структур не нужно включать мы храним указатель на них.(т. е. нужен только рамер указателей на вложенные структуры)
	// проходимся по всей структуре и записываем указатели в список
	all_size = all_size / sizeof(word_t) + 1; // вычисляем для отведения памяти в объекте 
	void * object2;
	void * object = create_generic_object(all_size, num_ptr, 0);
	object2 = object + sizeof(BLOCK_TAG) + 1 + (num_ptr * sizeof(POINTER_DESCR)); // указатель на начало слов
  // заранее должны знать число указателей(посчитать), структуры - как указатели
	list <size_t>::iterator it_offset1 = offsets.begin();
	list <size_t>::iterator it_mod = mod.begin();	
	list <size_t>::iterator it_offset2 = offsets.begin();
	it_offset2++;
	size_t offset = (char *)object2 - (char *)object;  
	list <size_t>::iterator it_type = type.begin();
	list <size_t>::iterator it_size = num_el_inobj.begin();
	size_t iter_w = 1;  //  следим за заполненностью слов
	unsigned char iter_p = 1;  // следим за заполненностью ячеек под дескриптор
	void * ptr_to_heap2;
	size_t len2;
	list<size_t> num_el_inobj2;
	list<size_t> offsets2;
	list<size_t> type2;
	list<size_t> mod2;
	size_t offset_diff;
	size_t num_ptr2;
	void * obj2;
	size_t all_size2;  // размер внутренней структуры;
	POINTER_DESCR descr;
	if ( *it_offset1 != 0) {
		offset_diff = *it_offset1 - *it_offset2;  // разность смещений, нужна для вычисления следующего указателя на объект в куче, при работе с вложенными струтурами
		it_offset1++;
		it_offset2++;
	} else {
		offset_diff = 0;
	}
 	
	for ( int i = 1; i < len + 1; i++, it_size++,it_type++,it_mod++) {
		switch(* it_mod) {
			case 0:  //простой объект
				for ( int k = 1; k < *it_size; k++) {
					word_t value = 0;
					for (size_t j = 0; j < *it_type / BYTESIZE; j++)
						value |= (*((unsigned char *)ptr_to_heap + j)) << (j * BYTESIZE);
					
					set_word(object, iter_w, value);
					iter_w++;
					offset += (*it_type) / BYTESIZE;
					ptr_to_heap += (*it_type) / BYTESIZE;
				}					
				
				break;	
			case 1:   // указатели 
				descr = { offset, 0};
				set_word(object, iter_w, (word_t)ptr_to_heap);  // записываем указатель
				set_ptr_descr(object, iter_p, descr);  // записываем смещение -- допистаь в prototype
				set_go_size(object, iter_p);  // перезаписываем количество указателей -- дописать в prototype
				iter_w++;
				iter_p++;
				offset += sizeof(void *);
				ptr_to_heap = ptr_to_heap + sizeof(it_size);
				break;
			case 2:
				descr = { offset, 1};
				set_word(object, iter_w, (word_t)ptr_to_heap);  // записываем указатель
				set_ptr_descr(object, iter_p, descr);  // записываем смещение -- допистаь в prototype
				set_go_size(object, iter_p);  // перезаписываем количество указателей -- дописать в prototype
				iter_w++;
				iter_p++;
				offset += sizeof(void *);
				ptr_to_heap = ptr_to_heap + sizeof(it_size);
				break;
			case 3:  //вложенная структура, структура
				ptr_to_heap2 = ptr_to_heap;
				//len2 = get_len_str(ptr_to_heap);  // нижеиспользованные функции пока не понятно откуда брать(=
				//num_el_inobj2 = get_num_elobj_str(ptr_to_heap);
				//offsets2 = get_offsets_str(ptr_to_heap);
				//type = get_type_str(ptr_to_heap);
				//mod2 = get_mod_str(ptr_to_heap);
				//all_size2 = get_size_str(ptr_to_heap);
				obj2 = generic_box_struct(ptr_to_heap2, len2, num_el_inobj2, offsets2, type2, mod2, all_size2, num_ptr2);
				descr = {offset, 1};  // т.к. это boxed структура				
				set_ptr(object, iter_p, obj2);
				set_ptr_descr(object, iter_p, descr);  // записываем смещение -- дописать в prototype
				set_go_size(object, iter_p);  // перезаписываем количество указателей -- дописать в prototype
				iter_w++;
				iter_p++;
				offset += sizeof(void *);
				ptr_to_heap = ptr_to_heap + offset_diff;
				break;
		}
	}
	return object;	
}