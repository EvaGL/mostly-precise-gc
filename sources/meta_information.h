#pragma once
#include <pthread.h>

struct meta_holder{
    void* clMeta;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

template <typename T>
meta_holder* get_meta_holder() {
    static meta_holder holder;
    return &holder;
}

template <typename T>
void* get_meta() {
    meta_holder* holder = get_meta_holder<T>();
    pthread_mutex_lock(&holder->mutex);
    void* result = holder->clMeta;
    pthread_mutex_unlock(&holder->mutex);
    return result;
}

template <typename T>
void set_meta(void* clMeta) {
    meta_holder* holder = get_meta_holder<T>();
    pthread_mutex_lock(&holder->mutex);
    holder->clMeta = clMeta;
    pthread_mutex_unlock(&holder->mutex);
}