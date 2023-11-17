#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */

void demonstration(std::function<void()> && lambda) {
    lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads);
void parallel_for(int low1, int high1, int low2, int high2,
std::function<void(int, int)> &&lambda, int numThreads);

#define main user_main

typedef struct {
    int start;
    int end;
    std::function<void(int)>* func; 
}thread_args;

void *thread_func(void *ptr) {
    thread_args *t = static_cast<thread_args*>(ptr);
    for (int i = t->start; i < t->end; i++) {
        (*(t->func))(i);
    }
    return NULL;
}

void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
   auto start_time = std::chrono::high_resolution_clock::now();
    pthread_t tid[numThreads];
    thread_args args[numThreads];
    int chunkSize = (high - low) / numThreads;
    int remainder = (high - low) % numThreads;

    
    for (int i = 0; i < numThreads; i++) {
        args[i].start = i * chunkSize;
        if (i==(numThreads-1)){
          args[i].end = (i+1)*(chunkSize)+remainder;
          }
        else{
          args[i].end = (i + 1) * chunkSize;
        }
        args[i].func = (&lambda);//&lambdaArray[i];  // Pass the address of the i-th element

        pthread_create(&tid[i], NULL, thread_func, static_cast<void*>(&args[i]));
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(tid[i], NULL);
    }

     auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  printf("Time taken in parallelfor1: %lld miliseconds\n", static_cast<long long>(duration.count())/1000);
    
}

typedef struct {
  int start;
  int end;
  int size;
  std::function<void(int,int)>*func;
}mat_thread;

void *mat_thread_func(void *ptr){
  mat_thread* t = static_cast<mat_thread*>(ptr);
  for (int i= t->start;i<t->end;i++){
    for (int j=0; j< t->size; j++){
         (*(t->func))(i,j);
    }
  }
  return NULL;
}
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads){
  auto start_time = std::chrono::high_resolution_clock::now();
  pthread_t tid[numThreads];
  mat_thread args[numThreads];
  int chunkSize = (high1-low1)/numThreads;

  for (int i=0;i<numThreads;i++){
       args[i].start=i*chunkSize;
       if (i==numThreads-1){
        args[i].end=high1;
       }
       else{
        args[i].end=(i+1)*chunkSize;
       }
       args[i].func = &lambda;
       args[i].size = (high2-low2);
       pthread_create(&tid[i], NULL, mat_thread_func, static_cast<void*>(&args[i]));

  }

   for (int i = 0; i < numThreads; i++) {
        pthread_join(tid[i], NULL);
    }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  printf("Time taken in parallelfor2 :: %lld miliseconds\n", static_cast<long long>(duration.count())/1000);
}
