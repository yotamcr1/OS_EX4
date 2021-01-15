#ifndef SERVER_H
#define SERVER_H

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#define NUM_OF_WORKER_THREADS 2
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
#include "SocketSendRecvTools.h"
#include "Massage.h"
#include "Lock.h"

void write_to_file(HANDLE file, char* str);

void read_from_file(HANDLE file, char* str);

void initialize_mutex();

void MainServer(int ServerPort);
static int FindFirstUnusedThreadSlot();

DWORD get_file_orig_size(HANDLE file);
void get_client_name(char* client_request_massage, char* destination_client_name);
int send_massage(char* str, SOCKET* t_socket);
static DWORD ServiceThread(SOCKET* t_socket);
int get_4digit_number_from_massage(char* str);
void game_first_client(SOCKET* t_socket, HANDLE game_file, char* client_name);
void game_calculate_and_update_status(int numA, int numB, int A_guess_B, int B_guess_A);
void read_file_get_opponent_user_name(int am_i_first, char* Oponent_Client_Name, int my_user_name_size);
int write_client_name_to_game_file(int* am_i_first, char* Client_Name, int client_name_length);

#endif