#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "Massage.h"


#define NUM_OF_WORKER_THREADS 2
#define SERVER_ADDRESS_STR "192.168.0.1"
#define DEFAULT_BUFLEN 512
#define SEND_STR_SIZE 120


HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
int Threads_wanna_play[NUM_OF_WORKER_THREADS] = { 0 };

//functions from recitation: maybe we should need to create them.
//should be in the header file
static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET* t_socket);
//



void MainServer(int ServerPort) {

	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes,ListenRes,Ind;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		//TBD: Terminate program
	}
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //Create a socket

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		//TBD: Terminate program
	}

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		//TBD: terminate program
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(ServerPort);

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		//TBD: terminate program
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		//TBD: terminate program
	}

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;
	// Initialize all thread handles to NULL, to mark that they have not been initialized

	printf("Waiting for a client to connect...\n");
	
	while(1){
		//TBD: Change this for loop to while infinty loop - with stop condition of recive "exit" string
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			//TBD: terminate program
		}

		printf("Client Connected.\n");

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS){ //no slot is available
			printf("No slots available for client, dropping the connection.\n");
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.

		}
		else {
			ThreadInputs[Ind] = AcceptSocket; 
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL); 
		}
	}
}

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}
/// <summary>
/// Check if the transaction success. if not, print an error and return 1. else return 0
/// </summary>
/// <param name="tr"></param> enum contains the transaction status
/// <param name="t_socket"></param> the related socket of the transaction
/// <returns></returns> 1 if failed, 0 otherwise.

int check_transaction_return_value(TransferResult_t tr, SOCKET* t_socket) {
	if (tr == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}
	else if (tr == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}
	return 0;
}

void get_client_name(char* client_request_massage, char* destination_client_name) {
	int i = 0,j=0; 
	while (client_request_massage[i] != ':')
		i++;
	i++;
	while (client_request_massage[i] != '\n') {
		destination_client_name[j] = client_request_massage[i];
		i++;
		j++; //different indexes for the strings.
	}
	destination_client_name[j] = '\0';
}

static DWORD ServiceThread(SOCKET* t_socket) {

	char SendStr[SEND_STR_SIZE];
	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char* AcceptedStr = NULL;
	char Client_Name[MAX_USER_NAME];
	char Massage_type_str[MAX_MASSAGE_TYPE];

	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	int massage_type = get_massage_type(AcceptedStr);
	get_str_of_massage_type(massage_type, Massage_type_str);
	if (massage_type != CLIENT_REQUEST) {
		printf("The Massage should be CLIENT_REQUEST, recived %s", Massage_type_str);
		//TBD: deal with incorrect massages
	}
	get_client_name(AcceptedStr, Client_Name);
	free(AcceptedStr);
	strcpy_s(SendStr, SEND_STR_SIZE, "SERVER_MAIN_MENU\n");
	SendRes = SendString(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}
	//TBD: here we have to wait forever until user decide in client side
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	massage_type = get_massage_type(AcceptedStr);
	if (massage_type == CLIENT_DISCONNECT) {
		//TBD: close the thread, maybe do more? 
		closesocket(*t_socket);
		free(AcceptedStr);
		return 0;
	}
	else if (massage_type != CLIENT_VERSUS) { //this is the only correct massage here.
		printf("Wrong Massage by the client\n");
		return 0;
		//TBD: terminate program.. this is for debug purpose anyway
	}
	//Client versus massage recived:
	//










	
	


	
	//TBD: server approved massage


	//TBD: server main manu massage
	//TBD: server invite massage


}


/// <summary>
/// 
/// </summary>
/// <param name="numA"></param> The 4 digits number of player A, all digits must be different
/// <param name="numB"></param>The 4 digits number of player B, all digits must be different
/// <param name="A_guess_B"></param> the 4 digits player A guess that B has 
/// <param name="B_guess_A"></param> the 4 digits player B guess that A has 
void game_calculate_and_update_status(int numA, int numB, int A_guess_B, int B_guess_A) {

	int A_secret_digits[4] = { numA % 10,(numA / 10) % 10,(numA / 100) % 10,(numA / 1000) % 10 };
	int B_secret_digits[4] = { numB % 10,(numB / 10) % 10,(numB / 100) % 10,(numB / 1000) % 10 };
	int A_guess_B_digits[4] = { A_guess_B % 10,(A_guess_B / 10) % 10,(A_guess_B / 100) % 10,(A_guess_B / 1000) % 10 };
	int B_guess_A_digits[4] = { B_guess_A % 10,(B_guess_A / 10) % 10,(B_guess_A / 100) % 10,(B_guess_A / 1000) % 10 };
	int num_of_bull_A = 0, num_of_bull_B = 0, num_of_cows_A = 0, num_of_cows_B = 0;
	for (int i = 0; i < 4; i++) {

		num_of_bull_A += (A_secret_digits[i] == B_guess_A_digits[i]) ? 1 : 0;
		num_of_bull_B += (B_secret_digits[i] == A_guess_B_digits[i]) ? 1 : 0;

		for (int j = 0; j < 4; j++) {
			if ((A_secret_digits[i] == B_guess_A_digits[j]) && (i != j))
				num_of_cows_A += 1;
			if ((B_secret_digits[i] == A_guess_B_digits[j]) && (i != j))
				num_of_cows_B += 1;
		}
	}
	
	printf("A number is %d, B number is %d\n",numA,numB);
	printf("A geuss %d, B guess %d\n", A_guess_B, B_guess_A);
	printf("A: bulls %d , cows %d\n",num_of_bull_A,num_of_cows_A);
	printf("B: bulls %d, cows %d\n",num_of_bull_B,num_of_cows_B);
}
