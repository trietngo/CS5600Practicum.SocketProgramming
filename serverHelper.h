/*
 * serverHelper.h / Practicum II
 *
 * Triet Ngo / CS5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 *
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#ifndef SERVER_H
#define SERVER_H

// Thread structure that contains client socket
// Each thread containing a command corresponds to a server port
typedef struct Thread{
  int client_sock;
} ClientThreadArgs;

void serverSaveFile(char *remote_file_path, char *data);

void serverSendFile(int client_sock, char *remote_file_path);

void serverRemove(int client_sock, char *remote_file_path);

void serverList(int client_sock, char *remote_file_path);

void* client_routine(void* args);

#endif // SERVER_H
