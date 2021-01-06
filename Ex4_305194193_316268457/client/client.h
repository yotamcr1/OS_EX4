#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>

void handle_connection_problems(SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address, int server_denied_flag);
void ClientMain(char* username, int serverport, unsigned long serverIP_Address);




#endif