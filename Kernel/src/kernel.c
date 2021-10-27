#include "kernel.h"

int main() {
	if(!inicializar_kernel())
		return -1;

	bool seguir = true;

	//proceso de recepcion de los mate_init
	while(seguir) {
		int socket_auxiliar_carpincho = esperar_cliente(socket_kernel); // Espero a que llegue un nuevo carpincho

		if(socket_auxiliar_carpincho < 0) {
			log_error(logger, "Error en el socket recibido del carpincho que se intento conectar");
			seguir = false;
		}
		else {
			log_info(logger, "Se ha conectado un carpincho");

			int socket_mate_carpincho;
			int socket_memoria_carpincho;

			//creo un nuevo servidor en un puerto libre que asigne el SO
			socket_mate_carpincho = crear_conexion_servidor(ip_kernel, 0, 1);
			if(!validar_socket(socket_mate_carpincho, logger)) {
				close(socket_mate_carpincho);
				log_destroy(logger);
			}

			//comunico al carpincho el puerto por el cual me tiene que hablar
			t_mensaje* mensaje_out = crear_mensaje(SEND_PORT);
			agregar_a_mensaje(mensaje_out, "%d", puerto_desde_socket(socket_mate_carpincho));
			enviar_mensaje(socket_auxiliar_carpincho, mensaje_out);
			liberar_mensaje_out(mensaje_out);
			close(socket_auxiliar_carpincho);

			//creo una conexion con la memoria para que esta me devuelva el puerto por el cual se comunicara el carpincho
			int socket_auxiliar_memoria = crear_conexion_cliente(ip_memoria, puerto_memoria);
			if(!validar_socket(socket_auxiliar_memoria, logger)) {
				close(socket_auxiliar_memoria);
				log_error(logger, "Error en el socket generado para la memoria");
			}

			t_list* mensaje_in = recibir_mensaje(socket_kernel);

			if ((int)list_get(mensaje_in, 0) == SEND_PORT)
				socket_memoria_carpincho = crear_conexion_cliente(ip_memoria, (char*)list_get(mensaje_in, 1));
			else
				log_error(logger, "Error en la recepcion del puerto de la memoria");

			liberar_mensaje_in(mensaje_in);
			close(socket_auxiliar_memoria);

			//creo la estructura para el nuevo carpincho
			carpincho* nuevo_carpincho = malloc(sizeof(carpincho));

			nuevo_carpincho->socket_mateLib = socket_mate_carpincho;
			nuevo_carpincho->socket_memoria = socket_memoria_carpincho;
			nuevo_carpincho->rafaga_real_anterior = 0;
			nuevo_carpincho->estimacion_proxima_rafaga = estimacion_inicial;

			agregar_new(nuevo_carpincho);
		}
	}

}
