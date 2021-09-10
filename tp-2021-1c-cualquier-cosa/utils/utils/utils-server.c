/*
 *	utils-server.c
 *
 *	Created on: 1 may. 2021
 *	Author: cualquier-cosa
 */

#include"utils-server.h"

int crear_conexion_servidor(char *ip, int puerto, int cola_escucha) {
	int socket_servidor;
	int error;
	struct addrinfo hints, *servinfo, *addr_aux;
	
	char str_puerto[7];
	sprintf(str_puerto, "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;	// En pricipio es 0 (any)
	//hints.protocol = 0;			// TCP

	if ((error = getaddrinfo(ip, str_puerto, &hints, &servinfo)) != 0) {
		return -1;
	}

	int yes = 1;
	for (addr_aux=servinfo; addr_aux != NULL; addr_aux = addr_aux->ai_next) {
		if ((socket_servidor = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
			continue;
		}
		if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			continue;
		}
		
		if (bind(socket_servidor, addr_aux->ai_addr, addr_aux->ai_addrlen) == -1) {
			close(socket_servidor);
			continue;
		}
		
		break;
	}

	if(addr_aux == NULL) {
		return -2;
	}
	
	listen(socket_servidor, cola_escucha);
	if ((error = listen(socket_servidor, 1)) != 0) {
		return -3;
	}

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	return socket_cliente;
}