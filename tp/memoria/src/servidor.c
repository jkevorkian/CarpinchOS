#include "servidor.h"

void iniciar_servidor(char *ip, int puerto) {
	// Creo socket de escucha para recibir los carpinchos
	int server_fd = crear_conexion_servidor(ip,	puerto, SOMAXCONN);
	if(server_fd < 0) {
		return;
	}
	// log_info(logger, "Servidor listo");
	bool seguir = true;

	int socket_discord;
	while(seguir) {
		socket_discord = esperar_cliente(server_fd);
		if(socket_discord < 0) {
			seguir = false;
			continue;
		}

		pthread_t nuevo_carpincho;
		data_carpincho *info_carpincho = malloc(sizeof(data_carpincho));
		// info_carpincho->socket = ...;
		info_carpincho->kernel = es_cliente_kernel(info_carpincho->socket);

		pthread_create(&nuevo_carpincho, NULL, rutina_carpincho, (void *)info_carpincho);
	}



}

bool es_cliente_kernel(int socket) {
	return true;
}
