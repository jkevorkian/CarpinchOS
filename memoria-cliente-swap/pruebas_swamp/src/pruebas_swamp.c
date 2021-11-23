#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <utils/sockets.h>
#include <readline/readline.h>
#include <readline/history.h>

#define IP_RAM "127.0.0.1"

int main(int argc, char *argv[])
{
	t_log* logger = log_create("servidor.log", "SERVIDOR", 1, LOG_LEVEL_DEBUG);
	int server_fd = crear_conexion_servidor(IP_RAM, 9999, 1);
	
	if(!validar_socket(server_fd, logger)) {
		log_destroy(logger);
		close(server_fd);
		return 0;
	}
	data_socket(server_fd, logger);

	log_info(logger, "SWAMP lista para recibir al cliente");
	int socket = esperar_cliente(server_fd);
	
	log_info(logger, "Ha llegado un nuevo cliente");
	data_socket(socket, logger);

	char pagina_generica[32] = "Yo soy la loz del mundo, no hay";

	t_list* mensaje_in;
	t_mensaje* mensaje_out;
	
	while(true) {
		mensaje_in = recibir_mensaje(socket);
		if(!validar_mensaje(mensaje_in, (void *)logger))
			break;

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case NEW_PAGE:	// 11
			log_info(logger, "Memoria me pidio una nueva pagina");

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envio OK a memoria");
			break;
		case GET_PAGE:	// 12
			log_info(logger, "Memoria me pidio obtener la pagina %d del carpincho %d", (uint32_t)list_get(mensaje_in, 2), (uint32_t)list_get(mensaje_in, 1));
			log_info(logger, "El largo de la pagina es %d", strlen(pagina_generica));
			log_info(logger, "El contenido es: %s", pagina_generica);

			mensaje_out = crear_mensaje(DATA_CHAR);
			agregar_a_mensaje(mensaje_out, "%s", pagina_generica);
			enviar_mensaje(socket, mensaje_out);
			break;
		case SET_PAGE:	// 13
			log_info(logger, "Memoria me pidio actualizar la pagina % del carpincho %d", (uint32_t)list_get(mensaje_in, 2), (uint32_t)list_get(mensaje_in, 1));
			log_info(logger, "El nuevo contenido es: %s", (char *)list_get(mensaje_in, 3));

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case RM_PAGE:	// 14
			log_info(logger, "Memoria me pidio eliminar una pagina del carpincho %d", (int)list_get(mensaje_in, 1));

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case EXIT_C: // 15
			log_info(logger, "Memoria me pidio matar al carpincho %d", (int)list_get(mensaje_in, 1));

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		default:
			log_info(logger, "Acción desconocida");
			break;
		}
	}
	log_info(logger, "Let's die. See you on next world.");
	exit(1);
}
