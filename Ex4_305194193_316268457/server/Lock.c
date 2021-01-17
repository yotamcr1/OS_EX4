#include "Lock.h"
//Authers: Chen Katz And Yotam Carmi
//this file contains the lock from the last exercise. 
//implement lock using mutex and semaphores.

lock* InitializLock(int num_of_threads) {
	HANDLE mutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // don't lock mutex immediately
		NULL);				//unnamed mutex
	if (mutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		exit(1);
	}
	HANDLE turnstile = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // don't lock mutex immediately
		NULL);				//unnamed mutex
	if (turnstile == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		exit(1);
	}
	HANDLE roomEmpty = CreateSemaphore(
		NULL,	// Default security attributes 
		1,		// Initial Count 
		num_of_threads,		// Maximum Count 
		NULL); // unnamed semaphore
	if (roomEmpty == NULL)
	{
		printf("CreateSemaphore error: %d\n", GetLastError());
		exit(1);
	}
	lock* new_lock = allocate_place_for_lock(mutex, turnstile, roomEmpty);
	return new_lock;
}

lock* allocate_place_for_lock(HANDLE mutex, HANDLE turnstile, HANDLE roomEmpty) {

	lock* new_lock = (lock*)malloc(sizeof(lock));

	if (new_lock != NULL) {
		new_lock->Mutex = mutex;
		new_lock->roomEmpty = roomEmpty;
		new_lock->turnstile = turnstile;
		new_lock->activeReaders = 0;
	}
	return new_lock;
}

void read_lock(lock* lock) {
	DWORD wait_code;
	BOOL ret_val;
	wait_code = WaitForSingleObject(lock->turnstile, TIMEOUT);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("Error when waiting for turnstile\n");
		return NULL;
	}
	ret_val = ReleaseMutex(lock->turnstile);
	if (FALSE == ret_val)
	{
		printf("Error when releasing turnstile\n");
		return NULL;
	}
	wait_code = WaitForSingleObject(lock->Mutex, TIMEOUT);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("Error when waiting for mutex\n");
		return NULL;
	}
	lock->activeReaders = lock->activeReaders + 1;
	if (lock->activeReaders == 1) {
		wait_code = WaitForSingleObject(lock->roomEmpty, TIMEOUT);
		if (WAIT_OBJECT_0 != wait_code)
		{
			printf("Error when waiting for roomEmpty\n");
			return NULL;
		}
	}
	ret_val = ReleaseMutex(lock->Mutex);
	if (FALSE == ret_val)
	{
		printf("Error when releasing mutex\n");
		return NULL;
	}
}

void read_release(lock* lock) {
	DWORD wait_code;
	BOOL ret_val;
	wait_code = WaitForSingleObject(lock->Mutex, TIMEOUT);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("Error when waiting for mutex\n");
		return NULL;
	}
	lock->activeReaders = lock->activeReaders - 1;
	if (lock->activeReaders == 0) {
		ret_val = ReleaseSemaphore(lock->roomEmpty, 1, NULL);
		if (FALSE == ret_val)
		{
			printf("Error when releasing roomEmpty\n");
			return NULL;
		}
	}
	ret_val = ReleaseMutex(lock->Mutex);
	if (FALSE == ret_val)
	{
		printf("Error when releasing mutex\n");
		return NULL;
	}
	return;
}

void write_lock(lock* lock) {
	DWORD wait_code;
	wait_code = WaitForSingleObject(lock->turnstile, TIMEOUT);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("Error when waiting for turnstile\n");
		return NULL;
	}
	wait_code = WaitForSingleObject(lock->roomEmpty, TIMEOUT);
	if (WAIT_OBJECT_0 != wait_code)
	{
		printf("Error when waiting for roomEmpty\n");
		return NULL;
	}
	return;
}

void write_release(lock* lock) {
	BOOL ret_val;
	ret_val = ReleaseMutex(lock->turnstile);
	if (FALSE == ret_val)
	{
		printf("Error when releasing turnstile\n");
		return NULL;
	}
	ret_val = ReleaseSemaphore(lock->roomEmpty, 1, NULL);
	if (FALSE == ret_val)
	{
		printf("Error when releasing roomEmpty\n");
		return NULL;
	}
}

void DestroyLock(lock* lock) {
	CloseHandle(lock->Mutex);
	CloseHandle(lock->roomEmpty);
	CloseHandle(lock->turnstile);
	free(lock);
	lock = NULL;
}
