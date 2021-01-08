#include "SocketSendRecvTools.h"
#include <stdio.h>
#include <string.h>
#include "Massage.h"

//Some of the fils is taken from ExampleCode of Recitation 10, course "Introduction to Operation Systems."


int get_massage_type(const char* str) {

	char temp_str[MAX_MASSAGE_TYPE];
	int i = 0;
	for (i = 0; i < MAX_MASSAGE_TYPE; i++) {
		if ((str[i] != ':') && (str[i] != '\n')) { //end of massage or end of massage type reached. 
			temp_str[i] = str[i];
			break;
		}
	}
	temp_str[i + 1] = '\0';

	if (strcmp(temp_str, "CLIENT_REQUEST"))
		return CLIENT_REQUEST;
	if (strcmp(temp_str, "CLIENT_VERSUS"))
		return CLIENT_VERSUS;
	if (strcmp(temp_str, "CLIENT_PLAYER_MOVE"))
		return CLIENT_PLAYER_MOVE;
	if (strcmp(temp_str, "CLIENT_DISCONNECT"))
		return CLIENT_DISCONNECT;
	if (strcmp(temp_str, "SERVER_MAIN_MENU"))
		return SERVER_MAIN_MENU;
	if (strcmp(temp_str, "SERVER_APPROVED"))
		return SERVER_APPROVED;
	if (strcmp(temp_str, "SERVER_DENIED"))
		return SERVER_DENIED;
	if (strcmp(temp_str, "SERVER_INVITE"))
		return SERVER_INVITE;
	if (strcmp(temp_str, "SERVER_SETUP_REQUSET"))
		return SERVER_SETUP_REQUSET;
	if (strcmp(temp_str, "SERVER_PLAYER_MOVE_REQUEST"))
		return SERVER_PLAYER_MOVE_REQUEST;
	if (strcmp(temp_str, "SERVER_GAME_RESULTS"))
		return SERVER_GAME_RESULTS;
	if (strcmp(temp_str, "SERVER_WIN"))
		return SERVER_WIN;
	if (strcmp(temp_str, "SERVER_DRAW"))
		return SERVER_DRAW;
	if (strcmp(temp_str, "SERVER_NO_OPPONENTS"))
		return SERVER_NO_OPPONENTS;
	if (strcmp(temp_str, "SERVER_OPPONENT_QUIT"))
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
	if (type == SERVER_SETUP_REQUSET)
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

	if (!ret_val) {
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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

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


//the function treat recieve massage and return the massage type
int receive_msg(SOCKET socket) {
	int AcceptedStr, RecvRes;
	RecvRes = ReceiveString(&AcceptedStr, socket); 
	/*if (handle_return_value(RecvRes, m_socket))
	return 1;*/ //TBD: check RecvRes 
	int massage_type = get_massage_type(AcceptedStr);
	free(AcceptedStr);//AcceptedStr is dynamic allocated, and should be free
	return massage_type;
}