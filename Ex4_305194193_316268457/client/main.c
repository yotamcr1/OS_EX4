#define MAX_USER_NAME 20
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

int main(int argc, char* argv) {
	
	printf("welcom to main within client project\n");
	unsigned long serverIP_Address;
	int serverPort;
	char username[MAX_USER_NAME];

	serverIP_Address = inet_addr(argv[1]);
	serverPort = atoi(argv[2]);
	int res = strcpy_s(username, 20, argv[3]);

	//TBD: CHECKS INPUTS AND ETC

	ClientMain();

}