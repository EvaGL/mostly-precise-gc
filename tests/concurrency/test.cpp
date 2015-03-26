//
// Created by evagl on 26.03.15.
//
#include <libprecisegc/libprecisegc.h>
#include <libprecisegc/threading.h>
void* routine(void* num) {
    int n = *(int*) num;
    sleep(n);
    printf("Number is %d\n", n);
}

int main() {
    pthread_t thread1;
    pthread_t thread2;
    int one = 1;
    int two = 2;
    thread_create(&thread2, nullptr, routine, &two);
    thread_create(&thread1, nullptr, routine, &one);
    void* res;
    thread_join(thread1, &res);
    thread_join(thread2, &res);
    return 0;
}