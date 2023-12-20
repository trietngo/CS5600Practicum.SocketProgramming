#!/bin/bash

# Test 1: Client GET to server
./rfs GET remoteRepoData.txt local/localReceivedData.txt

# Test 2: Client GET to server without local file specified
./rfs GET remoteRepoData.txt

wait