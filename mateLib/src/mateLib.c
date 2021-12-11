#include "mateLib.h"

//---------------------------FUNCIONES GENERALES----------------------

int mate_init(mate_instance *lib_ref, char *config)
{

	//Asigno un espacio de memoria a la referencia a un mate_instance que me pasa por parametro el carpincho
	lib_ref = malloc(sizeof(mate_instance));

	t_config *mateConfig = config_create(config);
	logger = log_create("mateLib.log", "MATELIB", 1, LOG_LEVEL_INFO);

	//intento conectar al socket público por default del kernel
	int socket_auxiliar = crear_conexion_cliente(ip_kernel, puerto_kernel); //todavia son un define en el mateLib.h

	//este bool hace que se pueda intentar conectar a la memoria la lib CUANDO (a partir de que) FALLA EL KERNEL
	bool fallo_kernel = true;

	if (validar_socket(socket_auxiliar, logger))
	{
		log_info(logger, "Socket auxiliar funcionando");

		//espera el mensaje de handshake entre el kernel y la lib, en el que el kernel tiene que enviar el nuevo puerto de conexión para que el carpincho se comunique
		t_list *mensaje_in = recibir_mensaje(socket_auxiliar);

		//si el mensaje del handshake es el adecuado, se conecta al kernel ahora por el puerto recibido
		if ((int)list_get(mensaje_in, 0) == SEND_PORT)
		{
			fallo_kernel = false;
			log_info(logger, "Puerto recibido");

			//las siguientes 2 lineas están para castear el puerto que llega como string a un int
			char puerto[7];
			sprintf(puerto, "%d", (int)list_get(mensaje_in, 1));

			lib_ref->socket = crear_conexion_cliente(ip_kernel, puerto);
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
	return 0;
}

int mate_close(mate_instance *lib_ref)
{
	t_mensaje *mensaje_out = crear_mensaje(MATE_CLOSE);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);
	free(lib_ref);
	return 0;
}

//---------------------------FUNCIONES DE KERNEL----------------------

int mate_call_io(mate_instance *lib_ref, char *io, void *msg)
{
	t_mensaje *mensaje_out = crear_mensaje(CALL_IO);
	agregar_a_mensaje(mensaje_out, "%s", io);
	agregar_a_mensaje(mensaje_out, "%s", (char *)msg);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);
	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);
	log_info(logger, "%s", string_desde_mensaje((int)list_get(mensaje_in, 0))); //se imprime el mensaje de IO que llega
	liberar_mensaje_in(mensaje_in);
	return 0;
}

int mate_sem_init(mate_instance *lib_ref, char *sem, unsigned int value)
{
	t_mensaje *mensaje_out = crear_mensaje(SEM_INIT);
	agregar_a_mensaje(mensaje_out, "%s", sem);
	agregar_a_mensaje(mensaje_out, "%d", value);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == TODOOK)
	{
		log_info(logger, "La inicializacion del semaforo fue exitosa");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
	else
	{
		log_error(logger, "Error en la inicializacion del semaforo");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
}

int mate_sem_wait(mate_instance *lib_ref, char *sem)
{
	t_mensaje *mensaje_out = crear_mensaje(SEM_WAIT);
	agregar_a_mensaje(mensaje_out, "%s", sem);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == TODOOK)
	{
		log_info(logger, "La inicializacion del semaforo fue exitosa");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
	else
	{
		log_error(logger, "Error en la inicializacion del semaforo");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
}

int mate_sem_post(mate_instance *lib_ref, char *sem)
{
	t_mensaje *mensaje_out = crear_mensaje(SEM_POST);
	agregar_a_mensaje(mensaje_out, "%s", sem);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == TODOOK)
	{
		log_info(logger, "El post del semaforo fue exitoso");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
	else
	{
		log_error(logger, "Error en el post del semaforo");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
}

int mate_sem_destroy(mate_instance *lib_ref, char *sem)
{
	t_mensaje *mensaje_out = crear_mensaje(SEM_DESTROY);
	agregar_a_mensaje(mensaje_out, "%s", sem);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == TODOOK)
	{
		log_info(logger, "El cierre del semaforo fue exitoso");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
	else
	{
		log_error(logger, "Error en cierre del semaforo");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
}

//---------------------------FUNCIONES DE MEMORIA RAM----------------------

mate_pointer mate_memalloc(mate_instance *lib_ref, int size)
{

	t_mensaje *mensaje_out = crear_mensaje(MEM_ALLOC);
	agregar_a_mensaje(mensaje_out, "%d", size);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);
	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == NO_MEMORY)
	{
		log_error(logger,
				  "Ocurrió un fallo al intentar alocar la memoria. quizá no hay mas memoria disponible?");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
	else
	{
		log_info(logger, "La memoria fue alocada correctamente");
		mate_pointer puntero_auxiliar = (mate_pointer)list_get(mensaje_in, 1); //TODO: no se si está bien casteado el puntero que se retornó adentro del mensaje ni si es la posición correcta dentro de la lista.
		liberar_mensaje_in(mensaje_in);
		return puntero_auxiliar;
	}
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr)
{
	t_mensaje *mensaje_out = crear_mensaje(MEM_FREE);
	agregar_a_mensaje(mensaje_out, "%s", addr);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == SEG_FAULT)
	{
		log_error(logger,
				  "Ocurrió un fallo al intentar liberar la memoria (SEG_FAULT)");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
	else
	{
		log_info(logger, "La memoria fue liberada correctamente");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
}

//Se decidió retornar en "void* dest" lo que se encuentre en la dirección de memoria "mate_pointer origin"
int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size)
{
	t_mensaje *mensaje_out = crear_mensaje(MEM_READ);
	agregar_a_mensaje(mensaje_out, "%s", origin);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == SEG_FAULT)
	{ //TODO: el mensaje que contesta la memoria en caso de que falle el memwrite todavía no está definido, el SEG_FAULT se puso provisoriamente
		log_error(logger,
				  "Ocurrió un fallo al intentar leer en la direccion enviada memoria");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
	else
	{
		log_info(logger, "La memoria fue leida correctamente");
		void *lectura_auxiliar = list_get(mensaje_in, 1);
		liberar_mensaje_in(mensaje_in);
		dest = lectura_auxiliar;
		return 0;
	}
}

//se decidió escribir lo que sea que apunte "void* origin" enla direccion apuntada por "mate_pointer dest"
int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size)
{
	t_mensaje *mensaje_out = crear_mensaje(MEM_WRITE);
	agregar_a_mensaje(mensaje_out, "%s", dest);
	agregar_a_mensaje(mensaje_out, "%s", origin);
	enviar_mensaje(lib_ref->socket, mensaje_out);
	liberar_mensaje_out(mensaje_out);

	t_list *mensaje_in = recibir_mensaje(lib_ref->socket);

	if ((int)list_get(mensaje_in, 0) == SEG_FAULT)
	{ //TODO: el mensaje que contesta la memoria en caso de que falle el memwrite todavía no está definido, el SEG_FAULT se puso provisoriamente
		log_error(logger,
				  "Ocurrió un fallo al intentar escribir en la memoria");
		liberar_mensaje_in(mensaje_in);
		return 1;
	}
	else
	{
		log_info(logger, "La memoria fue escrita correctamente");
		liberar_mensaje_in(mensaje_in);
		return 0;
	}
}
