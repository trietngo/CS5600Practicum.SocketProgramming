/*
 * serverHelper.c / Practicum II
 *
 * Triet Ngo / CS5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 *
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

// Initialize mutexes for multi-threading
pthread_mutex_t command_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t execution_lock = PTHREAD_MUTEX_INITIALIZER;

// This function receives a file path and data from client and saves
// the data on the server at the specified path upon receiving a
// WRITE Command
//// remote_file_path: the location sent by client
//// data: data sent by client
void serverSaveFile(char* remote_file_path, char* data) {
  
  // Get last occurence of "/" to split path
  // into directory and file
  char* dash = strrchr(remote_file_path, '/');
  int pos = dash - remote_file_path + 1;

  // Get directories
  char directories[strlen(remote_file_path)];

  // Copy the directories path
  snprintf(directories, pos, "%s", remote_file_path);

  // Initialize data to be written into remote file
  char remote_data[BUFFER_SIZE] = "\0";

  // Get client data
  strcpy(remote_data, data);
  remote_data[strlen(data)] = '\0';

  // Initialize remote path with the server root
  char remote_path[BUFFER_SIZE] = "remoteRepo";

  // Create root folder
  int remote_server = mkdir(remote_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  // Create directories extracted from client message
  strcat(remote_path, "/");
  strcat(remote_path, directories);
  remote_path[strlen(remote_path)] = '\0';
  int remote_folder = mkdir(remote_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  // Initialize file location using the server root
  char remote_file_location[BUFFER_SIZE] = "remoteRepo/";

  // Get the target location for new file to be wrttien
  strcat(remote_file_location, remote_file_path);

  // Write Data to a file
  FILE* remote_file;

  // Create new file path for versioning
  char new_file_path[512];

  // Check if the file already exists
  if (access(remote_file_location, F_OK) != -1) {

    // If file already exists, append version number starting at 1
    int version_num = 1;

    // Get the file extension
    char* file_extension = strrchr(remote_file_path, '.');

    // Initialize location of newer versions, starting with the root
    // Same as original version
    char* new_file_root = "remoteRepo/";
    char new_file_location[BUFFER_SIZE];
    
    // While the file already exists, keep doing versioning
    do {
      // If the file name exists, create new versions by appending
      // version number
      if (file_extension != NULL) {
        snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
      }

      // If the file extension does not exist, keep as is
      else {
        snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
      }

      // Construct new file location
      sprintf(new_file_location, "%s%s", new_file_root, new_file_path);

      // Increment version
      version_num++;
      
    // Repeat as long as the file already exists
    } while (access(new_file_location, F_OK) == 0);

    // Copy data onto the new versions
    remote_file = fopen(new_file_location, "w");
  } 

  // Else if file path does not already exist, create a new one
  else {
    remote_file = fopen(remote_file_location, "w");

    // Check for error
    if (remote_file == NULL) {
      printf("Error opening file. Terminating...\n");
      return;
    }
  }

  // Copy data onto the file
  fprintf(remote_file, "%s", remote_data);

  // Close file when done
  fclose(remote_file);
}

// This function sends data to client and from a remote file
// specified by the client upon receiving a GET Command.
// The function also returns the most recent, or highest-numbered
// version of the file, if applicable.
//// client_sock: client socket to send data to
//// remote_file_path: location of the requested file on the server
void serverSendFile(int client_sock, char *remote_file_path) {

  // Load local data onto client_message
  FILE *localFile;

  // Remote root directory:
  char* server_root = "remoteRepo/";

  // Get construct the directory the file is in
  char target_directory[512];
  strcpy(target_directory, server_root);
  target_directory[strlen(target_directory)] = '\0';

  // Get full path to file
  strcat(target_directory, remote_file_path);

  // GET THE LATEST VERSION
  // Construct a new version template
  char new_version_location[BUFFER_SIZE] = "\0";

  // Initialize new file locations, starting with the server root
  char* new_file_root = "remoteRepo/";

  // Initialize version number
  int version_num = 1;

  // Get file extension
  char* file_extension = strrchr(remote_file_path, '.');

  // Create new file path for versioning
  char new_file_path[512];

  // Loop to check for new versions
  // Construct the oldest new version of the file first
  if (file_extension != NULL) {
    snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
  }

  // If the file extension does not exist, keep as is
  else {
    snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
  }

  // Construct new file location
  sprintf(new_version_location, "%s%s", new_file_root, new_file_path);

  // While a version is accessible
  // find the next one and keep accessing
  // until the highest-numbered version is found
  while (access(new_version_location, F_OK) == 0) {

    // Increment version
    version_num++;

    // Construct the next one
    if (file_extension != NULL) {
      snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
    }

    // If the file extension does not exist, keep as is
    else {
      snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
    }

    // Construct newer version
    sprintf(new_version_location, "%s%s", new_file_root, new_file_path);
  }

  // The loop ends when a non-existent file is accessed
  // Rollback to the previous version number to construct
  // the most recently modified file
  version_num--;
  if (file_extension != NULL) {
    snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
  }

  // If the file extension does not exist, keep as is
  else {
    snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
  }
  sprintf(new_version_location, "%s%s", new_file_root, new_file_path);

  // If the most recent file is accessible
  if (access(new_version_location, F_OK) == 0) {
    
    // Open local file and read data
    localFile = fopen(new_version_location, "r");

    // Check if local file exists
    if (localFile == NULL) {
      printf("Error opening file. Terminating...\n");
      return;
    }
  }

  // Else, if previously accessible version does not exist
  // Meaning there is only one file in the directory
  else {

    // Open local file and read data
    localFile = fopen(target_directory, "r");

    // Check if local file exists
    if (localFile == NULL) {
      printf("Error opening file. Terminating...\n");
      return;
    }
  }

  // Load data onto client message line by line
  char line[BUFFER_SIZE];
  
  // Initialize data to be sent
  char data_to_client[BUFFER_SIZE] = "\0";

  // Construct data for client using concatenation
  while(!feof(localFile)) {
    if (fgets(line, sizeof(line), localFile) != NULL) {
      strcat(data_to_client, line);
    };
  }

  // Close file when done
  fclose(localFile);

  // Null terminate to prevent contamination
  data_to_client[strlen(data_to_client)] = '\0';

  // Send data to to client
  if (send(client_sock, data_to_client, strlen(data_to_client),0) < 0) {
    printf("Can't send to server\n");
  };
}

// This function, upon receiving the RM command,
// gets the file specified by client and
// delete it if it exists on the server. If applicable,
// all versions of that file will also be deleted
//// client_sock: client socket to send data to
//// remote_file_path: location of the requested file on the server
void serverRemove(int client_sock, char *remote_file_path) {

  // Initialize target root
  char target_file[512] = "remoteRepo/";

  // Append the target filename
  strcat(target_file, remote_file_path);

  // Remove oldest or only file
  if (remove(target_file) == 0) {

    // Send delete confirmation
    send(client_sock, "File deleted from server.", strlen("File deleted from server."), 0);
  }

  // Construct a new version template
  char new_version_location[BUFFER_SIZE] = "\0";

  // Initialize new file locations, starting with therver root
  char* new_file_root = "remoteRepo/";

  // Initialize version number
  int version_num = 1;

  // Get file extension
  char* file_extension = strrchr(remote_file_path, '.');

  // Create new file path for versioning
  char new_file_path[512];

  // Loop to check for new versions
  // Construct the oldest new version of the file first
  if (file_extension != NULL) {
    snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
  }

  // If the file extension does not exist, keep as is
  else {
    snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
  }

  printf("New_file_path is: %s\n", new_file_path);

  // Construct new file location
  printf("Version is: %d\n", version_num);
  sprintf(new_version_location, "%s%s", new_file_root, new_file_path);
  printf("New_version_location is: %s\n", new_version_location);

  // Check in the same folder if newer versions exist
  // While they exist, delete
  while (remove(new_version_location) == 0) {

    // Increment version
    version_num++;

    // Construct the next one
    if (file_extension != NULL) {
      snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
    }

    // If the file extension does not exist, keep as is
    else {
      snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
    }

    // Construct the next version's name to be passed
    // into the loop
    sprintf(new_version_location, "%s%s", new_file_root, new_file_path);
  }
    
}

// This function, upon receiving the LS command,
// lists the file specified by client as well as its
// newer versions and return version number as well as the most-
// recently modified timestamp, if applicable,
//// client_sock: client socket to send data to
//// remote_file_path: location of the requested file on the server
void serverList(int client_sock, char *remote_file_path) {
  
  // Initialize target root
  char target_file[512] = "remoteRepo/";

  // Append the target filename
  strcat(target_file, remote_file_path);

  // Check in the same folder if original version exists
  // If yes, get metadata 
  if (access(target_file, F_OK) == 0) {

    // Last modified time
    struct stat attr;
    stat(target_file, &attr);
    printf("File version: Original\n");
    printf("File: %s \t Last modified: %s\n", target_file, asctime(localtime(&attr.st_ctime)));

    // Store in a string to be send to client
    char metadata[BUFFER_SIZE];
    memset(metadata, '\0', BUFFER_SIZE);

    // Construct metadata to send to client
    snprintf(metadata, BUFFER_SIZE, "File version: Original\nFile: %s \t Last modified: %s\n", target_file, asctime(localtime(&attr.st_ctime)));

    // Send metadata to client
    send(client_sock, metadata, strlen(metadata), 0);
  }

  // Construct a new version template
  char new_version_location[BUFFER_SIZE] = "\0";

  // Initialize new file locations, starting with therver root
  char* new_file_root = "remoteRepo/";

  // Initialize version number
  int version_num = 1;

  // Get file extension
  char* file_extension = strrchr(remote_file_path, '.');

  // Create new file path for versioning
  char new_file_path[512];

  // Loop to check for new versions
  // Construct the oldest new version of the file first
  if (file_extension != NULL) {
    snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
  }

  // If the file extension does not exist, keep as is
  else {
    snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
  }

  // Construct new file location
  sprintf(new_version_location, "%s%s", new_file_root, new_file_path);

  // Metadata for versioned files
  char metadata_ver[BUFFER_SIZE];

  // While a version is accessible, find the next one and keep
  // accessing and get metadata
  while (access(new_version_location, F_OK) == 0) {

    // Last modified time
    struct stat attr;
    stat(new_version_location, &attr);
    
    // Construct newer version's metadata
    printf("File version: %d\n", version_num);
    printf("File: %s \t Last modified: %s\n", new_version_location, asctime(localtime(&attr.st_ctime)));

    sprintf(metadata_ver, "File version: %d\nFile: %s \t Last modified: %s\n", version_num, target_file, asctime(localtime(&attr.st_ctime)));

    // Send to client
    send(client_sock, metadata_ver, sizeof(metadata_ver), 0);

    // Increment version
    version_num++;

    // Construct the next one
    if (file_extension != NULL) {
      snprintf(new_file_path, sizeof(new_file_path), "%.*s_new%d%s", (int)(file_extension - remote_file_path), remote_file_path, version_num, file_extension);
    }

    // If the file extension does not exist, keep as is
    else {
      snprintf(new_file_path, sizeof(new_file_path), "%s_%d", remote_file_path, version_num);
    }

    // Construct the next version's name to be passed
    // into the loop
    sprintf(new_version_location, "%s%s", new_file_root, new_file_path);
  }
};

// Main routine function that parses the client message
// and executes commands from client accordiingly.
// Takes in thread arguments as each routine run is handled
// by a different thread.
void* client_routine(void* args) {

  // Access the ClientThreadAtgs to get client socket and client message
  ClientThreadArgs *client_args = (ClientThreadArgs *)args;
  int client_sock = client_args->client_sock;

  // Command line buffer
  char command_buf[BUFFER_SIZE];

  // Clear buffer
  memset(command_buf, '\0', sizeof(command_buf));

  // CRITICAL REGION: receive client's command.
  // No thread should interfere with another

  pthread_mutex_lock(&command_lock);
  
  // Receive client's command:
  if (recv(client_sock, command_buf, 
           sizeof(command_buf), 0) < 0) {
    printf("Couldn't receive\n");
  }

  printf("Command: %s\n", command_buf);

  // Unlock command mutex
  pthread_mutex_unlock(&command_lock);
  
  // Extract command, local and remote directories, if applicable
  // Token Iterator for extraction
  char* tok = strtok(command_buf, " ");

  // CRITICAL REGION: command extraction and execution
  pthread_mutex_lock(&execution_lock);
  
  // 1st Argument
  char arg1[512];
  strcpy(arg1, tok);
  arg1[strlen(tok)] = '\0';
  printf("Arg[1] is: %s\n", arg1);

  // 2nd Argument
  tok = strtok(NULL, " ");
  char arg2[512];
  strcpy(arg2, tok);
  arg2[strlen(tok)] = '\0';
  printf("Arg[2] is: %s\n", arg2);

  // 3rd Argument (optional)
  tok = strtok(NULL, " ");
  char arg3[512] = "\0";

  // If the 3rd argument exists, extract the arg
  if (tok != NULL) {
    strcpy(arg3, tok);
    arg3[strlen(tok)] = '\0';
    printf("Arg[3] is: %s\n", arg3);
  }

  // If the 3rd argument does not exist,
  // copy content of arg2
  else {
    strcpy(arg3, arg2);
    arg3[strlen(arg2)] = '\0';
    printf("Arg[3]is: %s\n", arg3);
  }

  // Send acknowledgement
  if (send(client_sock, "READY", strlen("READY"), 0) < 0) {
    printf("Can't send\n");
  }

  // WRITE Command Received
  if (strcmp(arg1, "WRITE") == 0) {

    // Get actual data from client
    char clientData[BUFFER_SIZE];
    memset(clientData, '\0', sizeof(clientData));
    if (recv(client_sock, clientData, 
             sizeof(clientData), 0) < 0){
      printf("Couldn't receive\n");
    }

    // Execute command
    serverSaveFile(arg3, clientData);
  }

  // GET Command Received
  else if (strcmp(arg1, "GET") == 0) {

    // Get response from client
    char client_response[BUFFER_SIZE];
    memset(client_response, '\0', sizeof(client_response));
    if (recv(client_sock, client_response, 
             sizeof(client_response), 0) < 0){
      printf("Couldn't receive\n");
    }

    // Client response
    printf("Client Response is: %s\n", client_response);

    if (strcmp(client_response, "CLIENT_READY") == 0) {
      printf("GET command received.\n");

      printf("arg2 is: %s\n", arg2);

      // Execute command upon client's READY message
      serverSendFile(client_sock, arg2);
    }

  }

  // REMOVE Command Received
  else if (strcmp(arg1, "RM") == 0) {
    serverRemove(client_sock, arg2);
  }

  // LS Command Received
  else if (strcmp(arg1, "LS") == 0) {
    serverList(client_sock, arg2);
  }

  // If command is not valid, stop and send message
  else {
    printf("Invalid command: %s. Exiting...\n", arg1);
    // printf("strcmp WRITE: %d\n", strcmp("WRITE", arg1));
    // printf("strcmp GET: %d\n", strcmp("GET", arg1));
    // printf("strcmp RM: %d\n", strcmp("RM", arg1));
    // printf("strcmp LS: %d\n", strcmp("LS", arg1));
    
    // Send message to client
    if (send(client_sock, "Invalid Command.", strlen("Invalid Command."), 0) < 0) {
      printf("Error\n");
    }
  }

  // Unlock the execution mutex
  pthread_mutex_unlock(&execution_lock);

  // Free thread arguments
  free(client_args);

  // Close connection and exit thread when done
  close(client_sock);
  pthread_exit(NULL);
}
