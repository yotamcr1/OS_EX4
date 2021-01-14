#include "server.h"

//Global parameters
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS]; 
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
HANDLE Thread_Connection_File[NUM_OF_WORKER_THREADS];
lock* p_lock = NULL;
int number_of_connected_clients = 0;
HANDLE semaphore_gun = NULL;
int reader_count = 0;

void write_to_file(HANDLE file,const char* str) {

	DWORD dwBytesWrite = 0;
	//write_lock(p_lock);
	//TBD: write operation
	if (!WriteFile(file, str, strlen(str), &dwBytesWrite, NULL)) {
		printf("error while trying to write into the output file. exit\n");
		//TBD: close everything
	}
	//write_release(p_lock);
}

void read_from_file(HANDLE file, char* str) {
	//read_lock(p_lock);
	DWORD dwBytesRead = 0;
	//TBD: read opreation
	if (!ReadFile(file, str, 45, &dwBytesRead, NULL))
		//printf("%s", GetLastError());
		//read_release(p_lock);
		printf("Error occoured within read_from_file function\n");
}


void initialize_semaphore() {
	semaphore_gun = CreateSemaphoreA(NULL, 0, 1, NULL);  
	if (NULL == semaphore_gun){
		printf("Error within initialize sempaphore functionn\n");
		return 0; //Errror indicates
	}
	return 1; 
}
/*void initialize_mutex() {

	fisrtMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (fisrtMutex == NULL)
	{
		printf("CreateMutex error: %d\n, exit\n", GetLastError());
		exit (1);
	}
	secondMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	if (secondMutex == NULL)
	{
		CloseHandle(fisrtMutex);
		printf("CreateMutex error: %d\n, exit\n", GetLastError());
		exit(1);
	}
}*/

void MainServer(int ServerPort) {

	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes,ListenRes,Ind;
	//initialize_mutex(); //initialized global firstMutex, secondMutex 
	p_lock = InitializLock(NUM_OF_WORKER_THREADS);
	initialize_semaphore();
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


DWORD get_file_orig_size(HANDLE file) {
	DWORD size = GetFileSize(
		file, NULL
	);
	if (size == INVALID_FILE_SIZE) {
		printf("can't calculate origianl size file. exit\n");
		return -1;
	}
	return size;
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

int send_massage(char* str,SOCKET* t_socket) {

	char SendStr[SEND_STR_SIZE];
	TransferResult_t SendRes;
	strcpy_s(SendStr, SEND_STR_SIZE, str);
	SendRes = SendString(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return 0; //Error Occured
	}
	return 1; //Success
}

static DWORD ServiceThread(SOCKET* t_socket) {

	char SendStr[SEND_STR_SIZE];
	DWORD wait_code;
	BOOL Done = FALSE;
	TransferResult_t SendRes, RecvRes;
	char* AcceptedStr = NULL;
	char Client_Name[MAX_USER_NAME];
	char Oponent_Client_Name[MAX_USER_NAME];
	char Massage_type_str[MAX_MASSAGE_TYPE];
	DWORD last_error;
	int am_i_first = 0; //Only the first Client will have am_i_first = 1

	number_of_connected_clients++; //add current client to the counter
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	printf("Server Recived Massage:\n");
	printf("%s\n", AcceptedStr);
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	int massage_type = get_massage_type(AcceptedStr);
	get_str_of_massage_type(massage_type, Massage_type_str);
	if (massage_type != CLIENT_REQUEST) {
		printf("The Massage should be CLIENT_REQUEST, recived %s", Massage_type_str);
		//TBD: deal with incorrect massages
		//this is coding falut!!
	}
	get_client_name(AcceptedStr, Client_Name);
	free(AcceptedStr);
	AcceptedStr = NULL;
	if (number_of_connected_clients > 2) { //Server is Full
		if (!send_massage(SERVER_DENIED_MSG, t_socket))
			return 1;
		printf("Server Send Massage: SERVER_DENIED\n");
		free(AcceptedStr);
		closesocket(*t_socket);
		return 0;
	}
	if (!send_massage(SERVER_APPROVED_MSG, t_socket)) {
		//TBD: ERROR OCCUR
		return 1;
	}
	printf("Server Send Massage: SERVER_APPROVED\n");

server_main_menu:
	if (!send_massage(SERVER_MAIN_MENU_MSG, t_socket)) {
		//TBD: ERROR OCCUR
		return 1;
	}
	printf("Server Send Massage:SERVER_MAIN_MENU\n");
	//TBD: here we have to wait forever until user decide in client side
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	printf("Server Recived Massage:\n");
	printf("%s\n", AcceptedStr);
	massage_type = get_massage_type(AcceptedStr);
	if (massage_type == CLIENT_DISCONNECT) {
		//TBD: close the thread, maybe do more? 
		number_of_connected_clients--;
		closesocket(*t_socket);
		free(AcceptedStr);
		return 0;
	}
	else if (massage_type != CLIENT_VERSUS) { //this is the only correct massage here.
		printf("Wrong Massage by the client\n");
		return 0;
		//TBD: terminate program.. this is for debug purpose anyway
	}

	/*Client Versus Massage type:*/
	/* critical code section */

	int writing_return_val = write_client_name_to_game_file(&am_i_first, Client_Name, strlen(Client_Name));
	if (writing_return_val == SERVER_NO_OPPONENTS) {
		if (!send_massage(SERVER_NO_OPPONENTS_MSG, t_socket)) {
			//TBD: ERROR OCCUR
			return 1;
		}
		if (!send_massage(SERVER_MAIN_MENU_MSG, t_socket)) {
			//TBD: ERROR OCCUR
			return 1;
			//TBD:go back to main manu
		}
	}
	/*Here we should have 2 connected opponets,
	and am_i_first is equal to 1 only for the first client. this order should be saved
	*/
	read_file_get_opponent_user_name(am_i_first, Oponent_Client_Name, strlen(Client_Name));
	concatenate_str_for_msg(SERVER_INVITE_MSG, Oponent_Client_Name, SendStr);
	if (!send_massage(SendStr, t_socket)) {
		//TBD: ERROR OCCUR
		return 1;
	}
	printf("Server sending SERVER_INVITE_MSG massage:\n");
	printf("%s", SendStr);
	if (!send_massage(SERVER_SETUP_REQUEST_MSG, t_socket)) {
		//TBD: ERROR OCCUR
		return 1;
	}
	free(AcceptedStr);
	AcceptedStr = NULL;
	printf("Server sending SERVER_SETUP_REQUEST_MSG massage:\n");
	//here server should recieve client setup massage!
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	printf("Server Recived Massage:\n");
	printf("%s\n", AcceptedStr);
	int secret_number = get_4digit_number_from_massage(AcceptedStr);
	free(AcceptedStr);
	AcceptedStr = NULL;
	//TBD: HERE WE HAVE TO WAIT FOR BOTH CLIENT SENDS THEIR NUMBER
	//TBD: HERE WE HAVE TO WRITE IT TO THE FILE
	if (!send_massage(SERVER_PLAYER_MOVE_REQUEST_MSG, t_socket)) {
		//TBD: ERROR OCCUR
		return 1;
	}
	printf("Server sending SERVER_PLAYER_MOVE_REQUEST_MSG massage:\n");
	free(AcceptedStr);
	AcceptedStr = NULL;
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket))
		return 1;
	printf("Server Recived Massage:\n");
	printf("%s\n", AcceptedStr);
	massage_type = get_massage_type(AcceptedStr);
	get_str_of_massage_type(massage_type, Massage_type_str);
	if (massage_type != CLIENT_PLAYER_MOVE) {
		printf("The Massage should be CLIENT_SETUP, recived %s", Massage_type_str);
		//TBD: deal with incorrect massages
		//this is coding falut!!
	}
	int guess = get_4digit_number_from_massage(AcceptedStr);
	//have to write the 4 digit in the file???
	/*
		//TBD: call to function that calculate the result massage
		//Here we send the secret number and the guess of the first client
		char string[MAX_WRITE_BYTES_TO_GAME_FILE];
		char temp_num[5];
		strcpy_s(string, _countof(string), "Client1 secret number:");
		//itoa(secret_number,temp_num,
		strcat_s(string, _countof(string), temp_num);
		strcat_s(string, _countof(string), "and ");
		strcat_s(string, _countof(string), "strcat_s!");
	}*/
	}


int write_client_name_to_game_file(int* am_i_first,char* Client_Name, int client_name_length) {
	//am_i_first will be 1 only for the first client connected to the server.
	/*  CRITICAL Section: */
	write_lock(p_lock);
	HANDLE Thread_connection_file = CreateFileA(
		"GameSession.txt",
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file withing write_client_name_to_game_file function\n");
		//TBD: DEAL WITH IT
	}
	DWORD ret_val = get_file_orig_size(Thread_connection_file);
	if (-1 == ret_val) {
		//TBD: error occur, should close everything and die
		printf("error within write_client_name_to_game_file function\n");
	}

	if (ret_val == 0) { //This is the first player! 
		*am_i_first = 1; //Indicates to the thread that it is first
		write_to_file(Thread_connection_file, Client_Name);
	}
	else { //This is the second thread!
		DWORD dwPtrLow = SetFilePointer(Thread_connection_file,	NULL,	NULL, FILE_END);
		write_to_file(Thread_connection_file, Client_Name);
	}
	if (!CloseHandle(Thread_connection_file))
		printf("Error while close file handle within write_client_name_to_game_file function\n"); //No need to close everything here - the game should continue
	write_release(p_lock);

	if (*am_i_first == 1) {
		//Wait until second thread will write his name to the file! 
		//using semaphore gun for this.
		printf("First Client,name: %s, waiting for second client", Client_Name);
		DWORD wait_for_semaphore = WaitForSingleObject(semaphore_gun, TIMEOUT);
		if (wait_for_semaphore == WAIT_TIMEOUT)
		{
			printf("Timeout has been reached within write_client_name_to_game_file function for the semaphore\n");
			return SERVER_NO_OPPONENTS; //TBD: The Caller check it and send this massage to the client! 
		}
		else if (wait_for_semaphore == WAIT_OBJECT_0) {
			printf("First Client free! second thread realese me!\n");
		}
	}
	else { //This is the second Client
		if (!ReleaseSemaphore(semaphore_gun, 1, NULL)) {
			printf("Error: can't release the semphore gun within write_client_name_to_game_file function\n");
			//TBD: close program..
		}
		printf("Second Thread released the semphore\n");
	}
	return 0;
}

void read_file_get_opponent_user_name(int am_i_first, char* Oponent_Client_Name, int my_user_name_size) {
	reader_count++;
	HANDLE Thread_connection_file = CreateFileA(
		"GameSession.txt",
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	int first_byte_position, last_byte_position, len;
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file read_file_get_opponent_user_namee function\n");
		//TBD: deal with error
	}
	DWORD size_of_file = get_file_orig_size(Thread_connection_file);
	if (am_i_first == 1) {
		first_byte_position = my_user_name_size;
		last_byte_position = size_of_file;
		printf("read_file_get_opponent_user_name function:\n First Client! first_byte_position - %d, last_byte_position - %d\n", first_byte_position, last_byte_position);
	}
	else {
		first_byte_position = 0;
		last_byte_position = size_of_file - my_user_name_size;
		printf("read_file_get_opponent_user_name function:\n Second Client! first_byte_position - %d, last_byte_position - %d\n", first_byte_position, last_byte_position);
	}

	char file_str_name_contents[SEND_STR_SIZE];

	read_from_file(Thread_connection_file, file_str_name_contents); 
	int j = 0;
	for (int i = 0 ; i < (last_byte_position - first_byte_position); i++) {
		Oponent_Client_Name[j] = file_str_name_contents[i];
		j++;
	}
	Oponent_Client_Name[j] = '\0';
	
	if (!CloseHandle(Thread_connection_file)) {
		printf("Can't close the thread within read_file_get_opponent_user_name function\n");
		//TBD: ...
	}
	if (reader_count==2){
		if (DeleteFile("GameSession.txt") == 0) {
			printf("didnt delete file within read_file_get_opponent_name\n");
		}
	}
}

int get_4digit_number_from_massage(char* str) {
	int i = 0;
	while (str[i] != ':') //this is the delimeter. after the char ':', the next 4 chars will be the digits. 
		i++;
	i++;
	char str_num[5] = { str[i], str[i + 1],str[i + 2],str[i + 3], '\0' };
	int number = atoi(str_num);
	return number;
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
