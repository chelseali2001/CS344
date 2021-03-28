#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Sending the message to be encrypted to the server.
* 3. Print the encrypted message received from the server and exit the program.
*/

// Set up the address struct.
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct.
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable.
  address->sin_family = AF_INET;
  // Store the port number.
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name.
  struct hostent* hostInfo = gethostbyname("localhost"); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(2); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr.
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

// Calculating the size of a file.
long int fileSize(char *argv) {
  FILE* fp = fopen(argv, "r"); 
  
  // If the file can't be opened.
  if (fp < 0) {
    fprintf(stderr, "Error opening file.\n");
    exit(1);
  }
  
  // Getting the size of the file.
  fseek(fp, 0L, SEEK_END); 

  return ftell(fp);
}

int main(int argc, char *argv[]) {
  fflush(stdout);
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char bufferText[2048], bufferKey[2048];
  char client = 'e';    
  long int text = fileSize(argv[1]);
  long int key = fileSize(argv[2]);
  uint32_t longLength = 10;
  
  // Check usage & args.
  if (argc < 3) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(2); 
  } 

  // Create a socket.
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    fprintf(stderr, "CLIENT: ERROR opening socket");
    exit(2);
  }

   // Set up the server address struct.
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server.
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "CLIENT: ERROR connecting");
    exit(2);
  }
  
  // Checking if the key is shorter than the message.
  if (key < text) {
    fprintf(stderr, "Error: key %s is too short\n", argv[2]);
    exit(1);  
  }
  
  // Sending the size of the message to the server.
  uint32_t un = htonl(text);
  send(socketFD, &un, sizeof(longLength), 0); 
  recv(socketFD, &un, sizeof(longLength), 0);

  // Letting the server know that this client is calling it.
  char response;
  send(socketFD, &client, sizeof(char), 0);
  recv(socketFD, &response, sizeof(char), 0);

  // Checking if the wrong server was called.
  if (response == 'd') {
    fprintf(stderr, "Error: could not contact enc_server on port %s\n", argv[3]);
    exit(2);
  }
  
  // Sending the text file.
  int fp = open(argv[1] , O_RDONLY);
  
  // If the file can't be opened.
  if (fp < 0) {
    fprintf(stderr, "Error opening file.\n");
    exit(1);
  }
  
  int count = 0, textLength = 0;
  
  // Sending the message to the sever chunk by chunk.
  while (count < text) {
    // Clear out the buffer array
    memset(bufferText, '\0', sizeof(bufferText));
    // Getting the message from the input file.
    textLength = read(fp, bufferText, sizeof(bufferText) - 1);

    int x = 0;
    
    // Checking if the input file contains any bad characters.
    while (bufferText[x] != '\n' && bufferText[x] != '\0') {
      if ((int) bufferText[x] < 65 || (int) bufferText[x] > 90) {
        if (bufferText[x] != ' ') {
          fprintf(stderr, "enc_client error: input contains bad characters\n");
          exit(1);      
        }
      }
      
      x++;
    }

    // Send message to server.
    // Write to the server.
    charsWritten = send(socketFD, bufferText, sizeof(bufferText) - 1, 0); 
    
    if (charsWritten < 0){
      fprintf(stderr, "CLIENT: ERROR writing to socket");
      exit(2);
    }
  
    count += textLength;
  }

  // Closing the file
  close(fp);
  
  // Sending key file.
  fp = open(argv[2] , O_RDONLY);
  
  // If the key file can't be opened.
  if (fp < 0) {
    fprintf(stderr, "Error opening file.\n");
    exit(1);
  }
  
  count = 0;
  textLength = 0;
  
  // Sending the key to the sever chunk by chunk.
  while (count < text) {
    // Clear out the buffer array.
    memset(bufferKey, '\0', sizeof(bufferKey));
    // Getting the key from the input file.
    textLength = read(fp, bufferKey, sizeof(bufferKey) - 1);
    
    // Send key to server.
    // Write to the server.
    charsWritten = send(socketFD, bufferKey, sizeof(bufferKey) - 1, 0); 
    
    if (charsWritten < 0){
      fprintf(stderr, "CLIENT: ERROR writing to socket");
      exit(2);
    }
   
    count += textLength;
  }
  
  // Closing the file.
  close(fp);

  // Get encrypted message from server.
  count = 0;  
  // Clear out the buffer again for reuse.
  memset(bufferText, '\0', 2048);
  
  char* buffer = (char *)malloc(sizeof(char) * (text + 10));
  size_t loc = 0;
  // Clear out the buffer again for reuse.
  memset(buffer, '\0', sizeof(char) * (text + 10));

  // Retrieving the encrypted message from the server chunk by chunk.
  while (count < text) {
    // Read data from the socket, leaving \0 at end.
    charsRead = recv(socketFD, bufferText, sizeof(bufferText) - 1, 0); 
    
    if (charsRead < 0) {
      fprintf(stderr, "ERROR reading from socket");
      exit(2);
    }
    
    memcpy(&buffer[loc], bufferText, charsRead);
    count += charsRead;
    loc += charsRead;
  }
  
  // Printing encrypted message.
  fprintf(stdout, "%s", buffer);
  
  // Close the socket.
  close(socketFD); 
  
  return 0;
}