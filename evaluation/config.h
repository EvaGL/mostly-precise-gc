#ifdef BOEHM_GC
   #include "gc.h"
   #define ptype(T) T*
   #define alloc(T) new(GCNEW(T)) T()
   #define alloc_args(T, args...) new(GCNEW(T)) T(args)
   #define HAS_DESTRUCTOR 0
   #define free(obj) {}
   #define setnull(ptr) ptr = 0
#endif
#ifdef NO_GC
   #define ptype(T) T*
   #define alloc(T) new T()
   #define alloc_args(T, args...) new T(args)
   #define HAS_DESTRUCTOR 1
   #define free(obj) delete obj
   #define setnull(ptr) ptr = 0
#endif
#ifdef SHARED
   #include <memory>
   template< typename T >
   struct array_deleter
   {
      void operator ()( T const * p)
      { 
        delete[] p; 
      }
    };
   #define ptype(T) shared_ptr<T>
   #define alloc(T) make_shared<T>()
   #define alloc_args(T, args...) make_shared<T>(args)
   #define HAS_DESTRUCTOR 0
   #define free(obj) {}
   #define setnull(ptr) ptr.reset() 
#endif
#ifdef MOSTLY_PRECISE
   #include <libprecisegc/libprecisegc.h>
   #define ptype(T) gc_ptr<T>
   #define alloc(T) gc_new<T>()
   #define alloc_args(T, args...) gc_new<T, ptype(T), ptype(T)>(args)
   #define HAS_DESTRUCTOR 0
   #define free(obj) {}
   #define setnull(ptr) ptr.setNULL()
#endif
#ifdef GCPP
  #include <gc++.hpp>
  #define ptype(T) GCpp::gc_ptr<T>
  #define alloc(T) GCpp::gc_ptr<T>(gcnew (T))
  #define alloc_args(T, args...) GCpp::gc_ptr<T>(gcnew(T) (args))
  #define HAS_DESTRUCTOR 0
  #define free(obj) {}
  #define setnull(ptr) ptr = 0
#endif

