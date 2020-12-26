
int main(int argc, char* argv) {

	printf("welcom to server main\n");
	int serverport = atoi(argv[1]);
	MainServer(serverport);
}

