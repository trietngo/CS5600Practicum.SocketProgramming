#!/bin/bash

# Test 1: Client WRITE to server
./rfs WRITE local/localData.txt serverRepo/remoteData.txt

# Test 2: Client WRITE to server, no remote address specified
./rfs WRITE local/localData.txt

wait