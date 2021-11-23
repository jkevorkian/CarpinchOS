#include "servidor.h"

void iniciar_servidor(char *ip, int puerto) {
	log_info(logger, "Creo socket de escucha para recibir los carpinchos");
	// Creo socket de escucha para recibir los carpinchos
	int server_fd = crear_conexion_servidor(ip,	puerto, SOMAXCONN);
	if(server_fd < 0) {
		return;
	}
	log_info(logger, "Servidor listo");
	bool seguir = true;

	int id = 1;
	int fd_carpincho;
	while(seguir) {
		log_info(logger, "Espero a que llegue un nuevo carpincho");
		fd_carpincho = esperar_cliente(server_fd);
		if(fd_carpincho < 0) {
			log_info(logger, "Muero esperando cliente");
			seguir = false;
			continue;
		}

		log_info(logger, "Creo un nuevo carpincho");
		crear_carpincho(id);

		// Creo un hilo para que el carpincho se comunique de forma particular
		pthread_t nuevo_carpincho;
		data_carpincho *info_carpincho = malloc(sizeof(data_carpincho));

		// Para la comunicación, creo un nuevo servidor en un puerto libre que asigne el SO
		info_carpincho->socket = crear_conexion_servidor(ip, 0, 1);
		data_socket(info_carpincho->socket, logger);
		info_carpincho->id = id;
		// Original
		// pthread_create(&nuevo_carpincho, NULL, rutina_carpincho, (void *)info_carpincho);

		// Test
		pthread_create(&nuevo_carpincho, NULL, rutina_creador_movimientos, (void *)info_carpincho);

		log_info(logger, "Comunico al caprincho %d el nuevo puerto con el cual se debe comunicar.", id);
		// Comunico al caprincho el nuevo puerto con el cual se debe comunicar
		t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
		agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(info_carpincho->socket));
		enviar_mensaje(fd_carpincho, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		// Elimino la conexión auxiliar con el carpincho
		close(fd_carpincho);
		id++;

		// rutina_test_carpincho(info_carpincho);
		// exit(1);
	}
}

bool iniciar_swap(char *ip_swap, char *puerto_swap) {
	// Obtengo un socket para comunicarme con la swap
	int socket_swap = crear_conexion_cliente(ip_swap, puerto_swap);
	// Creo un hilo para reslover las solicitudes de swap de los carpinchos
	pthread_t cliente_swap;
	pthread_create(&cliente_swap, NULL, manejar_swap, (void *)socket_swap);

	// manejar_swap((void *)socket_swap);
	log_info(logger, "Salgo de iniciar_swap");
	return true;
}

void *rutina_creador_movimientos(void *pepe) {
	log_info(logger, "Nace un nuevo carpincho");
	bool seguir = true;
	data_carpincho* carpincho = (data_carpincho *)pepe;
	int socket = esperar_cliente(carpincho->socket);
	close(carpincho->socket);
	data_socket(socket, logger);

	t_list *mensaje_in;
	t_mensaje* mensaje_out;

	bool respuesta;
	char *buffer;
	while(seguir) {
		log_info(logger, "Espero un nuevo mensaje");
		mensaje_in = recibir_mensaje(socket);

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case NEW_PAGE:
			log_info(logger, "Me llego un new_page");
			respuesta = crear_movimiento_swap(NEW_PAGE, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), NULL);
			log_info(logger, "Respuesta: %d", respuesta);
			if(respuesta)
				mensaje_out = crear_mensaje(TODOOK);
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envío respuesta");
			break;
		case GET_PAGE:
			log_info(logger, "Me llego un get_page");
			buffer = malloc(config_memoria.tamanio_pagina);
			
			respuesta = crear_movimiento_swap(GET_PAGE, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), buffer);
			log_info(logger, "Respuesta: %d", respuesta);
			if(buffer) {
				log_info(logger, "Data: %s", buffer);
				mensaje_out = crear_mensaje(DATA_CHAR);
				agregar_a_mensaje(mensaje_out, "%s", buffer);
			}
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			free(buffer);
			break;
		case SET_PAGE:
			log_info(logger, "Me llego un set_page");

			respuesta = crear_movimiento_swap(SET_PAGE, (int)list_get(mensaje_in, 1), (int)list_get(mensaje_in, 2), (char *)list_get(mensaje_in, 3));
			log_info(logger, "Respuesta: %d", respuesta);
			if(respuesta)
				mensaje_out = crear_mensaje(TODOOK);
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envío respuesta");
			break;
		case RM_PAGE:
			log_info(logger, "Me llego un rm_page");
			
			respuesta = crear_movimiento_swap(RM_PAGE, (int)list_get(mensaje_in, 1), 0, NULL);
			log_info(logger, "Respuesta: %d", respuesta);
			if(respuesta)
				mensaje_out = crear_mensaje(TODOOK);
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envío respuesta");
			break;
		case EXIT_C:
		default:
			seguir = false;
			log_info(logger, "Murio el carpincho, nos vemos.");
			
			respuesta = crear_movimiento_swap(EXIT_C, (int)list_get(mensaje_in, 1), 0, NULL);
			log_info(logger, "Respuesta: %d", respuesta);
			continue;
		}
		liberar_mensaje_out(mensaje_out);
		liberar_mensaje_in(mensaje_in);
	}
	return NULL;
}