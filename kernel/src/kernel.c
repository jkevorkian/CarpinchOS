#include "kernel.h"

int main(int argc, char *argv[]) {
	char* direccion_config;

	if(argc < 2)
		direccion_config = "kernel.config";
	else
		direccion_config = argv[1];

	if(inicializar_kernel(direccion_config)) {
		log_error(logger, "FALLO AL INICIAR EL KERNEL");
		exit(1);
	}

	bool seguir = true;

	//proceso de recepcion de los mate_init
	while(seguir) {
		log_info(logger, "Kernel esperando algun carpincho");

		int socket_auxiliar_carpincho = esperar_cliente(socket_kernel); // Espero a que llegue un nuevo carpincho

		if(socket_auxiliar_carpincho < 0) {
			log_error(logger, "Error en el socket recibido del carpincho que se intento conectar");
			seguir = false;
		}
		else {
			log_warning(logger, "Se ha conectado un carpincho (%d)", id_proximo_carpincho);

			//creo la estructura para el nuevo carpincho
			carpincho* nuevo_carpincho = malloc(sizeof(carpincho));

			nuevo_carpincho->socket_mateLib = crear_socket_carpincho(socket_auxiliar_carpincho);
			nuevo_carpincho->socket_memoria = conectar_memoria();
			nuevo_carpincho->rafaga_real_anterior = 0;
			nuevo_carpincho->estimacion_proxima_rafaga = estimacion_inicial;
			nuevo_carpincho->id = id_proximo_carpincho;
			nuevo_carpincho->esta_suspendido = false;
			nuevo_carpincho->responder = false;
			nuevo_carpincho->id_semaforo_bloqueante = -1;
			nuevo_carpincho->debe_morir = false;
			nuevo_carpincho->semaforos_asignados = list_create();
			nuevo_carpincho->esperar_cliente = true;
			nuevo_carpincho->tiempo_llegada = NULL;

			agregar_new(nuevo_carpincho);
			id_proximo_carpincho++;
			mate_init++;
		}
	}

}

int conectar_memoria(int socket_auxiliar_carpincho) {
	int socket_memoria_carpincho = 0;

	if(MEMORIA_ACTIVADA) { //creo una conexion con la memoria para que esta me devuelva el puerto por el cual se comunicara el carpincho
		int socket_auxiliar_memoria = crear_conexion_cliente(ip_memoria, puerto_memoria);

		if(!validar_socket(socket_auxiliar_memoria, logger)) {
			close(socket_auxiliar_memoria);
			log_error(logger, "Error en el socket generado para la memoria");
		}

		t_list* mensaje_in = recibir_mensaje(socket_auxiliar_memoria);

		if ((int)list_get(mensaje_in, 0) == SEND_PORT) {
			char puerto[7];
			sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));
			socket_memoria_carpincho = crear_conexion_cliente(ip_memoria, puerto);
		} else
			log_error(logger, "Error en la recepcion del puerto de la memoria");

		liberar_mensaje_in(mensaje_in);
		close(socket_auxiliar_memoria);
	}

	return socket_memoria_carpincho;
}

int crear_socket_carpincho(int socket_auxiliar_carpincho) {
	//creo un nuevo servidor en un puerto libre que asigne el SO
	int socket_aux_carpincho = crear_conexion_servidor(ip_kernel, 0, SOMAXCONN);

	if(!validar_socket(socket_aux_carpincho, logger)) {
		close(socket_aux_carpincho);
		log_error(logger, "Error al crear un socket para comunicacion exclusiva con el carpincho");
	} else {
		//comunico al carpincho el puerto por el cual me tiene que hablar
		t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
		agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(socket_aux_carpincho));
		enviar_mensaje(socket_auxiliar_carpincho, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		close(socket_auxiliar_carpincho);
	}

	return socket_aux_carpincho;
}
