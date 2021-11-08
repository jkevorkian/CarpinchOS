#include "carpincho.h"
#include "memoria.h"

void *rutina_carpincho(void* info_carpincho) {
	// COMENTO PORQUE SINO NO COMPILA
/* 	bool seguir = true;
	int socket_carpincho = ((data_carpincho *)info_carpincho)->socket;
	int socket = esperar_cliente(socket_carpincho);
	close(socket_carpincho);

	// TODO: generar id en memoria, NO leerla del mensaje
	t_list *mensaje_in = recibir_mensaje(socket);
	int id = (int)list_get(mensaje_in, 2);
	t_carpincho* carpincho = crear_carpincho(id);

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
		}
	}
	return NULL; */
}

t_carpincho* crear_carpincho(uint32_t id) {
	t_carpincho* carpincho = malloc(sizeof(t_carpincho));
	carpincho->id = id;
	carpincho->tabla_paginas = list_create();

	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		if(!asignacion_fija(carpincho)) return NULL;
	}

	if(config_memoria.tipo_asignacion == DINAMICA_GLOBAL) {
		if(!asignacion_global(carpincho)) return NULL;
	}

	list_add(lista_carpinchos, carpincho);
	log_info(logger, "Se admitio correctamente el carpincho #%d", carpincho->id);
	return carpincho;
}

bool asignacion_fija(t_carpincho* carpincho) {
	uint32_t cant_marcos = config_get_int_value(config, "MARCOS_POR_CARPINCHO");

	if(tengo_marcos_suficientes(cant_marcos)){
		for(int i = 0; i < cant_marcos; i++){
			    t_marco* marco = obtener_marco_libre();
			    crear_nueva_pagina(marco->nro_real, carpincho);
		}

		return true;
	}

	return false;
}

bool asignacion_global(t_carpincho* carpincho) {
	return false;
}