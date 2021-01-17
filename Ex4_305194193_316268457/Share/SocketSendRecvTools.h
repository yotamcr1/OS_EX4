#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
//Authers: Chen Katz And Yotam Carmi

//Some of the fils is taken from ExampleCode of Recitation 10, course "Introduction to Operation Systems."

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;


//the function convert input str to an output integer with the massage type
//input: string of message
//output: integer of massage type
int get_massage_type(const char* str);

//the function uses a socket to send a buffer
//output:TRNS_SUCCEEDED - if sending succeeded
//TRNS_FAILED - otherwise
TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );

//the function uses a socket to send a string.
//TRNS_SUCCEEDED - if sending succeeded
//TRNS_FAILED - otherwise
TransferResult_t SendString( const char *Str, SOCKET sd );

//the function uses a socket to receive a buffer
//output:TRNS_SUCCEEDED - if receiving succeeded
//TRNS_DISCONNECTED - if the socket was disconnected
//TTRNS_FAILED - otherwise
TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

//the function uses a socket to receive a string, and stores it in dynamic memory
//output:TRNS_SUCCEEDED - if receiving and memory allocation succeeded
//TRNS_DISCONNECTED - if the socket was disconnected
// TRNS_FAILED - otherwise
TransferResult_t ReceiveString( char** OutputStrPtr, SOCKET sd );

//the function treat recieve massage and return the massage type
//if the function success, AcceptedStr is dynamic allocated and should be free!
char* receive_msg(SOCKET socket, char* AcceptedStr, int* massage_type);

//the function cocaatenate between the massage type string to the parameters string of the messege
//inputs: massage_type string, parameters string and sendstr string
//output: the full message that contains all the information: type and parameters
void concatenate_str_for_msg(char* massage_type, char* parameter, char* SendStr);

int check_transaction_return_value(TransferResult_t tr, SOCKET* t_socket);

//the functions set the timeout for the socket to the input timeout
void set_socket_timeout(DWORD timeout, SOCKET socket);

//used to free memory of str and set it back to null, for compability with Recived massage
void check_if_str_is_allocated(char** str);

#endif // SOCKET_SEND_RECV_TOOLS_H