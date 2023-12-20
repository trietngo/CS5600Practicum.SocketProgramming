/*
 * clientHelper.h / Practicum II
 *
 * Triet Ngo / CS5600 / Northeastern University
 * Fall 2023 / Dec 5, 2023
 *
 */

#ifndef CLIENT_H
#define CLIENT_H

void clientSendCommand(int socket_desc, const char *command,
                       const char *arg1, const char *arg2);

void clientWriteToServer(int socket_desc, const char *local_path,
                         const char *remote_path);

void clientGetFromServer(const char *remote_path, char *data);

#endif // CLIENT_H
