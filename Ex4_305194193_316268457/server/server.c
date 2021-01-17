#include "server.h"
//Authers: Chen Katz And Yotam Carmi

//Global parameters
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS]; 
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
SOCKET MainSocket = INVALID_SOCKET;
HANDLE Listening_to_cmd_thread;
int number_of_connected_clients;
lock* file_lock = NULL, *sync_function_lock = NULL,*geuss_lock = NULL,*game_result_lock = NULL;
HANDLE semaphore_gun = NULL,semaphore_mag = NULL,sempaphore_guess = NULL;
int reader_count = 0, first_client_secret_number = 0, second_client_secret_number = 0,arriavl=0;

void write_to_file(HANDLE file,const char* str) {

	DWORD dwBytesWrite = 0;
	if (!WriteFile(file, str, strlen(str), &dwBytesWrite, NULL)) {
		printf("write_to_file: error while trying to write into the output file. exit\n");
		//TBD: close everything
		gracefull_server_shutdown(NULL);
		
	}
}

void read_from_file(HANDLE file, char* str) {
	DWORD dwBytesRead = 0;
	if (!ReadFile(file, str, 45, &dwBytesRead, NULL)) {
		printf("read_from_file: Error occoured within read_from_file function\n");
		//TBD: close eveything
		gracefull_server_shutdown(NULL);
	}
}

void kill_locks() {
	DestroyLock(file_lock);
	DestroyLock(sync_function_lock);
	DestroyLock(geuss_lock);
	DestroyLock(game_result_lock);
}
void kill_semaphores() {
	CloseHandle(semaphore_gun);
	CloseHandle(semaphore_mag);
	CloseHandle(sempaphore_guess);
}
void initialize_semaphore() {
	semaphore_gun = CreateSemaphoreA(NULL, 0, 1, NULL);  
	if (NULL == semaphore_gun){
		printf("Error within initialize sempaphore functionn\n");
		kill_locks();
		exit(1);
	}
	semaphore_mag = CreateSemaphoreA(NULL, 0, 1, NULL);
	if (NULL == semaphore_mag) {
		printf("Error within initialize sempaphore functionn\n");
		kill_locks();
		exit(1);
	}
	sempaphore_guess = CreateSemaphoreA(NULL, 0, 1, NULL);
	if (NULL == sempaphore_guess) {
		printf("Error within initialize sempaphore functionn\n");
		kill_locks();
		exit(1);
	}
}

static DWORD listening_to_cmd_keystroke()
{
	char exit_st[LENGTH_EXIT] = { 0 };
	while (TRUE) {
		if (_kbhit() != 0) { //The _kbhit function checks the console for a recent keystroke. If the function returns a nonzero value, a keystroke is waiting in the buffer. //
			scanf_s("%s", exit_st, LENGTH_EXIT + 1);
			if (strcmp(exit_st,"exit")==0)
			{
				printf("recived exit string by the user\ntrying to close everything and finish run\n");
				break;
			}
		}
		else
			Sleep(TIME_FOR_POLLING);
	}
	gracefull_server_shutdown(NULL);
	return 0;
}

void MainServer(int ServerPort) {

	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes,ListenRes,Ind;
	file_lock = InitializLock(NUM_OF_WORKER_THREADS);
	sync_function_lock = InitializLock(NUM_OF_WORKER_THREADS);
	geuss_lock = InitializLock(NUM_OF_WORKER_THREADS);
	game_result_lock = InitializLock(NUM_OF_WORKER_THREADS);
	initialize_semaphore();
	// Initialize Winsock.
	Listening_to_cmd_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)listening_to_cmd_keystroke, NULL, 0, NULL);
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR){
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		kill_locks();
		kill_semaphores();
		exit(1);
	}
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //Create a socket
	if (MainSocket == INVALID_SOCKET){
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		kill_locks();
		kill_semaphores();
		closesocket(MainSocket);
		exit(1);
	}
	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE){
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",SERVER_ADDRESS_STR);
		kill_locks();
		kill_semaphores();
		closesocket(MainSocket);
		exit(1);
	}
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(ServerPort);
	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR){
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		kill_locks();
		kill_semaphores();
		closesocket(MainSocket);
		exit(1);
	}
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR){
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		kill_locks();
		kill_semaphores();
		closesocket(MainSocket);
		exit(1);
	}
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL; 	// Initialize all thread handles to NULL, to mark that they have not been initialized
	
	printf("Waiting for a client to connect...\n");
	while(1){
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			printf("Going back to AcceptSocket in order to reviced new connection.\nif you want to end the program, please write exit\n");
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
		printf("can't calculate origianl size file.\n");
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

void write_geuss_number_to_game_file(int am_i_first, int my_geuss) {
	//am_i_first will be 1 only for the first client connected to the server.
	char buffer[10];
	_itoa_s(my_geuss, buffer,5, 10);
	/*  CRITICAL Section: */
	write_lock(geuss_lock);
	int my_arrival = 0; //1 if I was here first, 2 otherwise
	HANDLE Thread_connection_file = CreateFileA(
		FILE_NAME,
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file withing write_client_name_to_game_file function\n");
		gracefull_server_shutdown(NULL);
	}
	DWORD ret_val = get_file_orig_size(Thread_connection_file);
	if (-1 == ret_val) {
		printf("error within write_client_name_to_game_file function\n");
		gracefull_server_shutdown(NULL);
	}
	/*Four optionons here:
	1. am_i_first = 0 and ret_val = 0
	the second client got here first
	2. am_i_first = 0 and ret_val !=0
	second client got here second
	3. am_i_first = 1 and ret_val = 0
	first client got here first
	4. am_i_first = 1 and ret_val !=0
	first client get here second
	*/
	if ((am_i_first == 0) && (ret_val == 0)) {
		printf("second player got first within write_geuss_number_to_game_file function\n");
		write_to_file(Thread_connection_file, buffer);
		DWORD dwPtrLow = SetFilePointer(Thread_connection_file, NULL, NULL, FILE_END);
		write_to_file(Thread_connection_file, buffer);	//we write the geuss twice, and the first 4 digits should be overide! 
		my_arrival = 1;
	}
	else if (am_i_first == 0 && ret_val != 0) {
		printf("second player got second within write_geuss_number_to_game_file function\n");
		DWORD dwPtrLow = SetFilePointer(Thread_connection_file, NULL, NULL, FILE_END);
		write_to_file(Thread_connection_file, buffer);
		my_arrival = 2;
	}
	else if (am_i_first == 1 && ret_val == 0) {
		printf("first player got first within write_geuss_number_to_game_file function\n");
		write_to_file(Thread_connection_file, buffer);
		my_arrival = 1;
	}
	else if (am_i_first == 1 && ret_val != 0) {
		printf("first player got second within write_geuss_number_to_game_file function\n");
		char temp_buffer[45];
		DWORD dwPtrLow = SetFilePointer(Thread_connection_file, NULL, NULL, FILE_BEGIN);
		read_from_file(Thread_connection_file, temp_buffer);
		dwPtrLow = SetFilePointer(Thread_connection_file, NULL, NULL, FILE_BEGIN);
		for (int i = 0; i < 4; i++) //Buffer contains first player geuss, temp_buffer contains twice the other player geuss. 
			buffer[4 + i] = temp_buffer[i];
		buffer[8] = '\0';
		write_to_file(Thread_connection_file, buffer);
		my_arrival = 2;
	}
	if (!CloseHandle(Thread_connection_file))
		printf("Error while close file handle within write_client_name_to_game_file function\n"); //No need to close everything here - the game should continue
	/*End of Critical Section*/
	write_release(geuss_lock);
	
	if (my_arrival == 1) {
		//Wait until second thread will write his name to the file! 
		printf("First Arrival waiting for second to release him\n");
		DWORD wait_for_semaphore = WaitForSingleObject(sempaphore_guess, TIMEOUT);
		if (wait_for_semaphore == WAIT_TIMEOUT)
		{
			printf("Timeout has been reached function for the semaphore\n");
		}
		else if (wait_for_semaphore == WAIT_OBJECT_0) {
			printf("First Arrivel released by second!\n");
		}
	}
	else { 
		if (!ReleaseSemaphore(sempaphore_guess, 1, NULL)) {
			printf("Error: can't release the semphore gun within write_client_name_to_game_file function\n");
			//TBD: close program..
		}
		printf("Second Arrival released the semphore\n");
	}
}

void sync_function() {
	write_lock(sync_function_lock);
	arriavl++;
	if (arriavl == 1) {
		write_release(sync_function_lock);
		DWORD wait_for_semaphore = WaitForSingleObject(semaphore_mag, TIMEOUT);
		if (wait_for_semaphore == WAIT_TIMEOUT)
		{
			printf("Timeout has been reached within sync function for the semaphore\n");
		}
		else if (wait_for_semaphore == WAIT_OBJECT_0) {
			printf("First Client free! second thread realese me!\n");
		}
	}
	else {
		write_release(sync_function_lock);
		if (!ReleaseSemaphore(semaphore_mag, 1, NULL)) {
			printf("Error: can't release the semphore gun within write_client_name_to_game_file function\n");
			//TBD: close program..
		}
		printf("Second Thread released the semphore\n");
	}
}

void format_win_result(char* Client_Name, int other_secret_number, char* temp_buffer) {
	char temp_num[7];
	memset(temp_buffer, 0, sizeof(temp_buffer));// rest the SendStr
	strcpy_s(temp_buffer, MAX_USER_NAME, Client_Name);
	strcat_s(temp_buffer, MAX_USER_NAME + 15, ";");
	_itoa_s(other_secret_number, temp_num, 5, 10);
	strcat_s(temp_buffer, MAX_USER_NAME + 30, temp_num);
	strcat_s(temp_buffer, 100, "\n");
}



void free_service_thread_memories(SOCKET* t_socket,char** AcceptedStr) {
	closesocket(*t_socket);
	check_if_str_is_allocated(AcceptedStr);
	number_of_connected_clients--;
	//pthread_cancel(thread1);
}

static DWORD ServiceThread(SOCKET* t_socket) {
	char SendStr[SEND_STR_SIZE];
	TransferResult_t RecvRes;
	char* AcceptedStr = NULL;
	char Client_Name[MAX_USER_NAME], Oponent_Client_Name[MAX_USER_NAME], Massage_type_str[MAX_MASSAGE_TYPE];
	int my_cows, my_bulls, oponent_cows, oponent_bulls, my_secret_number, other_secret_number, my_geuss, other_client_geuss, flag = 1, am_i_first = 0;

	number_of_connected_clients++; //add current client to the counter
	set_socket_timeout(SERVER_TIMEOUT, *t_socket);
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	printf("Server Recived Massage:\n%s\n", AcceptedStr);
	if (check_transaction_return_value(RecvRes, t_socket)) {
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	int massage_type = get_massage_type(AcceptedStr);
	get_str_of_massage_type(massage_type, Massage_type_str);
	get_client_name(AcceptedStr, Client_Name);
	check_if_str_is_allocated(&AcceptedStr);
	if (number_of_connected_clients > 2) { //Server is Full
		if (!send_massage(SERVER_DENIED_MSG, t_socket)) {
			printf("error during send massage. exit thread\n");
			free_service_thread_memories(t_socket, &AcceptedStr);
			return 1;
		}
		printf("Server Send Massage: SERVER_DENIED\n");
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	if (!send_massage(SERVER_APPROVED_MSG, t_socket)) {
		free_service_thread_memories(t_socket, &AcceptedStr);
		printf("error during send massage. exit thread\n");
		return 1;
	}
	printf("Server Send Massage: SERVER_APPROVED\n");
server_main_menu:
	flag = 1;
	set_socket_timeout(BLOCKING_TIMEOUT, *t_socket);
	if (!send_massage(SERVER_MAIN_MENU_MSG, t_socket)){
		free_service_thread_memories(t_socket, &AcceptedStr);
		printf("error during send massage. exit thread\n");
		return 1;
	}
	printf("Server Send Massage:SERVER_MAIN_MENU\n");
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket)){
		free_service_thread_memories(t_socket, &AcceptedStr);
		printf("error during send massage. exit thread\n");
		return 1;
	}
	printf("Server Recived Massage:\n%s\n", AcceptedStr);
	massage_type = get_massage_type(AcceptedStr);
	if (massage_type == CLIENT_DISCONNECT) {
		number_of_connected_clients--;
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	if (!(number_of_connected_clients == 2)) {
		if (!send_massage(SERVER_NO_OPPONENTS_MSG, t_socket)){
			free_service_thread_memories(t_socket, &AcceptedStr);
			printf("error during send massage. exit thread\n");
			goto server_main_menu;
		}
	}
	/*Client Versus Massage type:*/
	int writing_return_val = write_client_name_to_game_file(&am_i_first, Client_Name, strlen(Client_Name));
	if (writing_return_val == SERVER_NO_OPPONENTS) {
		if (!send_massage(SERVER_NO_OPPONENTS_MSG, t_socket)){
			free_service_thread_memories(t_socket, &AcceptedStr);
			printf("error during send massage. exit thread\n");
			return 1;
		}
		if (!send_massage(SERVER_MAIN_MENU_MSG, t_socket)){
			printf("error during send massage. exit thread\n");
			free_service_thread_memories(t_socket, &AcceptedStr);
			return 1;
		}
		goto server_main_menu;
	}
	read_file_get_opponent_user_name(am_i_first, Oponent_Client_Name, strlen(Client_Name));
	concatenate_str_for_msg(SERVER_INVITE_MSG, Oponent_Client_Name, SendStr);
	if (!send_massage(SendStr, t_socket)){
		printf("error during send massage. exit thread\n");
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	printf("Server Recived Massage:\n%s\n", AcceptedStr);
	if (!send_massage(SERVER_SETUP_REQUEST_MSG, t_socket)) {
		printf("error during send massage. exit thread\n");
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	check_if_str_is_allocated(&AcceptedStr);
	printf("Server sending SERVER_SETUP_REQUEST_MSG massage:\n");
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
	if (check_transaction_return_value(RecvRes, t_socket)){
		printf("error during send massage. exit thread\n");
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	printf("Server Recived Massage:\n%s\n", AcceptedStr);
	my_secret_number = get_4digit_number_from_massage(AcceptedStr);
	if (am_i_first == 1) 
		first_client_secret_number = my_secret_number;
	else 
		second_client_secret_number = my_secret_number;
	check_if_str_is_allocated(&AcceptedStr);
	if (!send_massage(SERVER_PLAYER_MOVE_REQUEST_MSG, t_socket)){
		printf("error during send massage. exit thread\n");
		free_service_thread_memories(t_socket, &AcceptedStr);
		return 1;
	}
	while (flag) {
		printf("Server sending SERVER_PLAYER_MOVE_REQUEST_MSG massage to client %s\n", Client_Name);
		RecvRes = ReceiveString(&AcceptedStr, *t_socket); //AcceptedStr is dynamic allocated, and should be free
		if (check_transaction_return_value(RecvRes, t_socket)){
			free_service_thread_memories(t_socket, &AcceptedStr);
			return 1;
		}
		massage_type = get_massage_type(AcceptedStr);
		get_str_of_massage_type(massage_type, Massage_type_str);
		my_geuss = get_4digit_number_from_massage(AcceptedStr);
		printf("server recieved massage CLIENT_PLAYER_MOVE with guess: %d \n ", my_geuss);
		sync_function();// Blocking until both thread will be here. the Porpuse is to make sure that the game file deleted and all information already set
		write_geuss_number_to_game_file(am_i_first, my_geuss);
		/*Already known:am_i_first,my_secret_number,other_secret_number, my_geuss Will be calculated within the function:My_cows,my_bulls,oponent_cows,oponent_bulls,other_client_geuss*/
		calculate_game_result(am_i_first, &my_cows, &my_bulls, my_secret_number, &other_secret_number, &oponent_cows, &oponent_bulls, &my_geuss, &other_client_geuss);
		printf("Client Name: %s, am_i_first: %d, my_cows: %d, my_bulls: %d, my_secret_number: %d\n,other_secret_number: %d,other_cows: %d, other_bulls: %d, my_geuss: %d, other_geuss: %d\n", Client_Name, am_i_first, my_cows, my_bulls, my_secret_number, other_secret_number, oponent_cows, oponent_bulls, my_geuss, other_client_geuss);
		char temp_buffer[SEND_STR_SIZE];
		format_game_results(my_bulls, my_cows, Oponent_Client_Name, other_client_geuss, temp_buffer);
		concatenate_str_for_msg(SERVER_GAME_RESULTS_MSG, temp_buffer, SendStr);
		printf("Server Send massage:\n%s\n", SendStr);
		if (!send_massage(SendStr, t_socket)){
			free_service_thread_memories(t_socket, &AcceptedStr);
			printf("error during send massage. exit thread\n");
			return 1;
		}
		check_if_str_is_allocated(&AcceptedStr);
		if ((my_bulls == 4) && (oponent_bulls == 4)) {
			if (!send_massage(SERVER_DRAW_MSG, t_socket)){
				free_service_thread_memories(t_socket, &AcceptedStr);
				printf("error during send massage. exit thread\n");
				return 1;
			}
			flag = 0;
			goto server_main_menu;
		}
		else if (my_bulls == 4) {
			memset(SendStr, 0, sizeof(SendStr));
			format_win_result(Client_Name, other_secret_number, temp_buffer);
			concatenate_str_for_msg(SERVER_WIN_MSG, temp_buffer, SendStr);
			printf("Server Send massage:\n%s\n", SendStr);
			if (!send_massage(SendStr, t_socket)){
				printf("error during send massage. exit thread\n");
				free_service_thread_memories(t_socket, &AcceptedStr);
				return 1;
			}
			flag = 0;
			goto server_main_menu;
		}
		else if (oponent_bulls == 4) {
			format_win_result(Oponent_Client_Name,other_secret_number, temp_buffer);
			concatenate_str_for_msg(SERVER_WIN_MSG, temp_buffer, SendStr);
			printf("Server Send massage:\n%s\n", SendStr);
			if (!send_massage(SendStr, t_socket)) {
				printf("error during send massage. exit thread\n");
				free_service_thread_memories(t_socket, &AcceptedStr);
				return 1;
			}
			flag = 0;
			goto server_main_menu;
		}
		else {
			if (!send_massage(SERVER_PLAYER_MOVE_REQUEST_MSG, t_socket)){
				printf("error during send massage. exit thread\n");
				free_service_thread_memories(t_socket, &AcceptedStr);
				return 1;
			}
		}
		//TBD:unexpected disconnection of one of the client: send server_opponent_quit and then server_main_menu
	}
	return 0;
}

void calculate_game_result (int am_i_first, int* my_cows, int* my_bulls,int my_secret_number,int* other_secret_number,int* oppnent_cows, int* oponent_bulls, int* my_geuss, int* other_client_geuss) {
	write_lock(game_result_lock);
	reader_count++;
	HANDLE Thread_connection_file = CreateFileA(
		FILE_NAME,
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	int  other_client_secret_number;
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file calculate_game_result function\n");
		gracefull_server_shutdown(NULL);
	}
	char first_temp_num[5], second_temp_num[5];
	char buffer[MAX_MASSAGE_TYPE];
	read_from_file(Thread_connection_file, buffer);
	int j = 0;
	for (int i = 0; i < 4; i++) {
		first_temp_num[j] = buffer[i];
		second_temp_num[j] = buffer[i + 4];
		j++;
	}
	first_temp_num[j] = '\0';
	second_temp_num[j] = '\0';
	if (am_i_first == 1) {
		other_client_secret_number = second_client_secret_number;
		*other_client_geuss = atoi(second_temp_num);
		*my_geuss = atoi(first_temp_num);
	}
	else {
		other_client_secret_number = first_client_secret_number;
		*my_geuss = atoi(second_temp_num);
		*other_client_geuss = atoi(first_temp_num);
	}
	if (!CloseHandle(Thread_connection_file)) {
		printf("Can't close the thread within read_file_get_opponent_user_name function\n");
		gracefull_server_shutdown(NULL);
	}
	*other_secret_number = other_client_secret_number;
	game_calculate_and_update_status(other_client_secret_number, *my_geuss, my_cows, my_bulls);
	game_calculate_and_update_status(my_secret_number, *other_client_geuss, oppnent_cows, oponent_bulls);
	printf("am_i_first = %d, number of bulls - %d, number of cows - %d,\n",am_i_first, *my_bulls, *my_cows);
	if (reader_count == 2) {
		if (DeleteFile(FILE_NAME) == 0) {
			printf("didnt delete file within read_file_get_opponent_name\n");
			printf("without it - the server can't continue the game, exit\n");
			gracefull_server_shutdown(NULL);
		}
		reader_count = 0;
	}
	write_release(game_result_lock);
	
}

int write_client_name_to_game_file(int* am_i_first,char* Client_Name, int client_name_length) {
	//am_i_first will be 1 only for the first client connected to the server.
	/*  CRITICAL Section: */
	write_lock(file_lock);
	HANDLE Thread_connection_file = CreateFileA(
		FILE_NAME,
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file withing write_client_name_to_game_file function. shutdown the server\n");
		gracefull_server_shutdown(NULL);
	}
	DWORD ret_val = get_file_orig_size(Thread_connection_file);
	if (-1 == ret_val) {
		printf("error within write_client_name_to_game_file function, shutdown\n");
		gracefull_server_shutdown(NULL);

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
	write_release(file_lock);

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
			gracefull_server_shutdown(NULL);
		}
		printf("Second Client,name: %s\n", Client_Name);
		printf("Second Thread released the semphore\n");
	}
	return 0;
}

void read_file_get_opponent_user_name(int am_i_first, char* Oponent_Client_Name, int my_user_name_size) {
	write_lock(file_lock);
	/*Critical Section begin*/
	reader_count++;
	HANDLE Thread_connection_file = CreateFileA(
		FILE_NAME,
		GENERIC_READ | GENERIC_WRITE, //Open file with write read
		FILE_SHARE_READ | FILE_SHARE_WRITE, //the file should be shared by the threads.
		NULL, //default security mode
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, //normal attribute
		NULL);
	int first_byte_position, last_byte_position;
	if (Thread_connection_file == INVALID_HANDLE_VALUE) {
		printf("ERROR opening the game file read_file_get_opponent_user_namee function\n");
		gracefull_server_shutdown(NULL);
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

	for (int i = first_byte_position; i < last_byte_position; i++) {
		Oponent_Client_Name[j] = file_str_name_contents[i];
		j++;
	}
	Oponent_Client_Name[j] = '\0';
	
	if (!CloseHandle(Thread_connection_file)) {
		printf("Can't close the file within read_file_get_opponent_user_name function\n");
		gracefull_server_shutdown(NULL);
	}
	if (reader_count == 1) {
		write_release(file_lock);
		DWORD wait_for_semaphore = WaitForSingleObject(semaphore_gun, TIMEOUT);
		if (wait_for_semaphore == WAIT_TIMEOUT)
		{
			printf("Timeout has been reached within write_client_name_to_game_file function for the semaphore\n");
		}
		else if (wait_for_semaphore == WAIT_OBJECT_0) {
			printf("First Client free! second thread realese me!\n");
		}
	}
	else if (reader_count == 2){
		write_release(file_lock);
		reader_count = 0;
		if (DeleteFileA(FILE_NAME) == 0) {
			printf("didnt delete file within read_file_get_opponent_name\nreason: %d\n",GetLastError());
		}
		if (!ReleaseSemaphore(semaphore_gun, 1, NULL)) {
			printf("Error: can't release the semphore gun within write_client_name_to_game_file function\n");
			gracefull_server_shutdown(NULL);
		}
		printf("Second Thread released the semphore\n");
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

void game_calculate_and_update_status(int oponent_secret_number, int my_geuss, int* cows, int* bulls) {

	int oponent_secret_num_array[4] = { oponent_secret_number % 10,(oponent_secret_number / 10) % 10,(oponent_secret_number / 100) % 10,(oponent_secret_number / 1000) % 10 };
	int my_geuss_array[4] = { my_geuss % 10,(my_geuss / 10) % 10,(my_geuss / 100) % 10,(my_geuss / 1000) % 10 };
	int num_of_bull_A = 0, num_of_cows_A = 0;
	for (int i = 0; i < 4; i++) {

		num_of_bull_A += (oponent_secret_num_array[i] == my_geuss_array[i]) ? 1 : 0;

		for (int j = 0; j < 4; j++) {
			if ((oponent_secret_num_array[i] == my_geuss_array[j]) && (i != j))
				num_of_cows_A += 1;
		}
	}
	*cows = num_of_cows_A;
	*bulls = num_of_bull_A;
}

//server gracefull shutdown- first recieve data, then shutdown and close socket
///and free all resources- semaphores and locks
int gracefull_server_shutdown(char* AcceptedStr) {
	int RecvRes, iResult;
	set_socket_timeout(SERVER_TIMEOUT, MainSocket);
	for (int i = 0; i < NUM_OF_WORKER_THREADS; i++) {
		if (&ThreadInputs[i] != NULL) {
			set_socket_timeout(SERVER_TIMEOUT, ThreadInputs[i]);
			RecvRes = ReceiveString(&AcceptedStr, ThreadInputs[i]);
			if (check_transaction_return_value(RecvRes, &ThreadInputs[i]))
				printf("error occourd within gracefull server shutdown.. continue to shutdown\n");
			shutdown(ThreadInputs[i], SD_SEND); //signal end of session and that server has no more data to send
			iResult = closesocket(ThreadInputs[i]);
			if (iResult == SOCKET_ERROR)
				printf("closesocket function failed with error: %ld\n", WSAGetLastError());
		}
		if (ThreadHandles[i] != NULL) {
			CloseHandle(ThreadHandles[i]);
		}
	}
	if (WSACleanup()) {//if wsacleanup failed
		printf("WSACleanup failed with error code: %d\n", WSAGetLastError());
	}
	if (AcceptedStr != NULL) {//have to check the recieved massage?
		free(AcceptedStr);
	}
	kill_semaphores();
	kill_locks();
	exit(0);
}	

void format_game_results(int my_bulls, int my_cows, char* Oponent_Client_Name,int other_client_geuss, char* temp_buffer) {
	char temp_num[7];
	_itoa_s(my_bulls, temp_num, 5, 10);
	strcpy_s(temp_buffer, 20, temp_num);
	strcat_s(temp_buffer, 25, ";");
	_itoa_s(my_cows, temp_num, 5, 10);
	strcat_s(temp_buffer, 30, temp_num);
	strcat_s(temp_buffer, 40, ";");
	strcat_s(temp_buffer, strlen(Oponent_Client_Name)+40 , Oponent_Client_Name);
	strcat_s(temp_buffer, SEND_STR_SIZE, ";");
	_itoa_s(other_client_geuss, temp_num, 5, 10);
	strcat_s(temp_buffer, SEND_STR_SIZE, temp_num);
}