#ifndef CLIENT_H
#define CLIENT_H
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//Authers: Chen Katz And Yotam Carmi

#define CLIENT_TIMEOUT 30000 //timeout in miliseconds
#define BLOCKING_TIMEOUT 0
#define SERVER_TIMEOUT 15000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>



void ClientMain(char* username, int serverport, char* serverIP_Address_str);

void game_routine(SOCKET m_socket);

void extract_winner_name_and_opponent_number(char* AcceptedStr, char* winner_name, char* opponent_number);

//the function extracts the game result from the recieved string from the server
// input: recieved string from server, strings of each result from the server that will be fill by the function
void extract_game_results(char* AcceptedStr, char* Bulls, char* Cows, char* opponent_username, char* opponent_guess);

//the function handle cases of: 1. unexpected disconnecting from the server 2. timeout 3.when server denied connection
//input: clientservice, port, ip address, server_denied_flag- if its 1 will handle server denied case, else handle disconnecting
void handle_connection_problems(SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address, int server_denied_flag);

//for every shutdown we have to gracefully shutdown the client:
//In a graceful shutdown, any data that has been queued, but not yet transmitted can be sent prior to the connection being closed
int gracefull_client_shutdown(SOCKET m_socket, char* AcceptedStr);




#endif


