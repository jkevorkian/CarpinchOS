#include "mateLib.h"

t_log *logger;
int id_proxima_instancia = 0;
//---------------------------FUNCIONES GENERALES----------------------

int mate_init(mate_instance *lib_ref, char *config)
{
	//Asigno un espacio de memoria a la referencia a un mate_instance que me pasa por parametro el carpincho

	t_config *mateConfig = config_create(config); //"../mateLib/mateLib.config"

	logger = log_create("mateLib.log", "MATELIB", 1, LOG_LEVEL_INFO);

	char *ip_kernel, *ip_memoria, *puerto_kernel, *puerto_memoria;

	ip_kernel 					= config_get_string_value(mateConfig, "IP_KERNEL");
	ip_memoria 					= config_get_string_value(mateConfig, "IP_MEMORIA");
	puerto_kernel 				= config_get_string_value(mateConfig, "PUERTO_KERNEL");
	puerto_memoria 				= config_get_string_value(mateConfig, "PUERTO_MEMORIA");

	//intento conectar al socket público por default del kernel
	int socket_auxiliar = crear_conexion_cliente(ip_kernel, puerto_kernel); //todavia son un define en el mateLib.h

	//este bool hace que se pueda intentar conectar a la memoria la lib CUANDO (a partir de que) FALLA EL KERNEL
	bool fallo_kernel = true;

	if (validar_socket(socket_auxiliar, logger))
	{
		log_info(logger, "Socket auxiliar funcionando");

		//espera el mensaje de handshake entre el kernel y la lib, en el que el kernel tiene que enviar el nuevo puerto de conexión para que el carpincho se comunique
		t_list *mensaje_in = recibir_mensaje(socket_auxiliar);

		printf("recibido un %d", (int)list_get(mensaje_in, 0));

		//si el mensaje del handshake es el adecuado, se conecta al kernel ahora por el puerto recibido
		if ((int)list_get(mensaje_in, 0) == SEND_PORT)
		{
			fallo_kernel = false;

			//las siguientes 2 lineas están para castear el puerto que llega como string a un int
			char puerto[7];
			sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

			lib_ref->socket = crear_conexion_cliente(ip_kernel, puerto);
			data_socket(lib_ref->socket, logger);
		}
		else
		{
			log_error(logger, "Error en la comunicacion");
			return 1;
		}

		liberar_mensaje_in(mensaje_in);
		close(socket_auxiliar);
	}

	if (fallo_kernel)
	{
		int socket_auxiliar = crear_conexion_cliente(ip_memoria,
													 puerto_memoria); //todavia son un define en el mateLib.h

		if (validar_socket(socket_auxiliar, logger))
		{
			log_info(logger, "Socket auxiliar funcionando");

			t_list *mensaje_in = recibir_mensaje(socket_auxiliar);

			if ((int)list_get(mensaje_in, 0) == SEND_PORT)
			{ //el mensaje puede que sea SND_PO (por lo que vi en iniciar_servidor(), en servidor.c en la memoria, en rama-pato) pero como no aparece dentro de cod_op no lo  puse.
				log_info(logger, "Puerto recibido");

				char puerto[7];
				sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

				lib_ref->socket = crear_conexion_cliente(ip_memoria, puerto);
				data_socket(lib_ref->socket, logger);
			}
			else
			{
				log_error(logger, "Error en la comunicacion");
				return 1;
			}

			liberar_mensaje_in(mensaje_in);
			close(socket_auxiliar);
		}
	}

	lib_ref->id = id_proxima_instancia;
	lib_ref->murio = false;
	id_proxima_instancia++;

	return 0;
}

int mate_close(mate_instance *lib_ref) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(MATE_CLOSE);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);
		lib_ref->murio = true;
		log_info(logger, "Carpincho %d: mate_close realizado", lib_ref->id);
	}
	return 0;
}

//---------------------------FUNCIONES DE KERNEL----------------------

int mate_call_io(mate_instance *lib_ref, char *io, void *msg) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(CALL_IO);
		agregar_a_mensaje(mensaje_out, "%s", io); //agregamos el dispositivo de io al cual llama pero no hacemos nada con el mensaje
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);
		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if((int)list_get(mensaje_in, 0) == TODOOK)
			log_info(logger, "Carpincho %d: volvio de IO correctamente", lib_ref->id);
		else {
			log_error(logger, "Carpincho %d: hubo un fallo al realizar IO", lib_ref->id);
			lib_ref->murio = true;
		}

		liberar_mensaje_in(mensaje_in);
	}

	return 0;
}

int mate_sem_init(mate_instance *lib_ref, char *sem, unsigned int value) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(SEM_INIT);
		agregar_a_mensaje(mensaje_out, "%s%d", sem, value);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK) {
			log_info(logger, "Carpincho %d: La inicializacion del semaforo fue exitosa", lib_ref->id);
		} else {
			log_error(logger, "Carpincho %d: Error en la inicializacion del semaforo", lib_ref->id);
			lib_ref->murio = true;
		}

		liberar_mensaje_in(mensaje_in);
	}

	return 0;
}

int mate_sem_wait(mate_instance *lib_ref, char *sem) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(SEM_WAIT);
		agregar_a_mensaje(mensaje_out, "%s", sem);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK)
			log_info(logger, "Carpincho %d: El wait del semaforo fue exitoso", lib_ref->id);
		else {
			log_error(logger, "Carpincho %d: Error en el wait del semaforo", lib_ref->id);
			lib_ref->murio = true;
		}

		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

int mate_sem_post(mate_instance *lib_ref, char *sem) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(SEM_POST);
		agregar_a_mensaje(mensaje_out, "%s", sem);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK)
			log_info(logger, "Carpincho %d: El post del semaforo fue exitoso", lib_ref->id);
		else {
			log_error(logger, "Carpincho %d: Error en el post del semaforo", lib_ref->id);
			lib_ref->murio = true;
		}
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

int mate_sem_destroy(mate_instance *lib_ref, char *sem) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(SEM_DESTROY);
		agregar_a_mensaje(mensaje_out, "%s", sem);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK)
			log_info(logger, "Carpincho %d: El cierre del semaforo fue exitoso", lib_ref->id);
		else {
			log_error(logger, "Carpincho %d: Error en cierre del semaforo", lib_ref->id);
			lib_ref->murio = true;
		}
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

//---------------------------FUNCIONES DE MEMORIA RAM----------------------

mate_pointer mate_memalloc(mate_instance *lib_ref, int size) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(MEM_ALLOC);
		agregar_a_mensaje(mensaje_out, "%d", size);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if((int)list_get(mensaje_in, 0) == DATA_INT) {
			log_info(logger, "Carpincho %d: La memoria fue alocada correctamente en la posicion %d", lib_ref->id, (int)list_get(mensaje_in, 1));
			return (int)list_get(mensaje_in, 1);
		} else {
			if((int)list_get(mensaje_in, 0) == NO_MEMORY)
				log_error(logger, "Carpincho %d: Error al alocar la memoria, no hay mas espacio", lib_ref->id);
			else
				log_error(logger, "Carpincho %d: Fallo en la comunicacion (Codigo de error: %s)", lib_ref->id, string_desde_mensaje((int)list_get(mensaje_in, 0)));

			lib_ref->murio = true;
		}
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(MEM_FREE);
		agregar_a_mensaje(mensaje_out, "%d", addr);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK)
			log_info(logger, "Carpincho %d: La memoria fue liberada correctamente", lib_ref->id);
		else {
			log_error(logger, "Carpincho %d: Error al liberar la memoria (Codigo de error: %s - %d)", lib_ref->id, string_desde_mensaje((int)list_get(mensaje_in, 0)), (int)list_get(mensaje_in, 1));
			lib_ref->murio = true;
		}
		
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

//Se decidió retornar en "void* dest" lo que se encuentre en la dirección de memoria "mate_pointer origin"
int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(MEM_READ);
		agregar_a_mensaje(mensaje_out, "%d%d", origin, size);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == DATA_PAGE) {
			log_info(logger, "Carpincho %d: La memoria fue leida correctamente", lib_ref->id);
			memcpy(dest, list_get(mensaje_in, 2), size);
			data_bloque(list_get(mensaje_in, 2), (int)list_get(mensaje_in, 1));
		} else {
			log_error(logger, "Carpincho %d: Error al leer la memoria (Codigo de error: %s - %d)", lib_ref->id, string_desde_mensaje((int)list_get(mensaje_in, 0)), (int)list_get(mensaje_in, 1));
			lib_ref->murio = true;
		}
		
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

//se decidió escribir lo que sea que apunte "void* origin" enla direccion apuntada por "mate_pointer dest"
int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size) {
	if(!lib_ref->murio) {
		t_mensaje *mensaje_out = crear_mensaje(MEM_WRITE);
		agregar_a_mensaje(mensaje_out, "%d%sd", dest, size, origin);
		enviar_mensaje(lib_ref->socket, mensaje_out);
		liberar_mensaje_out(mensaje_out);

		t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

		if ((int)list_get(mensaje_in, 0) == TODOOK) {
			log_info(logger, "Carpincho %d: La memoria fue escrita correctamente", lib_ref->id);
		} else {
			log_error(logger, "Carpincho %d: Error al escribir la memoria (Codigo de error: %s - %d)", lib_ref->id, string_desde_mensaje((int)list_get(mensaje_in, 0)), (int)list_get(mensaje_in, 1));
			lib_ref->murio = true;
		}
		liberar_mensaje_in(mensaje_in);
	}
	return 0;
}

void data_bloque(void *data, uint32_t tamanio) {
	// log_info(logger, "Contenido data:");
	printf("Contenido data: ");
	uint8_t byte;
	for(int i = 0; i < tamanio; i++) {
		memcpy(&byte, data + i, 1);
		printf("%3d|", byte);
	}
	printf("\n");
}
