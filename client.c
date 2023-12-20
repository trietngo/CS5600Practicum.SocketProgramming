/*
 * client.c -- TCP Socket Client
 * 
 * 
 * Triet Ngo / CS 5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "clientHelper.h"
#define BUFFER_SIZE 8196


// Main function of the rfs executable
int main(int argc, char *argv[]) {

  // Check if user enters the minimum number of arguments
  // Minimum is 3, maximum is 4: executable, COMMAND, arg1, arg2
  // COMMAND is either WRITE, GET, RM (remove), or LS
  // If no remote file path is specified, then file is written to
  // to a remote path the same as local file path
  
  if (argc < 3 || argc > 4) {
    printf("Invalid arguments. Please specify the following:\n");
    printf("--Command: WRITE, GET, RM, LS\n");
    printf("--The local file path\n");
    printf("--The remote file path\n");
    printf("Terminating...\n");
    return -1;
  }

  // Initialize
  int socket_desc;
  struct sockaddr_in server_addr;

  // Message buffers
  char server_message[BUFFER_SIZE], client_message[BUFFER_SIZE];

  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));

  // Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc < 0){
      printf("Cannot create socket\n");
      return -1;
  }

  printf("Socket created.\n");

  // Initialize Server's Address Info
  // Set port and IP the same as server-side:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  // Send a connection request to server
  if (connect(socket_desc, 
      (struct sockaddr*)&server_addr, 
      sizeof(server_addr)) < 0) {
    printf("Failed to connect\n");
    return -1;
  };
  printf("Connection established. Input Request Below.\n");

  // Check command syntax
  int commandIsWrite = strcmp(argv[1], "WRITE");
  int commandIsGet = strcmp(argv[1], "GET");
  int commandIsRemove = strcmp(argv[1], "RM");
  int commandIsLs = strcmp(argv[1], "LS");

  // If no remote path is specified, remote path
  // is the same as local path. Send command to server
  if (argc == 3) {
    clientSendCommand(socket_desc, argv[1], argv[2], argv[2]);
  } else {
    clientSendCommand(socket_desc, argv[1], argv[2], argv[3]);
  }

  // Get response from server
  char server_response[BUFFER_SIZE];
  memset(server_response, '\0', sizeof(server_response));
  if (recv(socket_desc, server_response, 
           sizeof(server_response), 0) < 0){
    printf("Couldn't receive\n");
    return -1;
  }

  // If all arguments are satisfied
  // --exe
  // --command
  // --local path
  // --remote path
  // And server is ready
  // Execute

  // Compare string to get server acknowledgement status
  printf("Server acknowledged: %s\n", server_response);
  int server_ack = strcmp(server_response, "READY");

  // Check acknowledgement
  if (server_ack == 0) {

    // If there are only 3 args
    if (argc == 3) {

      // WRITE Command
      if (commandIsWrite == 0) {

        // Execute Write Command
        clientWriteToServer(socket_desc, argv[2], argv[2]);
      }

      // GET Command
      else if (commandIsGet == 0) {
        
        // Client READY response
        if (send(socket_desc, "CLIENT_READY", strlen("CLIENT_READY"), 0) < 0) {
          printf("Can't post to server\n");
          return -1;
        };

        // Receive data from server
        char data_buffer[BUFFER_SIZE];
        memset(data_buffer, '\0', sizeof(data_buffer));
        if (recv(socket_desc, data_buffer, 
                 sizeof(data_buffer), 0) < 0){
          printf("Couldn't receive\n");
          return -1;
        }

        // Execute GET command
        clientGetFromServer(argv[2], data_buffer);

      }
      
      // RM Command
      else if (commandIsRemove == 0) {

        clientSendCommand(socket_desc, argv[1], argv[2], argv[2]);

        // Delete confirmation
        char delete_confirmation[BUFFER_SIZE];
        memset(server_message, '\0', sizeof(server_message));
        if (recv(socket_desc, delete_confirmation, 
                 sizeof(delete_confirmation), 0) < 0){
          printf("Couldn't receive\n");
          return -1;
        }

        printf("%s\n", delete_confirmation);
      }

      // LS Command
      else if (commandIsLs == 0) {

        clientSendCommand(socket_desc, argv[1], argv[2], argv[2]);
        
        // Receive metadata
        char file_metadata[BUFFER_SIZE];
        memset(server_message, '\0', sizeof(server_message));
        if (recv(socket_desc, file_metadata, 
                 sizeof(file_metadata), 0) < 0){
          printf("Couldn't receive\n");
          return -1;
        }

        printf("%s\n", file_metadata);
      }
    }

    // If there are more than 3 args 
    else {
      
      // WRITE Command
      if (commandIsWrite == 0) {
        // Execute Write Command
        clientWriteToServer(socket_desc, argv[2], argv[3]);
      }

      // GET Command
      else if (commandIsGet == 0) {

        // Client READY response
        if (send(socket_desc, "CLIENT_READY", strlen("CLIENT_READY"), 0) < 0) {
          printf("Can't post to server\n");
          return -1;
        };

        // Receive data from server
        if (recv(socket_desc, server_message, sizeof(server_message),0) < 0) {
          printf("Can't receive from server\n");
          return -1;
        };

        // Execute the GET command
        clientGetFromServer(argv[3], server_message);
      }

    }
  }

  // Receive status after execution is done
  memset(server_response, '\0', sizeof(server_response));
  if (recv(socket_desc, server_response, 
           sizeof(server_response), 0) < 0){
    printf("Couldn't receive\n");
    return -1;
  }

  // Close Connection
  close(socket_desc);
  
  return 0;
}
