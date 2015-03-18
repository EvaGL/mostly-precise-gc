set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O3 -g")

set(precisegc_LIB_SRCS
    gc_new.cpp
    gc_new.h
    gc_ptr.h
    go.cpp
    go.h
    libprecisegc.h
    meta_information.cpp
    meta_information.h
    stack.cpp
    stack.h
    taginfo.cpp
    taginfo.h
    debug_print.h
    deref_roots.cpp)

add_library(precisegc SHARED ${precisegc_LIB_SRCS})

find_library(MSMALLOC_LIB msmalloc REQUIRED)
find_path(MSMALLOC_PATH msmalloc.h)
include_directories(${MSMALLOC_PATH})
target_link_libraries (precisegc ${MSMALLOC_LIB})

set_target_properties(precisegc PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS precisegc
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX})

install(FILES libprecisegc.h gc_new.h gc_ptr.h go.h meta_information.h stack.h taginfo.h debug_print.h deref_roots.h DESTINATION include/libprecisegc)