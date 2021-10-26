#include "kernel.h"

int main() {
	logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	socket_memoria = crear_conexion_cliente(ip_memoria,	config_get_string_value(config, "PUERTO_MEMORIA"));

	if(!validar_socket(socket_memoria, logger)) {
		close(socket_memoria);
		log_destroy(logger);
		return 0;
	}

	algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	grado_multiprocesamiento = config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
	alfa = config_get_int_value(config, "ALFA");
	estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");

	int server_fd = crear_conexion_servidor(config_get_string_value(config, "IP_KERNEL"), config_get_int_value(config, "PUERTO_ESCUCHA"), 1);

	if(!validar_socket(server_fd, logger)) {
		close(server_fd);
		log_destroy(logger);
		return -1;
	}

	log_info(logger, "Kernel listo");

	bool seguir = true;

	while(seguir) {
		int fd_carpincho = esperar_cliente(server_fd); // Espero a que llegue un nuevo carpincho

		if(fd_carpincho < 0)
			seguir = false;
		else {
			log_info(logger, "Se ha conectado un carpincho");

			/*
			// Creo un hilo para que el carpincho se comunique de forma particular
			pthread_t nuevo_carpincho;
			data_carpincho *info_carpincho = malloc(sizeof(data_carpincho));
			info_carpincho->socket = crear_conexion_servidor(ip, 0, 1);
			pthread_create(&nuevo_carpincho, NULL, rutina_carpincho, (void *)info_carpincho);
			*/

			// Para la comunicación, creo un nuevo servidor en un puerto libre que asigne el SO
			int socket_carpincho = crear_conexion_servidor(ip_memoria, 0, 1);

			// Comunico al caprincho el nuevo puerto con el cual se debe comunicar
			t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
			agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(socket_carpincho));
			enviar_mensaje(fd_carpincho, mensaje_out);


			close(fd_carpincho);// cierro la conexión auxiliar con el carpincho
		}
	}

}
