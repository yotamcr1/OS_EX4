#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>

void handle_connection_problems(SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address, int server_denied_flag);
void ClientMain(char* username, int serverport, unsigned long serverIP_Address);
void extract_game_results(char* AcceptedStr, char* Bulls, char* Cows, char* opponent_username, char* opponent_guess);
void extract_winner_name_and_opponent_number(char* AcceptedStr, char* winner_name, char* opponent_number);


#endif