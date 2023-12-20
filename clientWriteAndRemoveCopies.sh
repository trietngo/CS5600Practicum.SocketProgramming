#!/bin/bash

# Client WRITE to server repeatedly to get copies, no remote address specified.
# Running this script should produce nothing
# 1 original file, 9 new versions
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt
./rfs WRITE local/localData.txt

# Client RM the file and all versions
./rfs RM local/localData.txt

wait