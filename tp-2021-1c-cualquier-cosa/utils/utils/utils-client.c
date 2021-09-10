/*
 * conexiones.c
 *
 *  Created on: 2 mar. 2019
 *      Author: utnso
 */

#include "utils-client.h"

int crear_conexion_cliente(char *ip, char* puerto) {
	struct addrinfo hints, *server_info;
	int socket_cliente;
	int error;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;	// cualquier protocolo
	//hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_NUMERICHOST;

	if ((error = getaddrinfo(ip, puerto, &hints, &server_info)) != 0) {
		freeaddrinfo(server_info);
		return -1;
	}

	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	
	if(socket_cliente == -1) {
		freeaddrinfo(server_info);
		return -2;
	}
	
	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
		freeaddrinfo(server_info);
		return -3;
	}
	
	freeaddrinfo(server_info);
	return socket_cliente;
}