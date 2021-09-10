#include "utils-sockets.h"

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