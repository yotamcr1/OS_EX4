#ifndef CLIENT_H
#define CLIENT_H

void handle_diconnecting(int iResult, SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address);
void ClientMain(char* username, int serverport, unsigned long serverIP_Address);




#endif