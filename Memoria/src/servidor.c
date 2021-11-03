#include "servidor.h"

void iniciar_servidor(char *ip, int puerto) {
	// Creo socket de escucha para recibir los carpinchos
	int server_fd = crear_conexion_servidor(ip,	puerto, SOMAXCONN);
	if(server_fd < 0) {
		return;
	}
	// log_info(logger, "Servidor listo");
	bool seguir = true;

	int fd_carpincho;
	while(seguir) {
		// Espero a que llegue un nuevo carpincho
		fd_carpincho = esperar_cliente(server_fd);
		if(fd_carpincho < 0) {
			seguir = false;
			continue;
		}
		// puede_iniciar() hay_memoria_suficiente()
		// Creo un hilo para que el carpincho se comunique de forma particular
		pthread_t nuevo_carpincho;
		data_carpincho *info_carpincho = malloc(sizeof(data_carpincho));

		// Para la comunicación, creo un nuevo servidor en un puerto libre que asigne el SO
		info_carpincho->socket = crear_conexion_servidor(ip, 0, 1);
		pthread_create(&nuevo_carpincho, NULL, rutina_carpincho, (void *)info_carpincho);

		// Comunico al caprincho el nuevo puerto con el cual se debe comunicar
		t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
		agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(info_carpincho->socket));
		enviar_mensaje(fd_carpincho, mensaje_out);

		// Elimino la conexión auxiliar con el carpincho
		close(fd_carpincho);
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

