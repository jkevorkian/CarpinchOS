#include "carpincho.h"
#include "memoria.h"

void *rutina_carpincho(void* info_carpincho) {
	log_info(logger, "Nace un nuevo carpincho");
 	bool seguir = true;
	data_carpincho* carpincho = (data_carpincho *)info_carpincho;
	int socket = esperar_cliente(carpincho->socket);
	close(carpincho->socket);
	data_socket(socket, logger);

	t_list *mensaje_in;
	t_mensaje* mensaje_out;
	uint32_t desplazamiento_d;
	char* marioneta;
	uint32_t tamanio_mensaje;

	while(seguir) {
		mensaje_in = recibir_mensaje(socket);
		switch((int)list_get(mensaje_in, 0)) { 
		case MEM_ALLOC:
			log_info(logger, "Me llego un mem_alloc de tamanio %d", (int)list_get(mensaje_in, 1));
			mem_alloc(carpincho->id, (int)list_get(mensaje_in, 1));

			mensaje_out = crear_mensaje(MEM_ALLOC);
			agregar_a_mensaje(mensaje_out, "%d", 0);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_FREE:
			log_info(logger, "Me llego un mem_free para la posicion %d", (int)list_get(mensaje_in, 1));
			mem_free(carpincho->id, (int)list_get(mensaje_in, 1));
			
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_READ:
			log_info(logger, "Me llego un mem_read para la posicion %d", (int)list_get(mensaje_in, 1));
			desplazamiento_d = (int)list_get(mensaje_in, 1);
			
			marioneta = mem_read(carpincho->id, desplazamiento_d);
			log_info(logger, "El contenido del alloc es: %s", marioneta);

			mensaje_out = crear_mensaje(DATA);
			log_info(logger, "Creo mensaje");

			agregar_a_mensaje(mensaje_out, "%s", marioneta);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envío mensaje");
			break;
		case MEM_WRITE:
			log_info(logger, "Me llego un mem_write para la posicion %d", (int)list_get(mensaje_in, 1));
			log_info(logger, "El contenido es %s", (char *)list_get(mensaje_in, 2));

			marioneta = (char *)list_get(mensaje_in, 2);
			tamanio_mensaje = strlen(marioneta);
			desplazamiento_d = (int)list_get(mensaje_in, 1);

			if(mem_write(carpincho->id, desplazamiento_d, marioneta)) {
				free(marioneta);
				marioneta = obtener_bloque_paginacion(carpincho->id, desplazamiento_d, strlen(marioneta));
				
				log_info(logger, "Escribí: %s", marioneta);
				free(marioneta);

				mensaje_out = crear_mensaje(TODOOK);
			}
			else
				mensaje_out = crear_mensaje(SEG_FAULT);

			enviar_mensaje(socket, mensaje_out);	
			break;
		case SUSPEND:
			// ...;
			break;
		case MATE_CLOSE:
		default:
			seguir = false;
			log_info(logger, "Murio el carpincho, nos vemos.");
			break;
		}
	}
	return NULL;
}

t_carpincho* crear_carpincho(uint32_t id) {
	log_info(logger, "Creo carpincho");
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
			    t_marco* marco = obtener_marco_libre();	// La búsqueda en swap no debería hacerse, de última aclarar en el nombre que es solo de memoria
			    crear_nueva_pagina(marco->nro_real, carpincho);
		}
		carpincho->heap_metadata = NULL;
		return true;
	}

	return false;
}

bool asignacion_global(t_carpincho* carpincho) {
	return false;
}

void setear_condicion_inicial(uint32_t id) {
	uint32_t tamanio_alloc_1 = 20;
	uint32_t tamanio_alloc_2 = 13;
	uint32_t tamanio_alloc_3 = 32;
	
	uint32_t posicion_heap = 0;
	set_prevAlloc(id, posicion_heap, HEAP_NULL);
	set_nextAlloc(id, posicion_heap, tamanio_alloc_1 + TAMANIO_HEAP);	// El alloc de prueba ocupa 20 bytes
	reset_isFree(id, posicion_heap);

	posicion_heap = posicion_heap + tamanio_alloc_1 + TAMANIO_HEAP;
	set_prevAlloc(id, tamanio_alloc_1 + TAMANIO_HEAP, 0);
	set_nextAlloc(id, tamanio_alloc_1 + TAMANIO_HEAP, HEAP_NULL);	// El alloc de prueba ocupa 21 bytes
	reset_isFree(id, tamanio_alloc_1 + TAMANIO_HEAP);
}

void rutina_test_carpincho(data_carpincho *info_carpincho) {
	log_info(logger, "Nace un nuevo carpincho");
	bool seguir = true;
	data_carpincho* carpincho = (data_carpincho *)info_carpincho;
	int socket = esperar_cliente(carpincho->socket);
	close(carpincho->socket);
	data_socket(socket, logger);

	t_list *mensaje_in;
	t_mensaje* mensaje_out;
	uint32_t desplazamiento_d;
	char* marioneta;
	uint32_t tamanio_mensaje;

	setear_condicion_inicial(carpincho->id);

	while(seguir) {
		mensaje_in = recibir_mensaje(socket);

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case MEM_ALLOC:

			mensaje_out = crear_mensaje(MEM_ALLOC);
			agregar_a_mensaje(mensaje_out, "%d", 0);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_FREE:
			log_info(logger, "Me llego un mem_free para la posicion %d", (int)list_get(mensaje_in, 1));
			// mem_free(carpincho->id, (int)list_get(mensaje_in, 1));

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_READ:
			log_info(logger, "Me llego un mem_read para la posicion %d", (int)list_get(mensaje_in, 1));
			desplazamiento_d = (int)list_get(mensaje_in, 1);
			
			marioneta = mem_read(carpincho->id, desplazamiento_d);
			log_info(logger, "El contenido del alloc es: %s", marioneta);
			
			mensaje_out = crear_mensaje(DATA);
			log_info(logger, "Creo mensaje");
			agregar_a_mensaje(mensaje_out, "%s", marioneta);
			enviar_mensaje(socket, mensaje_out);
			log_info(logger, "Envío mensaje");
			break;
		case MEM_WRITE:
			log_info(logger, "Me llego un mem_write para la posicion %d", (int)list_get(mensaje_in, 1));
			log_info(logger, "El contenido es %s", (char *)list_get(mensaje_in, 2));
			marioneta = (char *)list_get(mensaje_in, 2);
			tamanio_mensaje = strlen(marioneta);
			desplazamiento_d = (int)list_get(mensaje_in, 1);

			if(mem_write(carpincho->id, desplazamiento_d, marioneta)) {
				free(marioneta);
				marioneta = obtener_bloque_paginacion(carpincho->id, desplazamiento_d, strlen(marioneta));
				
				log_info(logger, "Escribí: %s", marioneta);
				free(marioneta);

				mensaje_out = crear_mensaje(TODOOK);
			}
			else
				mensaje_out = crear_mensaje(SEG_FAULT);
			
			enviar_mensaje(socket, mensaje_out);
			break;
		case SUSPEND:
			// ...;
			break;
		case MATE_CLOSE:
		default:
			seguir = false;
			log_info(logger, "Murio el carpincho, nos vemos.");
			break;
		}
	}
}
