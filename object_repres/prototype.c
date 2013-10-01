# include </home/mariia/Desktop/object_repres/prototype.h>
# include <assert.h>

void * create_bitmap (size_t size, size_t descr_length, size_t count_descr...) {
    va_list args;
    void  * result = NULL;

    va_start (args, count_descr);

    if (count_descr == 0) {
      BLOCK_TAG tag = {TAG_MODEL_2, size, 0};
      result = malloc (size * sizeof (word_t) + sizeof (BLOCK_TAG) + 1);
      *(BLOCK_TAG *)result = tag;
    }
    else {
        BLOCK_TAG tag = {TAG_MODEL_0, size, 0};
        int bitmapsize = (0.125 * size) + 1;
        result = malloc (size*sizeof(word_t) + sizeof (BLOCK_TAG) + 2 * bitmapsize);
       *(BLOCK_TAG *)result = tag;
        unsigned int i = 0;
      for(; i < descr_length;i++) {
          POINTER_DESCR ptr_descr = va_arg(args, POINTER_DESCR);
          unsigned char * char_ptr = (unsigned char *)result + ptr_descr.offset / CHAR_BIT + sizeof(BLOCK_TAG);
          int bit_num = ptr_descr.offset - (ptr_descr.offset/CHAR_BIT) * CHAR_BIT;
          * char_ptr |= 1 << (8 - bit_num);
          if (ptr_descr.boxed)
            * (char_ptr + bitmapsize) |= 1 << (8 - bit_num);
        }
      }
    return result;
    }


void * create_generic_object (size_t size, size_t descr_length, size_t count_descr ...) {
  va_list args;
  void  * result = NULL;

  va_start (args, count_descr);

  if (count_descr == 0) {
    BLOCK_TAG tag = {TAG_MODEL_2, size, 0};

    result = malloc(size*sizeof (word_t) + sizeof (BLOCK_TAG) + 1);
    *(BLOCK_TAG *)result = tag;
  }
  else {
      BLOCK_TAG tag = {TAG_MODEL_1, size, 0};
      result = malloc (size * sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR));
     *(BLOCK_TAG *)result = tag;
     *((char *)result + sizeof(BLOCK_TAG)) = descr_length;
     unsigned int i = 0;
    for(; i < descr_length; i++) {
        *((POINTER_DESCR *)result + sizeof(BLOCK_TAG) + 1 + sizeof(POINTER_DESCR)*i) = va_arg(args, POINTER_DESCR);
    }
  }

  va_end (args);

  return result;
}

void set_ptr_descr (void* object, unsigned char iter_p, POINTER_DESCR descr) {
	*((POINTER_DESCR *)object + sizeof(BLOCK_TAG) + 1 + sizeof(POINTER_DESCR) * (size_t)(iter_p - 1)) = descr;
	return;
}
void set_go_size (void 	* object, unsigned char size) {
	//*((POINTER_DESCR *)object + sizeof(BLOCK_TAG)) = size;  
	*((char*)object+sizeof(BLOCK_TAG))=size;
	return;
} 
void * create_boxed_array(size_t size) {
    void * result = NULL;
    BLOCK_TAG tag = {TAG_MODEL_3, size, 0};
    result = malloc (size*sizeof (word_t) + sizeof (BLOCK_TAG) + 1);
    *(BLOCK_TAG *)result = tag;

    return result;
}
void * create_unboxed_array(size_t size) {
    void * result = NULL;
    BLOCK_TAG tag = {TAG_MODEL_4, size, 0};
    result = malloc (size*sizeof (word_t) + sizeof (BLOCK_TAG)+1);
    *(BLOCK_TAG *)result = tag;
    return result;
}
size_t get_size(void *object) {  // возвращает количество непустых слов
    BLOCK_TAG tag = *(BLOCK_TAG *)object;
    int bitmapsize;
    char descr_length;
    int size = 0;
    int j = 1;
    switch (((BLOCK_TAG *) object)->model) {
    case TAG_MODEL_1:
        //descr_length = *((char *)object + tag.size*sizeof(word_t) + sizeof (BLOCK_TAG));
        descr_length = *((char *)object + sizeof(BLOCK_TAG));
        for ( j; j < ((BLOCK_TAG *) object)->size + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
            if (get_word(object, j) == 0)
                return size;
            size++;
        }
        return size;
        //return (tag.size*sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR));
        break;
    case TAG_MODEL_2:
        for ( j; j < ((BLOCK_TAG *) object)->size + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
            if (get_word(object, j) == 0)
                return size;
            size++;
        }
        return size;
        //return (tag.size*sizeof (word_t) + sizeof (BLOCK_TAG));
        break;
    case TAG_MODEL_3:
        for ( j; j < ((BLOCK_TAG *) object)->size + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
            if (get_word(object, j) == 0)
                return size;
            size++;
        }
        // return (tag.size*sizeof (word_t) + sizeof (BLOCK_TAG));
        return size;
        break;
    case TAG_MODEL_4:
        for ( j; j < ((BLOCK_TAG *) object)->size + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
            if (get_word(object, j) == 0)
                return size;
            size++;
        }
        return size;
        //return (tag.size*sizeof (word_t) + sizeof (BLOCK_TAG));
        break;
    case TAG_MODEL_0:
        bitmapsize = (0.125 * size) + 1;
        for ( j; j < ((BLOCK_TAG *) object)->size + 1; j++) { // находим первую свободную ячейку(они свободны попарно)
            if (get_word(object, j) == 0)
                return size;
            size++;
        }
        return size;
        break;
    default:
        assert(0);
        return 0;

}
    }

void * copy_object (void * object) {
    void * new_obj = malloc(get_size(object));
    memcpy(new_obj, object, get_size(object));
    return new_obj;
}
word_t get_word (void * object, size_t index) {
    int bitmapsize;
    char descr_length;
    char * ptr_obj =  (char *) object;
    switch (((BLOCK_TAG *) object)->model) {
    case TAG_MODEL_1:
        //descr_length = *((char *)object + ((BLOCK_TAG *) object)->size*sizeof(word_t) + sizeof (BLOCK_TAG));
         descr_length =*((char *)object + sizeof(BLOCK_TAG));
        return *(word_t *)(ptr_obj + (index *sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR)));
        break;
    case TAG_MODEL_2:
        return *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_3:
        return *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_4:
        return *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_0:
        bitmapsize = 1+(0.125 * ((BLOCK_TAG *) object)->size);
        return *(word_t *)(ptr_obj + (index*sizeof(word_t) + sizeof (BLOCK_TAG) + 2 * bitmapsize));
        break;
    default:
        assert(0);
        return 0;
    }
}
void * get_ptr (void  * object, size_t index) {
    int bitmapsize;
    char descr_length;
    char * ptr_obj =  (char *) object;
    switch (((BLOCK_TAG *) object)->model) {
    case TAG_MODEL_1:
        //descr_length = *((char *)object + ((BLOCK_TAG *) object)->size*sizeof(word_t) + sizeof (BLOCK_TAG));
        descr_length = *((char *)object + sizeof(BLOCK_TAG));
        return (ptr_obj + (index *sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR))); 
        break;
    case TAG_MODEL_2:
        return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_3:
        return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_4:
        return (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG)));
        break;
    case TAG_MODEL_0:
        bitmapsize = 1+(0.125 * ((BLOCK_TAG *) object)->size);
        return (ptr_obj + (index*sizeof(word_t) + sizeof (BLOCK_TAG) + 2 * bitmapsize));
        break;
    default:
        assert(0);
        return 0;
    }
}

void set_word (void * object, size_t index, word_t data) {
    int bitmapsize;
    char descr_length;
    char * ptr_obj =  (char *) object;
    switch (((BLOCK_TAG *) object)->model) {
    case (TAG_MODEL_1):
          //descr_length = *((char *)object + ((BLOCK_TAG *) object)->size*sizeof(word_t) + sizeof (BLOCK_TAG));
        descr_length =*((char *)object + sizeof(BLOCK_TAG));
        *(word_t *)(ptr_obj + (index *sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR))) = data;
        break;
    case (TAG_MODEL_2):
        *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
        break;
    case (TAG_MODEL_3):
        *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
        break;
    case (TAG_MODEL_4):
        *(word_t *)(ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
        break;
    case (TAG_MODEL_0):
        bitmapsize = (0.125  * ((BLOCK_TAG *) object)->size)+1;
        *(word_t *)(ptr_obj + (index*sizeof(word_t) + sizeof (BLOCK_TAG) + 2 * bitmapsize)) = data;
        break;
    default:
        assert(0);
        return;

    }
}

void set_ptr (void  * object, size_t index, void * data) {
        int bitmapsize;
        char descr_length;
        char * ptr_obj =  (char *) object;
        switch (((BLOCK_TAG *) object)->model) {
        case TAG_MODEL_1:
              //descr_length = *((char *)object + ((BLOCK_TAG *) object)->size*sizeof(word_t) + sizeof (BLOCK_TAG));
            descr_length =*((char *)object + sizeof(BLOCK_TAG));
            *(void**) (ptr_obj + (index *sizeof(word_t) + sizeof (BLOCK_TAG) + 1 + descr_length*sizeof(POINTER_DESCR))) = data;
            break;
        case TAG_MODEL_2:
            *(void**) (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
            break;
        case TAG_MODEL_3:
            *(void**) (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
            break;
        case TAG_MODEL_4:
            *(void**) (ptr_obj + (index*sizeof (word_t) + sizeof (BLOCK_TAG))) = data;
            break;
        case TAG_MODEL_0:
            bitmapsize = 1+(0.125 * ((BLOCK_TAG *) object)->size);
            *(void**) (ptr_obj + (index*sizeof(word_t) + sizeof (BLOCK_TAG) + 2 * bitmapsize)) = data;
            break;
        default:
            assert(0);
            return;
        }
}

PTR_ITERATOR get_iterator (void * object) {
PTR_ITERATOR res_ptr;
char * begin_arr;
unsigned int i = 0;
char * ptr_obj =  (char *) object;
switch (((BLOCK_TAG *) object)->model) {
case TAG_MODEL_1:
    res_ptr.current = ((POINTER_DESCR *)((char *)ptr_obj + sizeof (BLOCK_TAG) + 1))->offset;  // это смещение указателя
    res_ptr.object = object;
    break;
case TAG_MODEL_2:
    res_ptr.object = NULL;  // указатель на конец(или отстутствие)
case TAG_MODEL_3:
    res_ptr.current = 0;
    res_ptr.object = object;  // это индекс элемента в массиве
    break;
case TAG_MODEL_4:
    res_ptr.object = NULL;  // указатель на конец(или отстутствие)
    break;
case TAG_MODEL_0:
    begin_arr = (ptr_obj + sizeof (BLOCK_TAG));
    for(;i<((BLOCK_TAG *)object)->size;i++) {
        if (*(begin_arr + i / CHAR_BIT) & (1 << i % CHAR_BIT) ) {
            res_ptr.current = i;
            res_ptr.object = object;
            break;
        }
    }
    break;
}
return res_ptr;
}

void * next_ptr (PTR_ITERATOR * iterator) {
    void * res_ptr;
    unsigned int i = 0;
    char * begin_arr;
    switch (((BLOCK_TAG *)(iterator->object))->model) {
    case TAG_MODEL_1:
        res_ptr = get_ptr(iterator->object,iterator->current);
        while(i < ((BLOCK_TAG *)(iterator->object))->size && iterator->current != ((POINTER_DESCR *)((char *)(iterator->object) + sizeof (BLOCK_TAG) + 1 + i * sizeof(POINTER_DESCR)))->offset) {
            i++;
        }
        if(i == ((BLOCK_TAG *)(iterator->object))->size)
            iterator->object = NULL;
        iterator->current = ((POINTER_DESCR *)((char *)(iterator->object) + sizeof (BLOCK_TAG) + 1 + i * sizeof(POINTER_DESCR)))->offset;
        break;
    case TAG_MODEL_2:
        iterator->object = NULL;  // указатель на конец(или отстутствие)
    case TAG_MODEL_3:
        res_ptr = get_ptr(iterator->object,iterator->current);
        iterator->current++;
        break;
    case TAG_MODEL_4:
        iterator->object = NULL;  // указатель на конец(или отстутствие)
        break;
    case TAG_MODEL_0:
        res_ptr = get_ptr(iterator->object,iterator->current);
        begin_arr = ((char *)iterator->object + sizeof (BLOCK_TAG));
        i = iterator->current;
        for(;i<((BLOCK_TAG *)(iterator->object))->size;i++) {
            if (*(begin_arr + i / CHAR_BIT) & (1 << i % CHAR_BIT) ) {
                iterator->current = i;
                break;
            }
        }
        if(i == ((BLOCK_TAG *)(iterator->object))->size)
            iterator->object = NULL;
        break;
    default:
        assert(0);
        return 0;
    }
    return res_ptr;
}

