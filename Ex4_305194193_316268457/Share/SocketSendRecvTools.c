//Authers: Chen Katz And Yotam Carmi


#include "SocketSendRecvTools.h"
#include "Massage.h"

//Some of the fils is taken from ExampleCode of Recitation 10, course "Introduction to Operation Systems."

int get_massage_type(const char* str) {

	char temp_str[MAX_MASSAGE_TYPE];
	int i = 0;
	for (i = 0; i < MAX_MASSAGE_TYPE; i++) {
		if ((str[i] != ':') && (str[i] != '\n')) { //end of massage or end of massage type reached. 
			temp_str[i] = str[i];
		}
		else break;
	}
	temp_str[i] = '\0';
	//strcmp return zero on identical values!!!!!
	if (!strcmp(temp_str, "CLIENT_REQUEST"))
		return CLIENT_REQUEST;
	if (!strcmp(temp_str, "CLIENT_VERSUS"))
		return CLIENT_VERSUS;
	if (!strcmp(temp_str, "CLIENT_PLAYER_MOVE"))
		return CLIENT_PLAYER_MOVE;
	if (!strcmp(temp_str, "CLIENT_DISCONNECT"))
		return CLIENT_DISCONNECT;
	if (!strcmp(temp_str, "SERVER_MAIN_MENU"))
		return SERVER_MAIN_MENU;
	if (!strcmp(temp_str, "SERVER_APPROVED"))
		return SERVER_APPROVED;
	if (!strcmp(temp_str, "SERVER_DENIED"))
		return SERVER_DENIED;
	if (!strcmp(temp_str, "SERVER_INVITE"))
		return SERVER_INVITE;
	if (!strcmp(temp_str, "SERVER_SETUP_REQUEST"))
		return SERVER_SETUP_REQUEST;
	if (!strcmp(temp_str, "SERVER_PLAYER_MOVE_REQUEST"))
		return SERVER_PLAYER_MOVE_REQUEST;
	if (!strcmp(temp_str, "SERVER_GAME_RESULTS"))
		return SERVER_GAME_RESULTS;
	if (!strcmp(temp_str, "SERVER_WIN"))
		return SERVER_WIN;
	if (!strcmp(temp_str, "SERVER_DRAW"))
		return SERVER_DRAW;
	if (!strcmp(temp_str, "SERVER_NO_OPPONENTS"))
		return SERVER_NO_OPPONENTS;
	if (!strcmp(temp_str, "SERVER_OPPONENT_QUIT"))
		return SERVER_OPPONENT_QUIT;

	return INVALID_MASSAGE_TYPE;
}

int get_str_of_massage_type(int type,char* destination) {
	char massage_type[MAX_MASSAGE_TYPE];
	int ret_val = 0 ; //only one of the condition can be satisfied. if it is, and strcpy_s failed, then ret_val will be !=0. 
	if (type == CLIENT_REQUEST)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "CLIENT_REQUEST");
	if (type == CLIENT_VERSUS)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "CLIENT_VERSUS");
	if (type == CLIENT_SETUP)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "CLIENT_SETUP");
	if (type == CLIENT_PLAYER_MOVE)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "CLIENT_PLAYER_MOVE");
	if (type == CLIENT_DISCONNECT)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "CLIENT_DISCONNECT");
	if (type == SERVER_MAIN_MENU)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_MAIN_MENU");
	if (type == SERVER_APPROVED)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_APPROVED");
	if (type == SERVER_DENIED)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_DENIED");
	if (type == SERVER_INVITE)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_INVITE");
	if (type == SERVER_SETUP_REQUEST)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_SETUP_REQUSET");
	if (type == SERVER_PLAYER_MOVE_REQUEST)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_PLAYER_MOVE_REQUEST");
	if (type == SERVER_GAME_RESULTS)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_GAME_RESULTS");
	if (type == SERVER_NO_OPPONENTS)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_NO_OPPONENTS");
	if (type == SERVER_OPPONENT_QUIT)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_OPPONENT_QUIT");
	if (type == SERVER_OPPONENT_QUIT)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_OPPONENT_QUIT");
	if (type == SERVER_WIN)
		ret_val = strcpy_s(massage_type, MAX_MASSAGE_TYPE, "SERVER_WIN");

	if (ret_val) { //strcpy_s return zero on success! 
		printf("strcpy_s failed within get_str_of_massage_type function.\n");
		return 1;
	}
	return 0;
}

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	
	while ( RemainingBytesToSend > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
		if ( BytesTransferred == SOCKET_ERROR ) 
		{
			printf("send() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t SendString( const char *Str, SOCKET sd )
{
	/* Send the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the string (stored in 
	   an int variable ), then the string itself. */
		
	TotalStringSizeInBytes = (int)( strlen(Str) + 1 ); // terminating zero also sent	

	SendRes = SendBuffer( 
		(const char *)( &TotalStringSizeInBytes ),
		(int)( sizeof(TotalStringSizeInBytes) ), // sizeof(int) 
		sd );

	if ( SendRes != TRNS_SUCCEEDED ) 
		return SendRes ;

	SendRes = SendBuffer( 
		(const char *)( Str ),
		(int)( TotalStringSizeInBytes ), 
		sd );

	return SendRes;
}

TransferResult_t ReceiveBuffer( char* OutputBuffer, int BytesToReceive, SOCKET sd )
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;
	
	while ( RemainingBytesToReceive > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if (BytesJustTransferred == SOCKET_ERROR) 
		{
			printf("recv() failed, error %d\n", WSAGetLastError() );
			return TRNS_FAILED;
		}		
		else if ( BytesJustTransferred == 0 )
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}

TransferResult_t ReceiveString( char** OutputStrPtr, SOCKET sd )
{
	/* Recv the the request to the server on socket sd */
	int TotalStringSizeInBytes;
	TransferResult_t RecvRes;
	char* StrBuffer = NULL;

	if ( ( OutputStrPtr == NULL ) || ( *OutputStrPtr != NULL ) )
	{
		printf("The first input to ReceiveString() must be " 
			   "a pointer to a char pointer that is initialized to NULL. For example:\n"
			   "\tchar* Buffer = NULL;\n"
			   "\tReceiveString( &Buffer, ___ )\n" );
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the string (stored in 
	   an int variable ), then the string itself. */
		
	RecvRes = ReceiveBuffer( 
		(char *)( &TotalStringSizeInBytes ),
		(int)( sizeof(TotalStringSizeInBytes) ), // 4 bytes
		sd );

	if ( RecvRes != TRNS_SUCCEEDED ) return RecvRes;

	StrBuffer = (char*)malloc( TotalStringSizeInBytes * sizeof(char) );

	if ( StrBuffer == NULL )
		return TRNS_FAILED;

	RecvRes = ReceiveBuffer( 
		(char *)( StrBuffer ),
		(int)( TotalStringSizeInBytes), 
		sd );

	if ( RecvRes == TRNS_SUCCEEDED ) 
		{ *OutputStrPtr = StrBuffer; }
	else
	{
		free( StrBuffer );
	}
		
	return RecvRes;
}


char* receive_msg(SOCKET socket,char* AcceptedStr,int* massage_type) {
	TransferResult_t RecvRes;
	RecvRes = ReceiveString(&AcceptedStr, socket); 
	if (check_transaction_return_value(RecvRes, &socket))
		return NULL;
	*massage_type = get_massage_type(AcceptedStr);
	return AcceptedStr;
}


void concatenate_str_for_msg(char* massage_type, char* parameter,char* SendStr) {
	strcpy_s(SendStr, (strlen(massage_type) + 1 ), massage_type);
	strcat_s(SendStr, (strlen(massage_type) + strlen(parameter) + 2) , parameter);
	strcat_s(SendStr, (strlen(massage_type) + strlen(parameter) + 2) , "\n");
}


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
 
void set_socket_timeout(DWORD timeout, SOCKET socket) {
	if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))) {
		printf("ERROR while trying to set the socket timeout within set_socket_timeout\n");
	}
}


void check_if_str_is_allocated(char** str) {
	if (NULL != *str) {
		free(*str);
		*str = NULL;
	}
}