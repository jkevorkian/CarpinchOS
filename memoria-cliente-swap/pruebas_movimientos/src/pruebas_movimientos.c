#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <utils/sockets.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define IP_RAM "127.0.0.1"

char* leer_consola() {
	char* buffer = readline(">");
	if(buffer)
		add_history(buffer);

	return buffer;
}

int main(int argc, char *argv[])
{
	t_log* logger = log_create("pruebas_memoria.log", "I'm Batman", 1, LOG_LEVEL_INFO);
	// t_config* config = config_create("discordiador.config");

	char* ip_ram = "127.0.0.1";
	char* puerto_ram = "9090";

	int socket_ram = crear_conexion_cliente(ip_ram,	puerto_ram);

	if(!validar_socket(socket_ram, logger)) {
		close(socket_ram);
		log_destroy(logger);
		return 0;
	}

	log_info(logger, "Hola. Un puerto, por favor");
	t_mensaje* mensaje_activacion_out = crear_mensaje(MATE_INIT);
	enviar_mensaje(socket_ram, mensaje_activacion_out);
	t_list* mensaje_activacion_in = recibir_mensaje(socket_ram);
	
	int socket;
	if((int)list_get(mensaje_activacion_in, 0) == SEND_PORT) {
		log_info(logger, "Recibí un puerto!");
		char puerto[7];
		sprintf(puerto, "%d", (int)list_get(mensaje_activacion_in, 1));

		socket = crear_conexion_cliente(ip_ram, puerto);
	} else {
		log_error(logger, "No se pudo establecer conexión con la RAM");
		exit(1);
	}
		
	liberar_mensaje_out(mensaje_activacion_out);
	liberar_mensaje_in(mensaje_activacion_in);

	log_info(logger, "Me pude conectar exitosamente con la RAM");
	int seguir = 1;

	t_mensaje* mensaje_out;
	t_list* mensaje_in;
	char *buffer;
	char pagina_generica[32] = "AAAAA_____AAAAA_____AAAAA_____A\0";
	data_socket(socket, logger);
	
	while(seguir) {
		char* buffer_consola = leer_consola();
		char** input = string_split(buffer_consola, " ");
		switch(atoi(input[0])) {
		case NEW_PAGE:	// 11
			log_info(logger, "Voy a asignar una nueva pagina");

			mensaje_out = crear_mensaje(NEW_PAGE);
			agregar_a_mensaje(mensaje_out, "%d%d", atoi(input[1]), atoi(input[2]));
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "Recibo un %d", (int)list_get(mensaje_in, 0));
			break;
		case GET_PAGE:	// 12
			log_info(logger, "Voy a obtener una pagina");

			mensaje_out = crear_mensaje(GET_PAGE);
			agregar_a_mensaje(mensaje_out, "%d%d", atoi(input[1]), atoi(input[2]));
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			if((int)list_get(mensaje_in, 0) == DATA_CHAR)
				log_info(logger, "Data: %s", (char *)list_get(mensaje_in, 1));
			else
				log_warning(logger, "Se rompio todo. Aborten");
			break;
		case SET_PAGE:	// 13
			log_info(logger, "Voy a escribir una pagina");

			buffer = malloc(32);
			memcpy(buffer, pagina_generica, 32);

			mensaje_out = crear_mensaje(SET_PAGE);
			agregar_a_mensaje(mensaje_out, "%d%d%s", atoi(input[1]), atoi(input[2]), buffer);
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "Recibo un %d", (int)list_get(mensaje_in, 0));
			break;
		case RM_PAGE:	// 14
			log_info(logger, "Voy a remover una pagina");

			mensaje_out = crear_mensaje(RM_PAGE);
			agregar_a_mensaje(mensaje_out, "%d", atoi(input[1]));
			enviar_mensaje(socket, mensaje_out);

			mensaje_in = recibir_mensaje(socket);
			log_info(logger, "Recibo un %d", (int)list_get(mensaje_in, 0));
			break;
		case EXIT_C: // 15
			log_info(logger, "Voy a matar al carpincho %d", atoi(input[1]));

			mensaje_out = crear_mensaje(EXIT_C);
			agregar_a_mensaje(mensaje_out, "%d", atoi(input[1]));
			enviar_mensaje(socket, mensaje_out);
			seguir = false;
			break;
		default:
			log_info(logger, "Acción desconocida");
			break;
		}
	}
	log_info(logger, "Let's die. See you on next world.");
	exit(1);
}
