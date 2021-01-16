#ifndef SERVER_H
#define SERVER_H

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define FILE_NAME "GameSession.txt"
#define LENGTH_EXIT 6
#define NUM_OF_WORKER_THREADS 3
#define SERVER_ADDRESS_STR "127.0.0.1"
#define DEFAULT_BUFLEN 512
#define SEND_STR_SIZE 120
#define MAX_WRITE_BYTES_TO_GAME_FILE 200 //TBD: check if it neccessary and the size 
#define SERVER_TIMEOUT 15000
#define BLOCKING_TIMEOUT 0

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "SocketSendRecvTools.h"
#include "Massage.h"
#include "Lock.h"

//writes string str to file. inputs: handle to a file, and string to write.
void write_to_file(HANDLE file, char* str);

//reads from a file and store it within str. inputs: file handle, and str buffer.
void read_from_file(HANDLE file, char* str);

//kills the locks and free memory. no inputs, no outputs (the locks are global parameters because they shared by the threads.)
void kill_locks();

//kill semaphores and free memory.no inputs, no outputs (the semaphores are global parameters because they shared by the threads.)
void kill_semaphores();

//initialize the semapohres
void initialize_semaphore();

//listening to the CMD terminal of server program, and check every 15 seconds if exit was written, if yes - close the server
static DWORD listening_to_cmd_keystroke();

//main server. open the socket that the server is listening to, and open a thread for every new connection. input - server port
void MainServer(int ServerPort);

//find the first unused thread slot within the global handle thread array
static int FindFirstUnusedThreadSlot();

//input - file handle, output - dword file size
DWORD get_file_orig_size(HANDLE file);

//get the client name from the client massage. input - client request massage: the massage sent by the client, and destionation buffer. the buffer is initialized with the client name
void get_client_name(char* client_request_massage, char* destination_client_name);

//send massage from server to the client and check for errors during the send. input - str of the massage, and the socket to send. return 0 on fails, 1 otherwise
int send_massage(char* str, SOCKET* t_socket);

//write the guess number to the file game. am i first determine who is the first client, and my guess is the 4 guess numbers.
void write_geuss_number_to_game_file(int am_i_first, int my_geuss);

//forced the threads to sync, using semaphores.
void sync_function();

//build the format of win results massage sent by the server to the client
//inputs: client name, secret number of the other, and temp buffer which will contains the string
void format_win_result(char* Client_Name, int other_secret_number, char* temp_buffer);

//used to free memory of str and set it back to null, for compability with Recived massage
void check_if_str_is_allocated(char** str);

//free the resources of the thread. close the socket, free the accpetedstr and return 1.
void free_service_thread_memories(SOCKET* t_socket, char** AcceptedStr);


static DWORD ServiceThread(SOCKET* t_socket);

int calculate_game_result(int am_i_first, int* my_cows, int* my_bulls, int my_secret_number, int* other_secret_number, int* oppnent_cows, int* oponent_bulls, int* my_geuss, int* other_client_geuss);

int write_client_name_to_game_file(int* am_i_first, char* Client_Name, int client_name_length);

void read_file_get_opponent_user_name(int am_i_first, char* Oponent_Client_Name, int my_user_name_size);

int get_4digit_number_from_massage(char* str);

void game_calculate_and_update_status(int oponent_secret_number, int my_geuss, int* cows, int* bulls);

int gracefull_server_shutdown(char* AcceptedStr);

void format_game_results(int my_bulls, int my_cows, char* Oponent_Client_Name, int other_client_geuss, char* temp_buffer);

#endif