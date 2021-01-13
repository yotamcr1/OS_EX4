#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

//Some of the fils is taken from ExampleCode of Recitation 10, course "Introduction to Operation Systems."

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;

int get_str_of_massage_type(int type, char* destination);

int get_massage_type(const char* str);

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );

TransferResult_t SendString( const char *Str, SOCKET sd );

TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

TransferResult_t ReceiveString( char** OutputStrPtr, SOCKET sd );

char* receive_msg(SOCKET socket, char* AcceptedStr, int* massage_type);

void concatenate_str_for_msg(char* massage_type, char* parameter, char* SendStr);

int check_transaction_return_value(TransferResult_t tr, SOCKET* t_socket);
#endif // SOCKET_SEND_RECV_TOOLS_H