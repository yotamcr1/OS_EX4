#ifndef CLIENT_H
#define CLIENT_H
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define CLIENT_TIMEOUT 30000 //timeout in miliseconds
#define BLOCKING_TIMEOUT 0
#define SERVER_TIMEOUT 15000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void handle_connection_problems(SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address, int server_denied_flag);
int gracefull_client_shutdown(SOCKET m_socket, char* AcceptedStr);
void ClientMain(char* username, int serverport, char* serverIP_Address_str);
void extract_game_results(char* AcceptedStr, char* Bulls, char* Cows, char* opponent_username, char* opponent_guess);
void extract_winner_name_and_opponent_number(char* AcceptedStr, char* winner_name, char* opponent_number);
void game_routine(SOCKET m_socket);

#endif