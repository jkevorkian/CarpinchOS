#include "sockets.h"

int crear_conexion_cliente(char *ip, char *puerto)
{
	struct addrinfo hints, *server_info;
	int socket_cliente;
	int error;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0; // cualquier protocolo
	//hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_NUMERICHOST;

	if ((error = getaddrinfo(ip, puerto, &hints, &server_info)) != 0)
	{
		freeaddrinfo(server_info);
		return -1;
	}

	socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if (socket_cliente == -1)
	{
		freeaddrinfo(server_info);
		return -2;
	}

	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		freeaddrinfo(server_info);
		return -3;
	}

	freeaddrinfo(server_info);
	return socket_cliente;
}

int crear_conexion_servidor(char *ip, int puerto, int cola_escucha)
{
	int socket_servidor;
	int error;
	struct addrinfo hints, *servinfo, *addr_aux;

	char str_puerto[7];
	sprintf(str_puerto, "%d", puerto);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE;	// En pricipio es 0 (any)
	//hints.protocol = 0;			// TCP

	if ((error = getaddrinfo(ip, str_puerto, &hints, &servinfo)) != 0)
	{
		return -1;
	}

	int yes = 1;
	for (addr_aux = servinfo; addr_aux != NULL; addr_aux = addr_aux->ai_next)
	{
		if ((socket_servidor = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		{
			continue;
		}
		if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			continue;
		}

		if (bind(socket_servidor, addr_aux->ai_addr, addr_aux->ai_addrlen) == -1)
		{
			close(socket_servidor);
			continue;
		}

		break;
	}

	if (addr_aux == NULL)
	{
		return -2;
	}

	listen(socket_servidor, cola_escucha);
	if ((error = listen(socket_servidor, 1)) != 0)
	{
		return -3;
	}

	freeaddrinfo(servinfo);

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void *)&dir_cliente, &tam_direccion);
	return socket_cliente;
}

int puerto_desde_socket(int socket)
{
	struct sockaddr_in dir_cliente;
	socklen_t largo = sizeof(struct addrinfo);

	getsockname(socket, (struct sockaddr *)&dir_cliente, &largo);
	return ntohs(dir_cliente.sin_port);
}

bool validar_socket(int socket, t_log *logger)
{
	switch (socket)
	{
	case -1:
		log_error(logger, "Error al crear socket. La conexión falló en getaddrinfo");
		break;
	case -2:
		log_error(logger, "Error al crear socket. Puerto no disponible");
		break;
	case -3:
		log_error(logger, "Error al crear socket. Fallo al conectarse al servidor");
		break;
	default:
		return true;
	}
	return false;
}

void data_socket(int socket, t_log *logger)
{
	char ipstr[INET_ADDRSTRLEN];
	struct sockaddr_in dir_cliente;
	socklen_t largo = sizeof(struct addrinfo);

	if (getsockname(socket, (struct sockaddr *)&dir_cliente, &largo) == 0)
	{
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		log_info(logger, "Dirección local: %s:%d", ipstr, ntohs(dir_cliente.sin_port));
	}
	else
	{
		log_warning(logger, "El socket no está iniciado");
	}
	if (getpeername(socket, (struct sockaddr *)&dir_cliente, &largo) == 0)
	{
		inet_ntop(AF_INET, &(dir_cliente.sin_addr), ipstr, sizeof ipstr);
		log_info(logger, "Dirección remota: %s:%d", ipstr, ntohs(dir_cliente.sin_port));
	}
	else
	{
		log_warning(logger, "El socket no está conectado a un socket remoto");
	}
}

void crear_buffer(t_mensaje *mensaje)
{
	mensaje->buffer = malloc(sizeof(t_buffer));
	mensaje->buffer->tamanio = 0;
	mensaje->buffer->contenido = NULL;
}

t_mensaje *crear_mensaje(protocolo_msj cod_op)
{
	t_mensaje *mensaje = malloc(sizeof(t_mensaje));
	mensaje->op_code = cod_op;
	crear_buffer(mensaje);
	return mensaje;
}

void agregar_a_mensaje(t_mensaje *mensaje, char *formato, ...)
{
	char *ptr_form = formato;
	va_list ptr_arg;
	va_start(ptr_arg, formato);
	void *parametro_nuevo;

	while (*ptr_form)
	{
		if (*ptr_form != '%')
		{
			ptr_form++;
			continue;
		}
		ptr_form++;

		uint32_t valor_parametro;
		uint32_t tamanio_buffer;

		switch (*ptr_form)
		{
		case 'd':
			mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t));
			valor_parametro = va_arg(ptr_arg, uint32_t);
			parametro_nuevo = &valor_parametro;
			tamanio_buffer = sizeof(uint32_t);
			break;
		case 's':
			if (*(ptr_form + 1) == 'd')
			{
				tamanio_buffer = va_arg(ptr_arg, uint32_t);
				parametro_nuevo = (void *)va_arg(ptr_arg, char *);
			}
			else
			{
				parametro_nuevo = (void *)va_arg(ptr_arg, char *);
				tamanio_buffer = strlen((char *)parametro_nuevo) + 1;
			}
			mensaje->buffer->contenido = realloc(mensaje->buffer->contenido, mensaje->buffer->tamanio + sizeof(uint32_t) + tamanio_buffer);

			memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, &tamanio_buffer, sizeof(uint32_t));
			mensaje->buffer->tamanio = mensaje->buffer->tamanio + sizeof(uint32_t);
			break;
		}
		memcpy(mensaje->buffer->contenido + mensaje->buffer->tamanio, parametro_nuevo, tamanio_buffer);
		mensaje->buffer->tamanio = mensaje->buffer->tamanio + tamanio_buffer;
	}
}

void enviar_mensaje(int socket, t_mensaje *mensaje)
{
	uint32_t tamanio_buffer = mensaje->buffer->tamanio + sizeof(uint32_t);
	void *buffer_to_send = malloc(tamanio_buffer);
	memcpy(buffer_to_send, &(mensaje->op_code), sizeof(uint32_t));
	memcpy(buffer_to_send + sizeof(uint32_t), mensaje->buffer->contenido, mensaje->buffer->tamanio);
	send(socket, buffer_to_send, tamanio_buffer, 0);
	free(buffer_to_send);
}

void recibir_parametros(int socket, t_list *parametros, char *formato)
{
	uint32_t parametro;
	// char *ptr_form = formato_msj[(int)list_get(parametros, 0)];
	char *ptr_form = formato;

	while (*ptr_form)
	{
		if (*ptr_form != '%')
		{
			ptr_form++;
			continue;
		}
		ptr_form++;

		uint32_t tamanio_buffer;
		uint32_t num_cadenas;
		switch (*ptr_form)
		{
		case 'd':
			tamanio_buffer = sizeof(uint32_t);
			recv(socket, &parametro, tamanio_buffer, MSG_WAITALL);
			list_add(parametros, (void *)parametro);
			break;
		case 's':
			if (*(ptr_form + 1) == 's')
				num_cadenas = (uint32_t)list_get(parametros, list_size(parametros) - 1);
			else
				num_cadenas = 1;
			for (int i = 0; i < num_cadenas; i++)
			{
				tamanio_buffer = sizeof(uint32_t);
				recv(socket, &tamanio_buffer, tamanio_buffer, MSG_WAITALL);
				char *buffer = malloc(tamanio_buffer * sizeof(char));
				recv(socket, buffer, tamanio_buffer, MSG_WAITALL);
				list_add(parametros, buffer);
			}
			break;
		}
	}
}

t_list *recibir_mensaje(int socket)
{
	protocolo_msj op_code;
	t_list *lista_parametros = list_create();
	int error;
	error = recv(socket, &op_code, sizeof(uint32_t), MSG_WAITALL);

	if (error == 0)
	{
		op_code = ER_SOC;
		list_add(lista_parametros, (void *)ER_SOC);
		// perror("read");
	}

	if (error == -1)
	{
		op_code = ER_RCV;
		list_add(lista_parametros, (void *)ER_RCV);
		// perror("read");
	}

	if (error > 0)
		list_add(lista_parametros, (void *)op_code);

	switch (op_code)
	{
	// Validaciones
	case ER_RCV:
	case ER_SOC:
	case TODOOK:
	case NO_MEMORY:
	case SEG_FAULT:
		break;
	// Inicialización
	case MATE_INIT:
	case MATE_CLOSE:
		break;
	// Memoria
	case MEM_ALLOC:
		recibir_parametros(socket, lista_parametros, S_MEM_ALLOC);
		break;
	case MEM_FREE:
		recibir_parametros(socket, lista_parametros, S_MEM_FREE);
		break;
	case MEM_READ:
		recibir_parametros(socket, lista_parametros, S_MEM_READ);
		break;
	case MEM_WRITE:
		recibir_parametros(socket, lista_parametros, S_MEM_WRITE);
		break;
	// SWAMP
	case NEW_PAGE:
		recibir_parametros(socket, lista_parametros, S_NEW_PAGE);
		break;
	case GET_PAGE:
		recibir_parametros(socket, lista_parametros, S_GET_PAGE);
		break;
	case SET_PAGE:
		recibir_parametros(socket, lista_parametros, S_SET_PAGE);
		break;
	case RM_PAGE:
		recibir_parametros(socket, lista_parametros, S_RM_PAGE);
		break;
	// case NEW_C:		recibir_parametros(socket, lista_parametros, S_NEW_C);		break;
	case EXIT_C:
		recibir_parametros(socket, lista_parametros, S_EXIT_C);
		break;
	// Semaforos
	case SEM_INIT:
		recibir_parametros(socket, lista_parametros, S_SEM_INIT);
		break;
	case SEM_WAIT:
		recibir_parametros(socket, lista_parametros, S_SEM_WAIT);
		break;
	case SEM_POST:
		recibir_parametros(socket, lista_parametros, S_SEM_POST);
		break;
	case SEM_DESTROY:
		recibir_parametros(socket, lista_parametros, S_SEM_DESTROY);
		break;
	// Otros
	case CALL_IO:
		recibir_parametros(socket, lista_parametros, S_CALL_IO);
		break;
	case DATA_CHAR:
		recibir_parametros(socket, lista_parametros, S_DATA_CHAR);
		break;
	case DATA_INT:
		recibir_parametros(socket, lista_parametros, S_DATA_INT);
		break;
	case SEND_PORT:
		recibir_parametros(socket, lista_parametros, S_SEND_PORT);
		break;
	case SUSPEND:
		recibir_parametros(socket, lista_parametros, S_SUSPEND);
		break;
	case UNSUSPEND:
		recibir_parametros(socket, lista_parametros, S_UNSUSPEND);
		break;

	default:
		break;
	}

	return lista_parametros;
}

void liberar_mensaje_out(t_mensaje *mensaje)
{
	free(mensaje->buffer->contenido);
	free(mensaje->buffer);
	free(mensaje);
}

void liberar_mensaje_in(t_list *mensaje)
{
	// list_remove(mensaje, 0);
	// Es necesario borrar los strings compartidos ni bien se los termine de usar,
	// ya que en esta función no se hace esa tarea
	list_destroy(mensaje);
}

bool validar_mensaje(t_list *mensaje_in, void *logger)
{
	switch ((int)list_get(mensaje_in, 0))
	{
	case ER_RCV:
		if (logger)
			log_warning((t_log *)logger, "Ha ocurrido un fallo inesperado en la recepción del mensaje.");
		return false;
		break;
	case ER_SOC:
		if (logger)
			log_warning((t_log *)logger, "La conexión remota se ha desconectado.");
		return false;
		break;
	}
	return true;
}

char *string_desde_mensaje(int mensaje)
{
	char *listaDeStrings[] = {
		// Validaciones
		"ER_SOC", "ER_RCV", "TODOOK", "NO_MEMORY", "SEG_FAULT",
		// Inicializacion
		"MATE_INIT", "MATE_CLOSE",
		// Memoria
		"MEM_ALLOC", "MEM_FREE", "MEM_READ", "MEM_WRITE",
		// SWAMP
		"GET_PAGE", "SET_PAGE", "SUSPEND", "UNSUSPEND", "NEW_C", "EXIT_C",
		// Semaforos
		"SEM_INIT", "SEM_WAIT", "SEM_POST", "SEM_DESTROY",
		// Otros
		"CALL_IO", "DATA", "SEND_PORT"};

	return listaDeStrings[mensaje];
}
