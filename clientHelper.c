/*
 * clientHelper.c / Practicum II
 *
 * Triet Ngo / CS5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 *
 */

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "clientHelper.h"

#define BUFFER_SIZE 8196

// This function send the command line arguments to server
//// socket_desc: socket to connect to
//// command: WRITE, GET, RM, or LS
//// arg1: argv[2]
//// arg2: argv[3]
void clientSendCommand(int socket_desc, const char* command, const char* arg1, const char* arg2) {

  // Initialize command buffer to send to server
  char command_to_server[BUFFER_SIZE] = "\0";

  // Construct a command to send to server by concatenation
  const char* space = " ";
  
  strcat(command_to_server, command);
  strcat(command_to_server, space);
  strcat(command_to_server, arg1);
  strcat(command_to_server, space);
  strcat(command_to_server, arg2);

  // Null terminator to prevent leak
  command_to_server[strlen(command_to_server)] = '\0';

  // Send command to server
  if (send(socket_desc, command_to_server, strlen(command_to_server),0) < 0) {
    printf("Can't send to server\n");
  };
}

// This function sends data from the specified local file to
// a specified directory in the server. If remote path is not
// specified, the server will use the local path instead
//// socket_desc: socket to connect to
//// local_path: argv[2], origin path
//// remote_path: argv[3], remote path (optional, use argv[2] if unspecified)
void clientWriteToServer(int socket_desc, const char* local_path, const char* remote_path) {

  // Initialize data to be sent
  char data_to_server[BUFFER_SIZE] = "\0";

  // Load local data onto client_message
  FILE *localFile;

  // Open local file and read data
  localFile = fopen(local_path, "r");

  // Check if local file exists
  if (localFile == NULL) {
    printf("Error opening file. Terminating...\n");
    return;
  }

  // Load data onto client message line by line
  char line[BUFFER_SIZE];
  while(!feof(localFile)) {
    if (fgets(line, sizeof(line), localFile) != NULL) {
      strcat(data_to_server, line);
    };
  }

  // Close file when done
  fclose(localFile);

  // Send data to server
  if (send(socket_desc, data_to_server, strlen(data_to_server),0) < 0) {
    printf("Can't send to server\n");
    return;
  };
}

// This function gets data from the specified remote file from
// the server. If the local path is not specified, the client will 
// use the remote path instead
//// remote_path: argv[3], remote path (optional, use argv[2] if unspecified)
//// data: data received
void clientGetFromServer(const char* remote_path, char* data) {
  
  // Get last occurence of "/" to split path
  // into directory and file
  char* dash = strrchr(remote_path, '/');

  // Create a local repo whenever client GET a file
  if (dash != NULL) {

    // Position of the last dash
    int pos = dash - remote_path + 1;

    // Get directories
    char directories[strlen(remote_path)];

    // Copy the directories path
    snprintf(directories, pos, "%s", remote_path);

    // Initialize local repo directory
    char remote_file_directory[BUFFER_SIZE] = "localRepo";

    // Create directories: localRepo/local_path
    int remote_server = mkdir(remote_file_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    strcat(remote_file_directory, "/");
    strcat(remote_file_directory, directories);

    int remote_folder = mkdir(remote_file_directory, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  } 

  // Initialize file path with local root repo
  char remote_file_location[BUFFER_SIZE] = "localRepo/";
  strcat(remote_file_location, remote_path);

  // Initialize data sent by server
  char remote_data[BUFFER_SIZE];
  strcpy(remote_data, data);
  remote_data[strlen(data)] = '\0';

  // Open a new file in local
  FILE* remote_file;
  remote_file = fopen(remote_file_location, "w");
  if (remote_file == NULL) {
    printf("Error opening file. Terminating...\n");
    return;
  }

  // Write all data to the file
  fprintf(remote_file, "%s", remote_data);

  // Close file when done
  fclose(remote_file);
  
  return;
}
