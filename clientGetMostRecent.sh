#!/bin/bash

# Client WRITE to server repeatedly to get copies, no remote address specified
# 1 original file, 9 new versions
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt
./rfs WRITE local/localGetData.txt

# Client GET the most recent version of file
./rfs GET local/localGetData.txt

# Client GET version 4 of the file.
# Much better result if the file data is manually changed
./rfs GET local/localGetData_new4.txt

wait