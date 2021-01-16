

#include "Massage.h"
#include "client.h"
#include "SocketSendRecvTools.h"

SOCKET m_socket;

void ClientMain(char* username, int serverport, char* serverIP_Address_str) {
	printf("welcom to ClientMain \n");
	SOCKADDR_IN clientService;
	unsigned long serverIP_Address;
	serverIP_Address = inet_addr(serverIP_Address_str);
	int answer_num;
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	char SendStr[SEND_STR_SIZE];
	char Massage_type_str[MAX_MASSAGE_TYPE];
	char* AcceptedStr = NULL;
	int massage_type;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("WSAStartup function failed with error : % d\n", iResult);
		return 1;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {// Check for errors to ensure that the socket is a valid socket.
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	 //Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;//AF_INET is the Internet address family
	clientService.sin_addr.s_addr = serverIP_Address; //Setting the IP address to connect to
	clientService.sin_port = htons(serverport); //Setting the port to connect to.
	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	iResult = connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService));
	if (iResult == SOCKET_ERROR) 
		handle_connection_problems(clientService, serverport, serverIP_Address,0);
	printf("Connected to server on %s:%d\n", serverIP_Address_str, serverport);//connected successfully
	concatenate_str_for_msg(CLIENT_REQUEST_MSG, username,SendStr);
	printf("Client sending Client_Request massage:\n");
	printf("%s",SendStr);
	if ((SendString(SendStr, m_socket)) == TRNS_FAILED)// send client user name to the server
	{
		printf("error while sending username to server\n");
		return 0x555;
	}
	memset(SendStr, 0, sizeof(SendStr));// rest the SendStr
	/*TBD: use setsocket for getting answer from the server
	if there is no answer from server: handle_connection_problems(clientService,serverport,serverIP_Address,0);
	*/
	set_socket_timeout(SERVER_TIMEOUT, m_socket);
	AcceptedStr = receive_msg(m_socket, AcceptedStr, &massage_type);
	if (WSAGetLastError() == WSAETIMEDOUT) {
		handle_connection_problems(clientService, serverport, serverIP_Address, 0);
	}
	printf("Client Recived Massage:\n");
	printf("%s", AcceptedStr);
	if (massage_type == SERVER_DENIED) {
		handle_connection_problems(clientService, serverport, serverIP_Address,1);
	}
	free(AcceptedStr);
	AcceptedStr = NULL;
	if (massage_type == SERVER_APPROVED) {
		AcceptedStr = receive_msg(m_socket, AcceptedStr,&massage_type);
		printf("Client Recived Massage:\n");
		printf("%s", AcceptedStr);
		while ((massage_type == SERVER_MAIN_MENU)||(massage_type==SERVER_NO_OPPONENTS)) {
			printf("Choose what to do next:\n1. Play against another client\n2. Quit\n");
			scanf_s("%d", &answer_num);
			if (answer_num == 1) {
				if ((SendString(CLIENT_VERSUS_MSG, m_socket)) == TRNS_FAILED){
					printf("error while sending CLIENT_VERSUS_MSG to server\n");
					return 0x555;
				}
				printf("Client sending CLIENT_VERSUS massage\n");
			}
			else {//send to server client disconnect and exit
				if ((SendString(CLIENT_DISCONNECT_MSG, m_socket)) == TRNS_FAILED)
				{
					printf("error while sending CLIENT_DISCONNECT to server\n");
					return 0x555;
				}
				gracefull_client_shutdown(m_socket, AcceptedStr);
			}
			free(AcceptedStr);
			AcceptedStr = NULL;
			//TBD: have to wait 30 seconds for answer- check it:
		    set_socket_timeout(CLIENT_TIMEOUT, m_socket);
			AcceptedStr = receive_msg(m_socket, AcceptedStr, &massage_type);
			printf("Client Recived Massage:\n");
			printf("%s", AcceptedStr);
			free(AcceptedStr);
			AcceptedStr = NULL;
			set_socket_timeout(BLOCKING_TIMEOUT, m_socket);
			if (massage_type == SERVER_INVITE) {
				printf("Game is on!\n");
				game_routine(m_socket);
				AcceptedStr = receive_msg(m_socket, AcceptedStr, &massage_type);
				if (AcceptedStr != NULL) {
					free(AcceptedStr);
					AcceptedStr = NULL;
				}
			}
			//else: massage_type= SERVER_NO_OPPONENTS then show MAIN_MENU again!
		}
	}
	gracefull_client_shutdown(m_socket, AcceptedStr);
	return;
}


void game_routine(SOCKET m_socket) {
	char client_Numbers[5];
	char client_Guess[5];
	int RecvRes;
	char SendStr[SEND_STR_SIZE];
	char* AcceptedStr = NULL;
	char Bulls[2], Cows[2];
	char opponent_guess[5], opponent_number[5];
	char opponent_username[MAX_USER_NAME];
	char winner_name[MAX_USER_NAME];
	int massage_type;
	AcceptedStr = receive_msg(m_socket, AcceptedStr, &massage_type);
	TransferResult_t SendRes;
	printf("Client Recived Massage within game_routine:\n");
	printf("%s\n", AcceptedStr);
	if (massage_type == SERVER_SETUP_REQUEST) {
		printf("Choose your 4 digits:\n");
		if (!scanf_s("%4s", client_Numbers, (unsigned)_countof(client_Numbers))) {//Reading a string of the client chosen numbers from the keyboard
			printf("Error while scaning number from keyboard. exit\n");
			//TBD: deal with errors- i think we dont have to 
		}
		concatenate_str_for_msg(CLIENT_SETUP_MSG, client_Numbers,SendStr);
		SendRes = SendString(SendStr, m_socket);// send : CLIENT_SETUP:1234
		if (SendRes == TRNS_FAILED){
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		printf("Client Send Massage within game_routine:\n");
		printf("%s\n", SendStr);
		free(AcceptedStr);
		AcceptedStr = NULL;
		AcceptedStr = receive_msg(m_socket, AcceptedStr, &massage_type);
		printf("Client Recived Massage within game_routine:\n");
		printf("%s\n", AcceptedStr);
		while (massage_type == SERVER_PLAYER_MOVE_REQUEST) {
			printf("Choose your guess:\n");
		//	gets_s(client_Guess, sizeof(client_Guess)); //Reading a string of the client guess from the keyboard
			if (!scanf_s("%4s", client_Guess, (unsigned)_countof(client_Guess))) {
				printf("Error while scaning number from keyboard. exit\n");
				//TBD: deal with errors
			}
			memset(SendStr, 0, sizeof(SendStr));// rest the SendStr
			concatenate_str_for_msg(CLIENT_PLAYER_MOVE_MSG, client_Guess,SendStr);
			SendRes = SendString(SendStr, m_socket);// send : CLIENT_SETUP:1234
			printf("Client sending CLIENT_PLAYER_MOVE Massage:\n");
			printf("%s", SendStr);
			if (SendRes == TRNS_FAILED)
			{
				printf("Socket error while trying to write data to socket\n");
				return 0x555;
			}
			free(AcceptedStr);
			AcceptedStr = NULL;
			//Chen: Here we got stuck
			RecvRes = ReceiveString(&AcceptedStr, m_socket);
			if (check_transaction_return_value(RecvRes, &m_socket))
				return 1;//TBD: is it OK
			//here we get SERVER_GAME_RESULT FROM SERVER
			massage_type = get_massage_type(AcceptedStr);
			if (massage_type == SERVER_GAME_RESULTS) {
				extract_game_results(AcceptedStr, Bulls, Cows, opponent_username, opponent_guess);
				printf("Bulls: %s\n Cows: %s\n %s played: %s\n", Bulls, Cows, opponent_username, opponent_guess);
			}
			printf("Client Recived Massage within game_routine:\n");
			printf("%s\n", AcceptedStr);
			free(AcceptedStr);
			AcceptedStr = NULL; 
			RecvRes = ReceiveString(&AcceptedStr, m_socket);
			printf("Client Recived Massage within game_routine:\n");
			printf("%s\n", AcceptedStr);
			if (check_transaction_return_value(RecvRes, &m_socket))
				return 1; 
			massage_type = get_massage_type(AcceptedStr);
		}
		//if we are here so there is a winner or a tie or opponent quit:
		if (massage_type == SERVER_OPPONENT_QUIT) {
			printf("Opponent quit\n");
		}
		if (massage_type == SERVER_WIN) {
			extract_winner_name_and_opponent_number(AcceptedStr, winner_name, opponent_number);
			printf("%s won!\n opponents numbers was %s\n", winner_name, opponent_number);
		}
		if (massage_type == SERVER_DRAW){
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
		printf("Failed connecting to server on % lu: %d.\n", serverIP_Address, serverport);
	while (iResult_local == SOCKET_ERROR) {
		printf("Choose what to do next:\n1. Try to reconnect\n2. Exit\n");
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

//for every shutdown we have to gracefully shutdown the client:
//In a graceful shutdown, any data that has been queued, but not yet transmitted can be sent prior to the connection being closed
int gracefull_client_shutdown(SOCKET m_socket,char* AcceptedStr) {
	int RecvRes, iResult;
	shutdown(m_socket,SD_SEND); //signal end of session and that client has no more data to send
	RecvRes = ReceiveString(&AcceptedStr, m_socket);
	if (check_transaction_return_value(RecvRes, &m_socket))
		printf("continue shutdown\n");
	iResult = closesocket(m_socket);
	if (iResult == SOCKET_ERROR)
		printf("closesocket function failed with error: %ld\n", WSAGetLastError());
	if (WSACleanup()) {//if wsacleanup failed
		printf("WSACleanup failed with error code: %d\n", WSAGetLastError());
		}
	if (AcceptedStr != NULL) {
		free(AcceptedStr);
	}
	exit(0);
}



