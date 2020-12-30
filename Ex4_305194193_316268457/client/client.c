
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"

SOCKET m_socket;


//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;

	while (1)
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(SendStr, "quit"))
			return 0x555; //"quit" signals an exit from the client side

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
	HANDLE hThread[2];
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
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
	handle_diconnecting(iResult,clientService,serverport,serverIP_Address);
	printf("Connected to server on %lu:%d\n", serverIP_Address, serverport);
	//should send the client name to the server


	// Send and receive data.
	
	hThread[0] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)SendDataThread,
		NULL,
		0,
		NULL
	);

	hThread[1] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);

	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	TerminateThread(hThread[0], 0x555);//TBD: 0X555?
	TerminateThread(hThread[1], 0x555);
	CloseHandle(hThread[0]);
	CloseHandle(hThread[1]);
	closesocket(m_socket);
	WSACleanup();
	return;
}


//the function handle in cases of unexpected disconnecting from the server or timeout
void handle_diconnecting(int iResult, SOCKADDR_IN clientService, int serverport, unsigned long serverIP_Address) {
	int answer_num;
	while (iResult == SOCKET_ERROR) {
		printf("Failed connecting to server on %lu:%d.\nChoose what to do next:\n 1. Try to reconnect\n2. Exit\n", serverIP_Address, serverport);
		scanf_s("%d", &answer_num);
		if (answer_num == 1) {
			iResult = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
		}
		else {
			WSACleanup();//TBD: check if all resources are free:
			iResult = closesocket(m_socket);
			if (iResult == SOCKET_ERROR)
				wprintf("closesocket function failed with error: %ld\n", WSAGetLastError());
			exit(1);//TBD: is it OK to use exit(1) here?
		}
	}
}