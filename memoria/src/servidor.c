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
		pthread_create(&nuevo_carpincho, NULL, rutina_carpincho, (void *)info_carpincho);

		log_info(logger, "Comunico al caprincho %d el nuevo puerto con el cual se debe comunicar.", id);
		// Comunico al caprincho el nuevo puerto con el cual se debe comunicar
		t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
		agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(info_carpincho->socket));
		enviar_mensaje(fd_carpincho, mensaje_out);

		// Elimino la conexión auxiliar con el carpincho
		close(fd_carpincho);
		id++;

		// para testear o debuggear en un hilo
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

	// Envío información protocolar (modo de asignación física o global)
	// TODO
	return true;
}
