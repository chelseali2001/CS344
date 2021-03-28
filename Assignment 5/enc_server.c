#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>

// Set up the address struct for the server socket.
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 
  
  // The address should be network capable.
  address->sin_family = AF_INET;
  // Store the port number.
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server.
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char bufferText[2048], bufferKey[2048];
  char client;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args.
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(2);
  } 
  
  // Create the socket that will listen for connections.
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    fprintf(stderr, "ERROR opening socket");
    exit(2);
  }

  // Set up the address struct for the server socket.
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port.
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    fprintf(stderr, "ERROR on binding");
    exit(2);
  }

  // Start listening for connetions. Allow up to 5 connections to queue up.
  listen(listenSocket, 5); 

  // Accept a connection, blocking if one is not available until one connects.
  while(1){
    // Accept the connection request which creates a connection socket.
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 

    if (connectionSocket < 0){
      fprintf(stderr, "ERROR on accept");
      exit(2);
    }
    
    int pid = fork();
    
    // If fork is successful, the value of spawnpid will be 0 in the child, the child's pid in the parent.
    if (pid < 0) {
      // Code in this branch will be exected by the parent when fork() fails and the creation of child process fails as well.               
      fprintf(stderr, "fork() failed!\n");
      exit(1);      
    } else if (pid == 0) {
      // Getting the size of the text file from client.
      int recievedSize = 0;
      // Read the client's message from the socket.
      charsRead = recv(connectionSocket, &recievedSize, sizeof(recievedSize), 0); 
      
      if (charsRead < 0){
        fprintf(stderr, "ERROR reading from socket");
        exit(2);
      }
      
      int textSize = ntohl(recievedSize);
      send(connectionSocket, &textSize, sizeof(int), 0); // Returning the size.
    
      // Check which server the client tried to connect with.
      recv(connectionSocket, &client, sizeof(char), 0);
      
      if (charsRead < 0){
        fprintf(stderr, "ERROR reading from socket");
        exit(2);
      }
      
      // If dec_client tries to connect to this server, return 'e' to show that dec_client tried to connect to the wrong server.
      if (client == 'd') {
        char actual = 'e';
        send(connectionSocket, &actual, sizeof(char), 0);
      } else {  
        send(connectionSocket, &client, sizeof(char), 0);
      } 
      
      // Getting the text file from client.
      int count = 0;
      // Clear out the buffer array
      memset(bufferText, '\0', 2048);
      
      char* buffer = (char *)malloc(sizeof(char) * (textSize + 10));
      size_t loc = 0;
      // Clear out the buffer array
      memset(buffer, '\0', sizeof(char) * (textSize + 10));
      
      // Reading the client's message from the socket chunk by chunk.
      while (count < textSize) {
        charsRead = recv(connectionSocket, bufferText, sizeof(bufferText) - 1, 0); 
        if (charsRead < 0) {
          fprintf(stderr, "ERROR reading from socket");
          exit(2);
        }
        
        memcpy(&buffer[loc], bufferText, charsRead);
        count += charsRead;
        loc += charsRead;
      }
      
      // Getting the key file from client.
      int count0 = 0;
      // Clear out the buffer array
      memset(bufferKey, '\0', 2048);
      
      char* buffer0 = (char *)malloc(sizeof(char) * (textSize + 10));
      size_t loc0 = 0;
      // Clear out the buffer array
      memset(buffer0, '\0', sizeof(char) * (textSize + 10));
      
      // Reading the client's message from the socket chunk by chunk.
      while (count0 < textSize) {
        charsRead = recv(connectionSocket, bufferKey, sizeof(bufferKey) - 1, 0); 
        if (charsRead < 0) {
          fprintf(stderr, "ERROR reading from socket");
          exit(2);
        }
         
        memcpy(&buffer0[loc0], bufferKey, charsRead);
        count0 += charsRead;
        loc0 += charsRead;
      }
         
      // Encryption  
      int x = 0;

      while (buffer[x] != '\n') {
        int val1 = 0, val2 = 0, total = 0;

        // Combining the key and message with modular addition. 
        if (buffer[x] == ' ' && buffer0[x] == ' ') {
          val1 = 26;
          val2 = 26;
        } else if (buffer[x] == ' ') {
          val1 = 26;
          val2 = (int) buffer0[x] - 65;
        } else if (buffer0[x] == ' ') {
          val1 = (int) buffer[x] - 65;
          val2 = 26;
        } else {
          val1 = (int) buffer[x] - 65;
          val2 = (int) buffer0[x] - 65;
        }
        
        total = 65 + ((val1 + val2) % 27);

        if (total == 91) {
          buffer[x] = ' ';
        } else {
          buffer[x] = (char) total;
        }
        
        x++;
      }
   
      // Returning the encrypted message.
      char* buffer1 = (char *)malloc(sizeof(char) * (textSize + 10));
      size_t loc1 = 0;
      int count1 = 0;
      
      // Send encrypted message back to the client chunk by chunk.
      while (count1 < textSize) {
        // Clear out the buffer array
        memset(buffer1, '\0', sizeof(buffer1));
        memcpy(&buffer1[loc1], buffer, textSize);
        
        charsRead = send(connectionSocket, buffer1, textSize, 0); 
        if (charsRead < 0){
          fprintf(stderr, "ERROR writing to socket");
          exit(2);
        }
        
        count1 += charsRead;
        loc1 += charsRead;
      }      
      
      // Close the connection socket for this client
      close(connectionSocket); 
    }
  }
  
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
