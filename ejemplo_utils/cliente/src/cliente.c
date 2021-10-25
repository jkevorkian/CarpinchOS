/*
 ============================================================================
 Name        : discordiador.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "cliente.h"

int main() {
	t_log* logger = log_create("cliente.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	int socket_cliente;
	// char* ip = config_get_string_value(config, "IP_SERVIDOR");
	char* ip = "127.0.0.1";
	// int puerto = config_get_string_value(config, "PUERTO_SERVIDOR");
	// char* puerto = config_get_string_value(config, "PUERTO_SERVIDOR");
	char* puerto = "9008";
	log_info(logger, "IP servidor: %s. Puerto servidor: %s", ip, puerto);
	socket_cliente = crear_conexion_cliente(ip, puerto);
	if(!validar_socket(socket_cliente, logger)) {
		close(socket_cliente);
		log_destroy(logger);
		return ERROR_CONEXION;
	}
	data_socket(socket_cliente, logger);

	t_mensaje* mensaje_out;
	t_list *mensaje_in;

	log_info(logger, "Recibo mensaje");
	mensaje_in = recibir_mensaje(socket_cliente);
	
	log_info(logger, "Paso a loggear la mensaje_in");
	if((int)list_get(mensaje_in, 0) == MEM_ALLOC) {
		log_info(logger, "Largo de lista %d", mensaje_in->elements_count);
		log_info(logger, "Protocolo: %d", (int)list_get(mensaje_in, 0));
		log_info(logger, "Recibi puerto %d", (int)list_get(mensaje_in, 1));
	}
	else
		log_info(logger, "ERROR, NO SE RECIBIÓ NADA");
	
	char tarea1[] = "Soy la tarea 1";

	log_info(logger, "Voy a crear mensaje");
	mensaje_out = crear_mensaje(MEM_WRITE);
	agregar_a_mensaje(mensaje_out, "%d%s", 1, tarea1);
	enviar_mensaje(socket_cliente, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	log_info(logger, "Libero mensaje y termino ejecución");
	return 0;
}
