#include "carpincho.h"

void *rutina_carpincho(void* info_carpincho) {
	bool seguir = true;
	int socket_carpincho = ((data_carpincho *)info_carpincho)->socket;
<<<<<<< HEAD
	// en asignación fija, reservar paginas
=======
	int socket = esperar_cliente(socket_carpincho);
	close(socket_carpincho);

	t_list *mensaje_in = recibir_mensaje(socket);
	int id = (int)list_get(mensaje_in, 2);
	t_carpincho* carpincho = crear_carpincho(id);

>>>>>>> 6c0d826f3802c0cb7bad2a60716508bb6a355d32
	while(seguir) {
		mensaje_in = recibir_mensaje(socket_carpincho);
		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case MEM_ALLOC:
			mem_alloc(carpincho, (int)list_get(mensaje_in, 1));
			break;
		case MEM_FREE:
			// mem_free(id_carpincho, dir_logica);
			break;
		case MEM_READ:
			// mem_read(id_carpincho, dir_logica);
			break;
		case MEM_WRITE:
			// mem_write(id_carpincho, dir_logica, data);
			break;
		case SUSPEND:
			// ...;
			break;
		}
	}
	return NULL;
}

t_carpincho* crear_carpincho(int id) {
	t_carpincho* carpincho = (t_carpincho*)malloc(sizeof(t_carpincho));
	carpincho->id = id;
	// carpincho->tabla_paginas = list_create();
	//mutex
	list_add(lista_carpinchos, carpincho);
	// log_trace(logger_trace, "Se admitio correctamente el proceso #%d", proceso->id);
	return carpincho;
}
