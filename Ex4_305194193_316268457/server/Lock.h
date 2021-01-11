#ifndef LOCK_H_
#define LOCK_H_

//Authers: Chen Katz And Yotam Carmi

#include <stdio.h>
#include <stdbool.h>
#include <math.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>

#define TIMEOUT 10000 //TBD: CHANGE THE VALUE TO REALLY 15 SEC
#define TIMEOUT_15SEC 100000 //TBD: CHANGE THE VALUE TO REALLY 15 SEC
#define TIMEOUT_30SEC 100000 //TBD: CHANGE THE VALUE TO REALLY 15 SEC

//struct definition:

typedef struct lock {
	int activeReaders;
	HANDLE Mutex; //mutex
	HANDLE turnstile;//mutex
	HANDLE roomEmpty; //semaphore
}lock;


//lock functions declaration:


//the function create all the mutex and semaphore for the struct and initialize the lock struct
//input: number of threads
//output: initialized struct of lock
lock* InitializLock(int num_of_threads);

//the function allocate place for the lock and fill his fields
//input: handles to 2 mutex and 1 semaphore
//output: initialized lock struct
lock* allocate_place_for_lock(HANDLE mutex, HANDLE turnstile, HANDLE roomEmpty);

//The function performs a lock for reading
//threads can read at the same time but while another thread is writing.
//input: pointer to lock struct
void read_lock(lock* lock);

//The function performs a release of the lock from a reading lock made by the same thread
//input: pointer to lock struct
void read_release(lock* lock);

//The function performs a lock for writing
//input: pointer to lock struct
void write_lock(lock* lock);

//The function performs a release of the lock from a writing lock made by the same thread
//input: pointer to struct lock
void write_release(lock* lock);

//the function release all the resources of the lock
//input: pointer to struct lock
void DestroyLock(lock* lock);



#endif
