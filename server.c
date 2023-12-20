/*
 * server.c -- TCP Socket Server
 * 
 * adapted from: 
 *   https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 * Triet Ngo / CS 5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "serverHelper.h"
#include <pthread.h>

#define BUFFER_SIZE 8196

// Main function of the server executable
int main(void) {

  // Initialize socket programming
  int socket_desc, client_sock;
  socklen_t client_size;
  struct sockaddr_in server_addr, client_addr;
  char server_message[BUFFER_SIZE], client_message[BUFFER_SIZE];
  
  // Clean buffers:
  memset(server_message, '\0', sizeof(server_message));
  memset(client_message, '\0', sizeof(client_message));
  
  // Create socket:
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  
  if (socket_desc < 0) {
    printf("Error while creating socket\n");
    return -1;
  }
  printf("Socket created successfully\n");
  
  // Set port and IP:
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  
  // Bind to the set port and IP:
  if (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("Couldn't bind to the port\n");
    return -1;
  }
  printf("Done with binding\n");

  // Keep listening for requests
  while(1) {
    
    // Listen for clients:
    if(listen(socket_desc, 1) < 0){
      printf("Error while listening\n");
      return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);

    if (client_sock < 0){
      printf("Can't accept\n");
      return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", 
           inet_ntoa(client_addr.sin_addr), 
           ntohs(client_addr.sin_port));    

    // Reset client message after every loop
    memset(client_message, '\0', sizeof(client_message));

    // Create new client thread struct
    ClientThreadArgs *thread_args = malloc(sizeof(ClientThreadArgs));
    
    // Create thread for each connection
    thread_args->client_sock = client_sock;
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_routine, (void *)thread_args) != 0) {
      perror("Error creating new thread...");
      
      // Free memory and close socket when done
      free(thread_args);  
      close(client_sock);
      continue;           
    }

    // Wait for other threads to finish
    pthread_join(client_thread, NULL);
    
  }
  
  // Closing the socket:
  close(socket_desc);
  
  return 0;
}
