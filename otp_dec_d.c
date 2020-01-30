#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int confirmClient(int socketfd) {
	char buffer[40];
	//printf("confirming\n");
	memset(buffer, '\0', sizeof(buffer));
	int charsRead = 0;
	while (charsRead == 0) {
		charsRead = recv(socketfd, buffer, sizeof(buffer) - 1, 0);
		if ((strcmp(buffer, "otp_dec")) == 0) {
			return 1;
		}
		else {
			return 0;
		}
		memset(buffer, '\0', sizeof(buffer));
	}
	return 0;

}



int getSize(int size, int fd) {

	char buffer[400];
	memset(buffer, '\0', sizeof(buffer));
	int charsRead = 0;
	while (charsRead == 0) {
		charsRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
	}
	size = atoi(buffer);
	//printf("size: %d\n", size);
	memset(buffer, '\0', sizeof(buffer));
	return size;
}

void getInput(int fd, char message[], int size) {
	int i = 0, bytes = 0, count;
	char buffer[2000];
	//char msg[300];
	int bytestorecieve = 0;
	//memset(buffer, '\0', sizeof(buffer));
	memset(message, '\0', sizeof(message));
	//printf("S: trying to get file\n");
	//printf("S size: %d\n", size);
	while (bytes < size) {
		memset(buffer, '\0', sizeof(buffer));
	//	printf("S waiting to recieve message\n");
		i = recv(fd, buffer, sizeof(buffer) - 1, 0);
		bytes += i;
	//	printf("S: bytes recieved from message: %d\n", bytes);
		strcat(message, buffer);
		//printf("S: message: %s\n", message);
		//printf("S: buffer: %s\n", buffer);
	}
	//strcat(message, '\n');
	//printf("S: gotfile\n");


	return;

}


/***********************************************
function: changeToInt
This function changes the value to the needed
value for calculation
************************************************/
int changeToCorrectInt(int i) {
	if (i == 32) {
		return 26;
	}
	else {
		return (i - 65);
	}
}


void decodeMessage(char msg[], char key[], int size) {
	int i;
	int letter = 'A', c, k;
	for (i = 0; i < size; ++i) {
		c = (int)msg[i];
		k = (int)key[i];
		//printf("letter to change %c\n", msg[i]);
		//printf("key int %d\n", k);
		//printf("C is %d\n", c);
		if (c != 10) {

		
		
			//printf("S: letter to change %c\n", (char)c);
			//if (c != 26 && k != 26) {
			c = changeToCorrectInt(c) - changeToCorrectInt(k);
				c = c % 27;
			//}
			//else {
			//	c = c - k;
				
			//}
			if (c < 0) {
				c += 27;
			}
			//c = (msg[i] -letter)+(key[i]-letter);

			//printf("S: letter changed int %d\n", c);



			//c = c % 27;
			if(c == 26 || c==64){
				msg[i] = ' ';
			//	printf("decoded letter = %c\n", (char)c);
			}
			else {
				c = letter + c;
				msg[i] = (char)c;
				//printf("decoded letter = %c\n", (char)c);
			}
		}

	}


//	printf("encoded message %s\n", msg);
	return;
}

int main(int argc, char *argv[]) {
	pid_t pid = -1;
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, charsWritten;
	socklen_t sizeOfClientInfo;
	char buffer[2000], message[7000], key[7000];
	struct sockaddr_in serverAddress, clientAddress;
	int fileSize = 0;
	if (argc < 2) {
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	} // Check usage & args
	else {
		//printf("Good to go\n");


	// Set up the address struct for this process(the server)
		memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out theaddress struct
		portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
		serverAddress.sin_family = AF_INET; // Create a network-capable socket
		serverAddress.sin_port = htons(portNumber); // Store the port number
		serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

		// Set up the socket
		listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
		if (listenSocketFD < 0) error("ERROR opening socket");
		//printf("Running\n");
		// Enable the socket to begin listening
		if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to port
			error("ERROR on binding\n");
		}


		//loop to continue to accept calls
		while (1) {

			listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5connections
			// Accept a connection, blocking if one is not available until one connects
			sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
			establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept

			if (establishedConnectionFD < 0) error("ERROR on accept\n");
			//printf("Connected\n");
			pid = fork();
			if (pid == -1) {
				error("Error in fork\n");
				exit(1);
			}
			if (pid == 0) {
				if (confirmClient(establishedConnectionFD)) {
					//send confirmation back
					char validate[] = "otp_dec";
					charsRead = send(establishedConnectionFD, validate, strlen(validate), 0);
					//printf("S: getting size\n");
					fileSize = getSize(&fileSize, establishedConnectionFD);//get the size
					char sizeBuffer[300];
					//	printf("S: got size\n");
						//memset(sizeBuffer, '\0', sizeof(sizeBuffer));
						//sprintf(sizeBuffer, "%d", fileSize);
						//printf("S: size buffer: %s\n", sizeBuffer);
					//	printf("S: size after function call: %d\n", fileSize);
						//confirm we got the size
					charsWritten = 0;
					//printf("S: sending good\n");
					charsWritten = send(establishedConnectionFD, "good", 4, 0);
					//printf("S: sent good\n");
					//memset(message, '\0', sizeof(message));
					getInput(establishedConnectionFD, &message, fileSize);
					//charsWritten = send(establishedConnectionFD, "cont", 4, 0);
					//memset(key, '\0', sizeof(key));
					getInput(establishedConnectionFD, &key, fileSize);
					//printf("S: got both files\n");
					decodeMessage(&message, &key, fileSize);
					//send the encoded message
					memset(buffer, '\0', sizeof(buffer));
					charsWritten = 0;
					//printf("S: trying to send decoded message: %s\n", message);
					while (charsWritten < fileSize) {

						charsWritten += send(establishedConnectionFD, message, strlen(message), 0);
						//printf("chars written %d\n", charsWritten);
					}
					//printf("sent decoded message\n");
					exit(0);
				}
				else {
					char notvalidated[] = "error";
					charsRead = send(establishedConnectionFD, notvalidated, sizeof(notvalidated), 0);
					fprintf(stderr, "Error, invalid client\n");
					exit(1);
					break;
				}

				exit(0);



			}
			int status;
			pid_t newPID = waitpid(pid, &status, WNOHANG);
			close(establishedConnectionFD);
		}
	}
	close(listenSocketFD); // Close the listening socket
	return 0;
}
















