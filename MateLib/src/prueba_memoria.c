#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "mateLib.h"
#include <utils/sockets.h>

int main(int argc, char *argv[])
{
	t_log* logger = log_create("pruebas_memoria.log", "I'm Batman", 1, LOG_LEVEL_INFO);
	// t_config* config = config_create("discordiador.config");

	char* ip_ram = "127.0.0.1";
	int puerto_ram = 9090;

	int socket_ram = crear_conexion_cliente(ip_ram,	puerto_ram);

	if(!validar_socket(socket_ram, logger)) {
		close(socket_ram);
		log_destroy(logger);
		return 0;
	}

	t_mensaje* mensaje_activacion_out = crear_mensaje(MATE_INIT);
	enviar_mensaje(socket_mongo, mensaje_activacion_out);
	t_list* mensaje_activacion_in = recibir_mensaje(socket_mongo);

	if((int)list_get(mensaje_activacion_in, 0) == SEND_PORT) {
		char puerto[7];
		sprintf(puerto, "%d", (int)list_get(mensaje_activacion_in, 1));

		int socket = crear_conexion_cliente(ip_ram, puerto);
	}else
		log_error(logger, "No se pudo establecer conexión con el hilo");

	liberar_mensaje_out(mensaje_activacion_out);
	liberar_mensaje_in(mensaje_activacion_in);
	
	return 0;
}
