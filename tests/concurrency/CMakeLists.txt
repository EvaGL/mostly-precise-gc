set(TEST concurrency)

set(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
file(GLOB SOURCE "*.cpp")
add_executable(${TEST} EXCLUDE_FROM_ALL ${SOURCE})

find_path(PRECISEGC_PATH libprecisegc/libprecisegc.h)
include_directories(${PRECISEGC_PATH})
find_path(MSMALLOC_PATH msmalloc.h)
include_directories(${MSMALLOC_PATH})

find_library(MSMALLOC_LIB msmalloc REQUIRED)
find_library(PRECISEGC_LIB precisegc REQUIRED)
find_library(PTHREAD_LIB pthread REQUIRED)

set(LIBS ${MSMALLOC_LIB} ${PRECISEGC_LIB} ${PTHREAD_LIB})
target_link_libraries(${TEST} ${LIBS})