#include "kernel.h"

int main() {
	logger = log_create("discordiador.log", "DISCORDIADOR", 1, LOG_LEVEL_INFO);
	config = config_create("discordiador.config");

	//lectura de las configuraciones
	ip_kernel 					= config_get_string_value(config, "IP_KERNEL");
	ip_memoria 					= config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria 				= config_get_string_value(config, "PUERTO_MEMORIA");

	algoritmo_planificacion 	= config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	grado_multiprogramacion 	= config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
	grado_multiprocesamiento 	= config_get_int_value(config, "GRADO_MULTIPROCESAMIENTO");
	alfa 						= config_get_int_value(config, "ALFA");
	estimacion_inicial 			= config_get_int_value(config, "ESTIMACION_INICIAL");

	//creacion de las colas de planificacion
	cola_new_carpinchos = queue_create();

	//creacion del socket por el cual me van a llegar todos los mensajes
	socket_kernel = crear_conexion_servidor(ip_kernel, config_get_int_value(config, "PUERTO_ESCUCHA"), 1);

	if(!validar_socket(socket_kernel, logger)) {
		close(socket_kernel);
		log_destroy(logger);
		return -1;
	}

	log_info(logger, "Kernel listo");

	bool seguir = true;

	//proceso de recepcion de todos los mate_init
	while(seguir) {
		int socket_auxiliar_carpincho = esperar_cliente(socket_kernel); // Espero a que llegue un nuevo carpincho

		if(socket_auxiliar_carpincho < 0) {
			log_error(logger, "Error en el socket recibido del carpincho que intento conectar");
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
				socket_memoria_carpincho = crear_conexion_cliente(ip_memoria, (int)list_get(mensaje_in, 1));
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

			queue_push(cola_new_carpinchos, nuevo_carpincho);
		}
	}

}
