
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include "Massage.h"
#include "client.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"

SOCKET m_socket;

//TBD: I THINK THE CLIENT LOGIC ITS OK NOW, WE JUST HAVE TO HANDLE GRACEFULL DISCONNECTING :)

void ClientMain(char* username, int serverport, unsigned long serverIP_Address) {
	printf("welcom to ClientMain\n");
	SOCKADDR_IN clientService;
	int answer_num;
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	char Massage_type_str[MAX_MASSAGE_TYPE];
	char* AcceptedStr = NULL;
	char SendStr[SEND_STR_SIZE];
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
	char* SendStr = concatenate_str_for_msg(CLIENT_REQUEST_MSG, username);
	if ((SendString(SendStr, m_socket)) == TRNS_FAILED)// send client user name to the server
	{
		printf("error while sending username to server\n");
		return 0x555;
	}
	memset(SendStr, 0, sizeof(SendStr));// rest the SendStr
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
				game_routine(m_socket);
				massage_type = receive_msg(m_socket);
			}
			//else: massage_type= SERVER_NO_OPPONENTS then show MAIN_MENU again!
			//TBD: is the loop condition is OK for another game???
		}
	}
	closesocket(m_socket);//TBD: gracefull shutdown?
	if (WSACleanup()) {//if wsacleanup failed
		printf("WSACleanup failed with error code: : %d\n", WSAGetLastError());
		return 1;
	}
	return;
}


void game_routine(SOCKET m_socket) {
	int massage_type = receive_msg(m_socket);
	char client_Numbers[5];
	char client_Guess[5];
	int AcceptedStr, RecvRes;
	char Bulls[2], Cows[2];
	char opponent_guess[5], opponent_number[5];
	char opponent_username[MAX_USER_NAME];
	char winner_name[MAX_USER_NAME];
	TransferResult_t SendRes;
	if (massage_type == SERVER_SETUP_REQUEST) {
		printf("Choose your 4 digits:\n");
		gets_s(client_Numbers, sizeof(client_Numbers)); //Reading a string of the client chosen numbers from the keyboard
		char* SendStr = concatenate_str_for_msg(CLIENT_SETUP, client_Numbers);
		SendRes = SendString(SendStr, m_socket);// send : CLIENT_SETUP:1234
		if (SendRes == TRNS_FAILED){
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		massage_type = receive_msg(m_socket);
		while (massage_type == SERVER_PLAYER_MOVE_REQUEST) {
			printf("Choose your guess:\n");
			gets_s(client_Guess, sizeof(client_Guess)); //Reading a string of the client guess from the keyboard
			memset(SendStr, 0, sizeof(SendStr));// rest the SendStr
			char* SendStr = concatenate_str_for_msg(CLIENT_PLAYER_MOVE, client_Guess);
			SendRes = SendString(SendStr, m_socket);// send : CLIENT_SETUP:1234
			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				return 0x555;
			}
			RecvRes = ReceiveString(&AcceptedStr, m_socket);
			if (handle_return_value(RecvRes, &m_socket))
				return 1;//TBD: is it OK?
			extract_game_results(AcceptedStr, Bulls, Cows, opponent_username, opponent_guess);
			printf("Bulls: %s\n Cows: %s\n %s played: %s\n", Bulls, Cows, opponent_username, opponent_guess);
			RecvRes = ReceiveString(&AcceptedStr, m_socket);
			/*if (handle_return_value(RecvRes, m_socket))
			return 1;*/ //TBD: check RecvRes 
			massage_type = get_massage_type(AcceptedStr);
			//TBD: CONTINUE FROM HERE!!- POINT 11
		}
		//if we are here so there is a winner or a tie or opponent quit:
		if (massage_type == SERVER_OPPONENT_QUIT) {
			printf("Opponent quit\n");
		}
		if (massage_type == SERVER_WIN) {
			extract_winner_name_and_opponent_number(AcceptedStr, winner_name, opponent_number);
			printf("%s won!\n opponents numbers was %s\n", winner_name, opponent_number);
		}
		else{
			printf("It's a tie\n");
		}	
	}
}


void extract_winner_name_and_opponent_number(char* AcceptedStr, char* winner_name, char* opponent_number) {
	int i = 0, j = 0;
	while (AcceptedStr[i] != ':') {
		i++;
	}
	i++;
	while (AcceptedStr[i] != ';') {
		winner_name[j] = AcceptedStr[i];
		i++;
		j++;
	}
	winner_name[j] = '\0';
	i++;
	j = 0;
	while (AcceptedStr[i] != '\n') {
		opponent_number[j] = AcceptedStr[i];
		i++;
		j++;
	}
	opponent_number[j] = '\0';
}



//the function extracts the game result from the recieved string from the server
// input: recieved string from server, strings of each result from the server that will be fill by the function
void extract_game_results(char* AcceptedStr, char* Bulls, char* Cows,char* opponent_username, char* opponent_guess) {
	int i = 0;
	while (AcceptedStr[i] != ':') {
		i++;
	}
	i++;
	Bulls[0] = AcceptedStr[i];
	Bulls[1] = '\0';
	i = i + 2;
	Cows[0] = AcceptedStr[i];
	Cows[1] = '\0';
	i = i + 2;
	int j = 0;
	while (AcceptedStr[i] != ';') {
		opponent_username[j] = AcceptedStr[i];
		i++;
		j++;
	}
	opponent_username[j] = '\0';
	i++;
	j = 0;
	while (AcceptedStr[i] != '\n') {
		opponent_guess[j] = AcceptedStr[i];
		i++;
		j++;
	}
	opponent_guess[j] = '\0';
}





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

void gracefull_shutdown() {

}