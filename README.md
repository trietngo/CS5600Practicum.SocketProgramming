Triet Ngo - Practicum II: Socket Programming

1. Using the OS terminal within the main directory, type "make" to compile the program.

2. To start the server, type "./server" the terminal. Depending on your testing environment, the connection may fail. If so, type "make clean" to clean up the directory then "make" to recompile, then "./server" again to start up the server.

3. To run the client, run "./rfs <COMMAND> <arg1> <arg2>".
  - COMMAND: WRITE, GET, RM, LS
  - arg1, arg2: file paths
  - Note: RM and LS only takes one argument
  - Note: remoteRepo folder is there to simulate a remote directory

4. Commands and their functions:
  - WRITE: write data of a file from <arg1> in client to the remote location <arg2> on the server, or <arg1> if <arg2> is not specified.
  - GET: get the specified data from <arg1> on the remote server and store it in local at <arg2>, or <arg1> if <arg2> is not specified.
  - RM: remove the specified file and all its versions, if applicable at <arg1> on the server
  - LS: return the metadata of the specified file and all its versions, if applicable at <arg1> on the server (NOT FULLY WORKING)

5. To speed up testing, run "bash <bashFile.sh>" files to see how the program runs
  - clientDelete.sh: delete an existing file
  - clientGetMostRecent.sh: create a file and its 9 versions and get the most recent file and the version 4 of the file
  - clientGetServer.sh: get a file from server
  - clientListFiles.sh: get metadata of a file and all its versions (ONLY ON SERVER)
  - clientWriteAndRemoveCopies.sh: write a file and its 9 versions, and immediately delete everything
  - clientWriteServer.sh: write a file to the server

6. To stop server from running, click "Ctrl/Cmd + C" in the terminal

7. Users are free to create new files and tests that are deemed necessary.

Note: Some bugs may persist