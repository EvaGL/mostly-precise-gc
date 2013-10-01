# include <stdlib.h>
# include <limits.h>
# include <stdarg.h>
# include <math.h>
# include <string.h>

# ifndef __WORDSIZE
  # error Cannot identify __WORDSIZE
# endif

# define MODEL_NUMBER_WIDTH  3 // моделей 5
# define MARK_BIT	     1 
# define SIZE_WIDTH         (sizeof(word_t) - 4)

//# if MODEL_NUMBER_WIDTH+MARK_BIT+SIZE_WIDTH != __WORDSIZE
//  # error Invalid MODEL_NUMBER_WIDTH
//# endif

typedef unsigned long long word_t;

# define TAG_MODEL_0 0x0
# define TAG_MODEL_1 0x1
# define TAG_MODEL_2 0x2
# define TAG_MODEL_3 0x3
# define TAG_MODEL_4 0x4

typedef struct {
  unsigned      model : MODEL_NUMBER_WIDTH;
  unsigned long size  : SIZE_WIDTH;
  unsigned	mbit  : MARK_BIT;
} BLOCK_TAG;

typedef struct {
  size_t offset;
  int    boxed;
} POINTER_DESCR;

typedef struct {
  void   * object;
  size_t   current;
} PTR_ITERATOR;

extern void *       create_generic_object (size_t size, size_t descr_length, size_t count_descr, ...);
extern void *       create_boxed_array    (size_t size);
extern void *       create_unboxed_array  (size_t size);
extern void *       copy_object           (void         * object);
extern size_t       get_size              (void         * object);
extern word_t       get_word              (void         * object, size_t index);
extern void *       get_ptr               (void         * object, size_t index); 
extern void         set_word              (void         * object, size_t index, word_t data);
extern void         set_ptr               (void         * object, size_t index, void * data);
extern void         set_ptr_descr         (void         * object, unsigned char iter_p, POINTER_DESCR desr);  // для generic_object функция (добавлет дескриптор)
extern void	    set_go_size           (void 	* object, unsigned char size); // для generic_object функция( меняет колличество указателей)
extern PTR_ITERATOR get_iterator          (void         * object);
extern void *       next_ptr              (PTR_ITERATOR * iterator);
extern void *       create_bitmap         (size_t size, size_t descr_length, size_t count_descr, ...);

