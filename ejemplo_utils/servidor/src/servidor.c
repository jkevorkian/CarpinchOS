#include "servidor.h"

int main(void) {
	t_log* logger = log_create("servidor.log", "SERVIDOR", 1, LOG_LEVEL_DEBUG);
	int server_fd = crear_conexion_servidor(IP_RAM, PUERTO_RAM, 1);
	
	if(!validar_socket(server_fd, logger)) {
		log_destroy(logger);
		close(server_fd);
		return ERROR_CONEXION;
	}
	data_socket(server_fd, logger);

	log_info(logger, "Servidor listo para recibir al cliente");
	int socket_cliente = esperar_cliente(server_fd);
	log_info(logger, "Prueba puerto_desde_socket\nServidor: %d\nCliente: %d", puerto_desde_socket(server_fd), puerto_desde_socket(socket_cliente));
	
	log_info(logger, "Ha llegado un nuevo cliente");
	data_socket(socket_cliente, logger);

	log_info(logger, "Envío mensaje_out");
	t_mensaje* mensaje_out = crear_mensaje(SND_PO);
	agregar_a_mensaje(mensaje_out, "%d",4040);
	enviar_mensaje(socket_cliente, mensaje_out);

	log_info(logger, "Entro a recibir mensaje");
	t_list* mensaje_in = recibir_mensaje(socket_cliente);
	
	log_info(logger, "Largo de lista %d", mensaje_in->elements_count);
	log_info(logger, "Protocolo: %d", (int)list_get(mensaje_in, 0));
	log_info(logger, "Recibi id_ patota %d", (int)list_get(mensaje_in, 1));
	log_info(logger, "Recibi cant_tripulantes %d", (int)list_get(mensaje_in, 2));
	log_info(logger, "Tarea 1: %s", (char *)list_get(mensaje_in, 3));
	log_info(logger, "Tarea 2: %s", (char *)list_get(mensaje_in, 4));

	liberar_mensaje_out(mensaje_out);
	
	return EXIT_SUCCESS;
}
