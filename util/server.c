// Copyright (C) 2017  Maxr1998
// For more information, view the LICENSE at the root of this project

#include "server.h"
#include <stdio.h>      // printf
#include <stdlib.h>     // malloc, free
#include <string.h>     // str*

#include <sys/socket.h> // sockets
#include <arpa/inet.h>  // inet_addr
#include <unistd.h>     // close

#define MESSAGE_LENGTH 1024

char *start_server() {
	int socket_fd, accept_fd;
	struct sockaddr_in server, client;
	int client_len;
	fd_set readset, tempset;
	struct timeval tv;
	char *client_message = (char*) malloc(MESSAGE_LENGTH), *code = NULL;

	// Create socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		return 0;
	}

	// Setup port
	server.sin_family = AF_INET;
	server.sin_port = htons(8567);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	int yes=1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		printf("setsockopt error\n");
		goto CLOSE;
	}

	// Bind
	if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) < 0) {
		printf("Bind error\n");
		goto CLOSE;
	}

	// Listen for connections
	if (listen(socket_fd, 1) < 0) {
		printf("Listen error\n");
		goto CLOSE;
	}

	// Initialize the set
	FD_ZERO(&readset);
	FD_SET(socket_fd, &readset);

	memcpy(&tempset, &readset, sizeof(tempset));

	// Initialize time out struct
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	// select()
	if (select(socket_fd + 1, &tempset, NULL, NULL, &tv) <= 0) {
		printf("Select error or timeout\n");
		goto CLOSE;
	}

	if (!FD_ISSET(socket_fd, &tempset)) {
		printf("Error\n");
		goto CLOSE;
	}

	client_len = sizeof(client);
	accept_fd = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &client_len);

	if (accept_fd < 0) {
		printf("Accept error\n");
		goto CLOSE;
	}

	// Receive connect attempt
	if (read(accept_fd, client_message, MESSAGE_LENGTH) < 0) {
		printf("Read error\n");
		goto CLOSE;
	}

	// Handle connect attempt
	char *message = "HTTP/1.1 200 OK\n\nSuccess. You can now close this window.\n";
	if(write(accept_fd, message, strlen(message)) < 0) {
		goto CLOSE;
	}

CLOSE:
	if (accept_fd > 0) {
		close(accept_fd);
	}
	if (socket_fd > 0) {
		close(socket_fd);
	}

	// Extract code
	char *tcode = strchr(client_message, '=') + 1;
	*(strchr(tcode, ' ')) = '\0';
	code = malloc(strlen(tcode) * sizeof(char));
	strcpy(code, tcode);
	free(client_message);

	// Save token to storage
	return code;
}