#include "sockets.h"

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

int puerto_desde_socket(int socket) {
	struct sockaddr_in dir_cliente;
	socklen_t largo = sizeof(struct addrinfo);
	
	getsockname(socket, (struct sockaddr *)&dir_cliente, &largo);
	return ntohs(dir_cliente.sin_port);
}

bool validar_socket(int socket, t_log* logger) {
    switch (socket){
    case -1:
        log_error(logger, "Error al crear socket. La conexión falló en getaddrinfo");
        break;
    case -2:
        log_error(logger, "Error al crear socket. Puerto no disponible");
        break;
	case -3:
        log_error(logger, "Error al crear socket. Fallo al conectarse al servidor");
        break;
    default:
        return true;
    }
    return false;
}

void data_socket(int socket, t_log* logger) {
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in dir_cliente;
	socklen_t largo = sizeof(struct addrinfo);
	
	if(getsockname(socket, (struct sockaddr *)&dir_cliente, &largo) == 0) {
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		log_info(logger, "Dirección local: %s:%d", ipstr, ntohs(dir_cliente.sin_port));
	}
	else {
		log_warning(logger, "El socket no está iniciado");
	}
	if(getpeername(socket, (struct sockaddr *)&dir_cliente, &largo) == 0) {
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		log_info(logger, "Dirección remota: %s:%d", ipstr, ntohs(dir_cliente.sin_port));
	}
	else {
		log_warning(logger, "El socket no está conectado a un socket remoto");
	}
}
