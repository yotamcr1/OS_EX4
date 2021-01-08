
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "Massage.h"
#include "client.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"

SOCKET m_socket;


//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[5];// the sent massage is 4 ints 
	TransferResult_t SendRes;
	while (1)
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		SendRes = SendString(SendStr, m_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
	}
}



//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;

	while (1)
	{
		char* AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Server closed connection. Bye!\n");
			return 0x555;
		}
		else
		{
			printf("%s\n", AcceptedStr);
		}

		free(AcceptedStr);
	}

	return 0;
}



void ClientMain(char* username, int serverport, unsigned long serverIP_Address) {
	printf("welcom to ClientMain\n");
	SOCKADDR_IN clientService;
	int answer_num;
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	char Massage_type_str[MAX_MASSAGE_TYPE];
	char* AcceptedStr = NULL;
	//Call WSAStartup and check for errors.
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("WSAStartup function failed with error : % d\n", iResult);
	}
	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {// Check for errors to ensure that the socket is a valid socket.
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return;//TBD: return 1?
	}

	 //Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;//AF_INET is the Internet address family
	clientService.sin_addr.s_addr = inet_addr(serverIP_Address); //Setting the IP address to connect to
	clientService.sin_port = htons(serverport); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.

	iResult = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) {
		handle_diconnecting(clientService, serverport, serverIP_Address,0);
	}
	printf("Connected to server on %lu:%d\n", serverIP_Address, serverport);//connected successfully

	//send the client name to the server:
	//TBD: the sent massage has to be CLIENT_REQUEST and the recv: SERVER_APPROVED

	if ((SendString(username, m_socket)) == TRNS_FAILED)// send client user name to the server
	{
		printf("error while sending username to server\n");
		return 0x555;
	}
	/*TBD: use setsocket for getting answer from the server
	if there is no answer from server: handle_diconnecting(clientService,serverport,serverIP_Address,0);
	*/
	int massage_type = receive_msg(m_socket);
	if (massage_type == SERVER_DENIED) {
		handle_connection_problems(clientService, serverport, serverIP_Address,1);
	}
	if (massage_type == SERVER_APPROVED) {
		massage_type = receive_msg(m_socket);
		while ((massage_type == SERVER_MAIN_MENU)||(massage_type==SERVER_NO_OPPONENTS)) {
			printf("Choose what to do next:\n 1. Play against another client\n2. Quit\n");
			scanf_s("%d", &answer_num);
			if (answer_num == 1) {
				if ((SendString(CLIENT_VERSUS_MSG, m_socket)) == TRNS_FAILED)
				{
					printf("error while sending CLIENT_VERSUS_MSG to server\n");
					return 0x555;
				}
			}
			else {//send to server client disconnect and exit
				if ((SendString(CLIENT_DISCONNECT_MSG, m_socket)) == TRNS_FAILED)
				{
					printf("error while sending CLIENT_DISCONNECT to server\n");
					return 0x555;
				}
				WSACleanup();//TBD: check if all resources are free:
				iResult = closesocket(m_socket);
				if (iResult == SOCKET_ERROR)
					printf("closesocket function failed with error: %ld\n", WSAGetLastError());
				exit(1);//TBD: call to gracefull shutdown function for client
			}
			massage_type = receive_msg(m_socket);
			//TBD: have to wait 30 seconds for answer- HOW?
			if (massage_type == SERVER_INVITE) {
				printf("Game is on!\n");
				//TBD: CALL GAME FUNCTION
				game_routine(m_socket);
				//!!!!TBD: POINT 9 IN CLIENT RUN DETAILS
			}
			//else: massage_type= SERVER_NO_OPPONENTS then show MAIN_MENU again!
		}
		
	}
	closesocket(m_socket);
	if (WSACleanup()) {//if wsacleanup failed
		printf("WSACleanup failed with error code: : %d\n", WSAGetLastError());
		return 1;
	}
	return;
}

/*
void game_routine(SOCKET m_socket) {
	int massage_type = receive_msg(m_socket);
	char SendStr[5];

	if (massage_type == SERVER_SETUP_REQUEST) {
		printf("Choose your 4 digits:\n");
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		SendRes = SendString(SendStr, m_socket);

		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
	}
}*/





//the function handle cases of: 1. unexpected disconnecting from the server 2. timeout 3.when server denied connection
//input: clientservice, port, ip address, server_denied_flag- if its 1 will handle server denied case, else handle disconnecting
void handle_connection_problems(SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address, int server_denied_flag) {
	int answer_num;
	int iResult_local = SOCKET_ERROR;
	if (server_denied_flag == 1) {
		printf("Server on %lu:%d denied the connection request.\n", serverIP_Address, serverport);
	}
	else 
		printf("Failed connecting to server on % lu: % d.\n", serverIP_Address, serverport);
	while (iResult_local == SOCKET_ERROR) {
		printf("Choose what to do next:\n 1. Try to reconnect\n2. Exit\n");
		scanf_s("%d", &answer_num);
		if (answer_num == 1) {
			iResult_local = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
		}
		else {
			WSACleanup();//TBD: check if all resources are free:
			iResult_local = closesocket(m_socket);
			if (iResult_local == SOCKET_ERROR)
				printf("closesocket function failed with error: %ld\n", WSAGetLastError());
			exit(1);//TBD: call to gracefull shutdown function for client
		}
	}
	return;
}
/*
void handle_server_deny(int iResult, SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address) {
	int answer_num;
	int iResult_local = iResult;
	while (iResult_local == SOCKET_ERROR) {
		printf("Server on %lu:%d denied the connection request.\nChoose what to do next:\n 1. Try to reconnect\n2. Exit\n", serverIP_Address, serverport);
		scanf_s("%d", &answer_num);
		if (answer_num == 1) {
			iResult_local = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
		}
		else {
			WSACleanup();//TBD: check if all resources are free:
			iResult_local = closesocket(m_socket);
			if (iResult_local == SOCKET_ERROR)
				printf("closesocket function failed with error: %ld\n", WSAGetLastError());
			exit(1);//TBD: call to shutdown function for client
		}
	}
	return;
}*/
/*
if answer_type == SERVER_DENIED printf :
Server on <ip> : <port> denied the connection request.
Choose what to do next :
	1. Try to reconnect
	2. Exit
	*/

