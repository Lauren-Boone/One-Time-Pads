#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

void error(const char *msg) {
	perror(msg); exit(1); 
} // Error function used for reporting issues


/*********************************************
function: getLength()
arguments: string
description: This function recieves the name
of a file. The file is opened and check for
length. The length is returned
*********************************************/
int getLength(char name[]) {
	
	//find the length of the file
	int f = open(name, O_RDONLY);//open the file
	int length = lseek(f, 0, SEEK_END);//find the end
	//printf("length: %d\n", length);
	close(f);
	return length;//return the length
}


/*******************************************
Function: checkText
arguments: string
Returns: none
Description: This funtion checks that the characters
in the file are valid. The must be A-Z and
spaces.
***********************************************/
void checkText(char name[], int size) {
	//check for bad characters
	char buffer[200];
	int count = 0;
	memset(buffer, 0, sizeof(buffer));
	FILE *file = fopen(name, "r"); //open the file
	if (file == NULL) {//if file didnt open there was an error
		perror("Error opening file\n");
		exit(1);
	}
	
	int c;//holds int value of character in file
	char letter = 'A';
	//printf("buffer: %c\n", letter);
	while((c=fgetc(file))!=EOF){//get each character of file
		
		if((c >= 65) && (c <=90)){//if A-Z
		}
		else if(c==32) {//if space
		}
		else if((c == 10)||(c=='\n')){//if at end of file
			break;
		}
		else {//otherwise invalid character
			fprintf(stderr, "Invalisd characters in file\n");
			exit(1);
		}
		if (size == count) {//no need to count all the way to end of key
			break;
		}
		count++;
	}
	fclose(file);
	return;
}


/********************************************************
Function: sendFile()
Arguments: fileName, int, int
Returns: None
Description: This function sends a file to the server.
*********************************************************/
void sendFile(char file[], int socket, int length) {
	char buffer[2000];
	int get=0, bytes=0;//used to keep track of bytes
	int f = open(file, O_RDONLY);
	//printf("C: opened file to read from\n");
	int bytesSent = 0;
	//printf("C: sending file\n");
	memset(buffer, '\0', sizeof(buffer));
	while (bytesSent < length) {//make sure to send all the file
		memset(buffer, '\0', sizeof(buffer));
		bytes = read(f, buffer, sizeof(buffer)-1);//read the file
		//printf("C buffer: %s\n", buffer);
		bytesSent += send(socket, buffer, strlen(buffer), 0);//send what was read

	}
	memset(buffer, '\0', sizeof(buffer));//clear buffer
	close(f);
	//printf("C: sent file\n");
	return;

}


/**********************************************************
Function: getEncriptedMessage
Arguments: string, int, int
Returns: none
Description: This function recieves an encripted message.
The message is directed to stdout.
************************************************************/
void getEncriptedMessage(char msg[], int size, int fd) {
	char buffer[2000];
	memset(buffer, 0, sizeof(buffer));
	int bytes = 0, charsRead = 0;//keeps track of what was recieved
	//printf("C: getting ecripted\n");
	memset(msg, '\0', sizeof(msg));
	while (bytes < size-1) {//(size-1) so that newline is left out
		
		charsRead = recv(fd, buffer, sizeof(buffer), 0);//recieve the message
		//printf("C: bytes recieve %d\n", charsRead);
		sprintf(buffer, "%s\0", buffer);//null terminate for strcat
		bytes += charsRead;//keep track fo bytes recieves
		strcat(msg, buffer);//concatenate msg and buffer
		charsRead = 0;
		memset(buffer, '\0', sizeof(buffer));

	}
	//printf("C: got encoded message\n");
	//strcat(msg, '\n');
	
	printf("%s", msg);
	//printf("\n");
		return;
}

int main(int argc, char *argv[]) {

	//this is very similar to the client.c example
	int socketFD, portNumber, charsWritten, charsRead;
	char encodedMsg[700];//holds encoded message
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
	char validate[] = "otp_enc";//used to make sure we are dealing with the correct server


	//make sure we have the correct number of arguments
	if (argc < 4) {
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		exit(1);
	} // Check usage & args

	// Set up the server address struct 
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string


	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) {
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	bcopy((char *)&serverAddress.sin_addr.s_addr, (char *)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address
	//bcopy((char *)serverHostInfo->h_addr, (char *)&serverAddress.sin_addr.s_addr, serverHostInfo->h_length);
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {// Connect socket to address
		error("CLIENT: ERROR connecting\n");
	}
	//printf("connected\n");
	//printf("sending first validation\n");
	//Make sure we are using the right server
	charsWritten= send(socketFD, validate, strlen(validate),0);
	memset(buffer, '\0', sizeof(buffer));
	if (charsWritten < 0) {//if nothing was sent
		error("Client: Error writing from socket\n");
	}
	charsRead = 0;
	//validating message from server that this is the correct server
	charsRead=recv(socketFD, buffer, sizeof(buffer)-1,0);

	if (charsRead < 0) {//make sure we recieved something
		error("Client: Error reading from socket\n");
	}
	//the server will send otp_enc if this is the correct client 
	if (strcmp(buffer, "otp_enc") != 0) {
		fprintf(stderr, "Error, invalid client\n");
	exit(2);
	}


	

//	char fileName[] = argv[1];
	//char keyFile[] = argv[2];
	memset(buffer, '\0', sizeof(buffer));
	int fileLength = getLength( argv[1]);//get the length
	int keyLength = getLength(argv[2]);
	//printf("key: %d\nfile: %d\n", keyLength, fileLength);
	if (fileLength > keyLength) {
		//printf("key: %d\nfile: %d\n", keyLength, fileLength);
		fprintf(stderr, "Error, invalid size\n");
		exit(1);
	}

	//buffer to hold the size of the file in string format
	char sizeBuffer[300];
	memset(sizeBuffer, '\0', sizeof(sizeBuffer));
	sprintf(sizeBuffer, "%d", fileLength);//make the size a string

	//if sizes are equal send the sizes
	charsWritten = send(socketFD, sizeBuffer, sizeof(sizeBuffer), 0);
	memset(buffer, '\0', sizeof(buffer));
	//Wait for response that they got the size
	charsRead = 0;
	//printf("C: checking good\n");
	while (charsRead == 0) {
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);//recieve response taht they got the size
		//printf("C: chars Read %d\n", charsRead);
	}
	if (strcmp(buffer, "good")==0) {//server sends good if they got the size
		//printf("C: got good\n");
		checkText(argv[2],fileLength);//check that the characters are valid
		checkText(argv[1],fileLength);
		sendFile(argv[1], socketFD, fileLength);//send the plaintext

		//wait to hear they got the file
		memset(buffer, '\0', sizeof(buffer));
		//charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
		//if (strcmp(buffer, "cont") == 0) {

			sendFile(argv[2], socketFD, keyLength);//send the key

			memset(buffer, '\0', sizeof(buffer));


			//Recieve the ecripted message
			getEncriptedMessage(&encodedMsg, fileLength, socketFD);
			//FILE *outfile = fopen(argv[4], "w");
		//	fputs(encodedMsg, outfile);
		//	fclose(outfile);
			//printf("%s", encodedMsg);
		//}
	}
	else {
		error("Invalid Server\n");

	}
	close(socketFD);

	return 0;

}





