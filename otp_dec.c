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
	int f = open(name, O_RDONLY);
	int length = lseek(f, 0, SEEK_END);//find the end of the file
	//printf("length: %d\n", length);
	close(f);//close the file
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
void checkText(char name[]) {
	//check for bad characters
	char buffer[200];
	memset(buffer, 0, sizeof(buffer));
	FILE *file = fopen(name, "r");//open the file
	if (file == NULL) {//if the file did not open print an error
		fprintf(stderr,"Error opening file\n");
		exit(1);
	}
	int c;//to hold value of letter
	char letter = 'A';
	//printf("buffer: %c\n", letter);
	while ((c = fgetc(file)) != EOF) { //get each character of the fie

		if ((c >= 65) && (c <= 90)) {//if character is A-Z continue
		}
		else if (c == 32) {//or if the character is a space continue
		}
		else if ((c == 10) || (c == '\n')) {//if the character is the end or newline its the end
			break;
		}
		else {//otherwise this character is invalide and and error is printed
			fprintf(stderr, "Invalisd characters in file\n");
			exit(1);
		}
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
	int get = 0, bytes = 0;//bytes to check how many have been sent
	int f = open(file, O_RDONLY);//open the file to read from
	//printf("C: opened file to read from\n");
	int bytesSent = 0;
	//printf("C: sending file\n");
	memset(buffer, '\0', sizeof(buffer));//clear the buffer
	while (bytesSent < length) {//make sure the whole file is sent
		memset(buffer, '\0', sizeof(buffer));//clear
		bytes = read(f, buffer, sizeof(buffer) - 1);//read from the file
		//printf("C buffer: %s\n", buffer);
		bytesSent += send(socket, buffer, strlen(buffer), 0);//send what we have read

	}
	memset(buffer, '\0', sizeof(buffer));
	close(f);//close the file
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
	memset(buffer, 0, sizeof(buffer));//clear the buffer
	int bytes = 0, charsRead = 0;//keep track of how many bytes have been sent
	//printf("C: getting ecripted\n");
	memset(msg, '\0', sizeof(msg));
	while (bytes < size-1) {//while we have gotten all the byte(subtract 1 to remove newline)

		charsRead = recv(fd, buffer, sizeof(buffer), 0);//recieve the characters
		//printf("C: bytes recieve %d\n", charsRead);
		sprintf(buffer, "%s\0", buffer);//put null terminater in buffer for strcat
		bytes += charsRead;//keep track for the while loop
		strcat(msg, buffer);//strcat the buffer into msg
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
	//variables very similar to example in client.c
	int socketFD, portNumber, charsWritten, charsRead;
	char encodedMsg[700];//used to hold message
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
	char validate[] = "otp_dec";//used to validate server/client


	//make sure we have all the information we need
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

	//Validate that this is the right server
	charsWritten = send(socketFD, validate, strlen(validate), 0);
	memset(buffer, '\0', sizeof(buffer));//clear the buffer
	if (charsWritten < 0) {//if nothing was sent print an error
		error("Client: Error writing from socket\n");
	}
	charsRead = 0;
	//the server should respond saying otp_dec or error if the server is incorect
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
	if (charsRead < 0) {
		error("Client: Error reading from socket\n");
	}
	//if the server is not otp_dec_d then print an error
	if (strcmp(buffer, "otp_dec") != 0) {
		fprintf(stderr, "Error, invalid client\n");
		exit(1);
	}




	//	char fileName[] = argv[1];
		//char keyFile[] = argv[2];
	memset(buffer, '\0', sizeof(buffer));//clear the buffer
	int fileLength = getLength(argv[1]);//get the length of the file
	int keyLength = getLength(argv[2]);//get the key length
	if (fileLength > keyLength) {//make sure the filelength is not greater than the key lenther
		fprintf(stderr, "Error, invalid size\n");
		exit(1);
	}
	char sizeBuffer[300];//this is used to send the size of the file to the server
	memset(sizeBuffer, '\0', sizeof(sizeBuffer));//clear the size buffer
	sprintf(sizeBuffer, "%d", fileLength);//add the integer size to string

	//if sizes are equal send the sizes
	charsWritten = send(socketFD, sizeBuffer, sizeof(sizeBuffer), 0);
	memset(buffer, '\0', sizeof(buffer));
	//Wait for response that they got the size
	charsRead = 0;
	//printf("C: checking good\n");
	while (charsRead == 0) {//While no characters have been sent
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);//Recieve notice that the size was recieved
		//printf("C: chars Read %d\n", charsRead);
	}
	if (strcmp(buffer, "good") == 0) {//if the size was reicieved by the server then continue
		//printf("C: got good\n");
		checkText(argv[2]);//check that the characters are valid
		checkText(argv[1]);
		sendFile(argv[1], socketFD, fileLength);//send the plaintext

		//wait to hear they got the file
		memset(buffer, '\0', sizeof(buffer));
		//charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
		//if (strcmp(buffer, "cont") == 0) {

		
		sendFile(argv[2], socketFD, keyLength);//send the key

		memset(buffer, '\0', sizeof(buffer));


		//Recieve the ecripted message
		getEncriptedMessage(&encodedMsg, fileLength, socketFD);
		//printf("%s", encodedMsg);
	//}
	}
	else { //the server did not recieve the size
		fprintf(stderr, "Error, Did not recieve size\n");

	}
	close(socketFD);

	return 0;

}





