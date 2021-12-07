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

	uint32_t dir_logica;

	while(seguir) {
		mensaje_in = recibir_mensaje(socket);
		switch((int)list_get(mensaje_in, 0)) { 
		case MEM_ALLOC:
			log_info(logger, "Me llego un mem_alloc de tamanio %d", (int)list_get(mensaje_in, 1));

			if((dir_logica = mem_alloc(carpincho->id, (int)list_get(mensaje_in, 1)))) {
				mensaje_out = crear_mensaje(DATA_INT);
				agregar_a_mensaje(mensaje_out, "%d", dir_logica);
			}
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_FREE:
			log_info(logger, "Me llego un mem_free para la posicion %d", (int)list_get(mensaje_in, 1));
			
			if(mem_free(carpincho->id, (int)list_get(mensaje_in, 1)))
				mensaje_out = crear_mensaje(TODOOK);
			else {
				log_info(logger, "SEGMENTATION FAULT");
				seguir = false;
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_FREE_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_READ:
			log_info(logger, "Me llego un mem_read para la posicion %d", (int)list_get(mensaje_in, 1));

			char *buffer;
			if((buffer = mem_read(carpincho->id, (int)list_get(mensaje_in, 1)))) {
				mensaje_out = crear_mensaje(DATA_CHAR);
				agregar_a_mensaje(mensaje_out, "%s", buffer);
			}
			else {
				log_info(logger, "SEGMENTATION FAULT");
				seguir = false;
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_READ_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_WRITE:
			log_info(logger, "Me llego un mem_write para la posicion %d", (int)list_get(mensaje_in, 1));

			if(mem_write(carpincho->id, (int)list_get(mensaje_in, 1), (char *)list_get(mensaje_in, 2)))
				mensaje_out = crear_mensaje(TODOOK);
			else {
				log_info(logger, "SEGMENTATION FAULT");
				seguir = false;
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_WRITE_FAULT);
			}
				
			enviar_mensaje(socket, mensaje_out);
			break;
		case SUSPEND:
			log_info(logger, "Me llego una orden de suspensión");
			
			suspend(carpincho->id);
			
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case UNSUSPEND:
			log_info(logger, "Me llego un permiso para volver a memoria");

			unsuspend(carpincho->id);
			
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MATE_CLOSE:
		default:
			seguir = false;
			break;
		}
	}
	
	log_info(logger, "Murio el carpincho, nos vemos.");
	crear_movimiento_swap(EXIT_C, carpincho->id, 0, NULL);
	return NULL;
}

t_carpincho* crear_carpincho(uint32_t id) {
	log_info(logger, "Creo carpincho");
	t_carpincho* carpincho = malloc(sizeof(t_carpincho));
	carpincho->id = id;
	carpincho->tabla_paginas = list_create();
	pthread_mutex_init(&carpincho->mutex_tabla, NULL);
	carpincho->sem_tlb = malloc(sizeof(sem_t));	// No se bien para qué sirve
	sem_init(carpincho->sem_tlb, 0 , 1);
	carpincho->offset = 0;

	pthread_mutex_lock(&mutex_lista_carpinchos);
	list_add(lista_carpinchos, carpincho);
	pthread_mutex_unlock(&mutex_lista_carpinchos);

	log_info(logger, "Se admitio correctamente el carpincho #%d", carpincho->id);
	return carpincho;
}

bool asignacion_fija(t_carpincho* carpincho) {
	uint32_t cant_marcos = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
	bool resultado = false;
	pthread_mutex_lock(&mutex_asignacion_marcos);
	if(tengo_marcos_suficientes(cant_marcos) && crear_movimiento_swap(NEW_PAGE, carpincho->id, cant_marcos, NULL)) {
		for(int i = 0; i < cant_marcos; i++){
			t_marco* marco = obtener_marco_libre();
			marco->duenio = carpincho->id;
			marco->pagina_duenio = i;
			
			t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));			
			pthread_mutex_init(&pagina->mutex, NULL);
			pagina->nro_marco = marco->nro_real;
			pagina->presencia = true;

			pthread_mutex_lock(&carpincho->mutex_tabla);
			list_add(carpincho->tabla_paginas, pagina);
			pthread_mutex_unlock(&carpincho->mutex_tabla);
		}
		resultado = true;
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);

	return resultado;
}

#define cant_alloc 4

void setear_condicion_inicial(uint32_t id) {
	t_carpincho * carpincho = carpincho_de_lista(id);
	asignacion_fija(carpincho);

	if(agregar_pagina(id)) {
		log_info(logger, "Pude pedir una nueva página, grande la swap");
	}
	// uint32_t cant_alloc = 4;
	uint32_t tamanio_alloc[cant_alloc] = { 20, 13, 32 , 25 };
	
	uint32_t posicion_heap = 0;
	uint32_t next_alloc = tamanio_alloc[0] + TAMANIO_HEAP;
	uint32_t prev_alloc = HEAP_NULL;
	for(int i = 1; i < cant_alloc + 1; i++) {
		log_info(logger, "Prev: %d; main: %d; next: %d", prev_alloc, posicion_heap, next_alloc);
		set_prevAlloc(id, posicion_heap, prev_alloc);
		set_nextAlloc(id, posicion_heap, next_alloc);
		reset_isFree(id, posicion_heap);
		prev_alloc = posicion_heap;
		posicion_heap = next_alloc;
		if(i < cant_alloc)	next_alloc += tamanio_alloc[i] + TAMANIO_HEAP;
		else				next_alloc = HEAP_NULL;
	}
	log_info(logger, "Prev: %d; main: %d; next: %d", prev_alloc, posicion_heap, next_alloc);
	set_prevAlloc(id, posicion_heap, prev_alloc);
	set_nextAlloc(id, posicion_heap, next_alloc);	// El alloc de prueba ocupa 21 bytes
	set_isFree(id, posicion_heap);
}

void obtener_condicion_final(uint32_t id) {
	// uint32_t cant_alloc = 4;
	uint32_t tamanio_alloc[cant_alloc] = { 20, 13, 32, 25 };
	
	uint32_t posicion_heap = 0;
	uint32_t next_alloc = tamanio_alloc[0] + TAMANIO_HEAP;
	for(int i = 1; i < cant_alloc + 1; i++) {
		log_info(logger, "Valor heap %d: %d", i - 1, get_prevAlloc(id, posicion_heap));
		log_info(logger, "Valor heap %d: %d", i - 1, get_nextAlloc(id, posicion_heap));
		log_info(logger, "Valor heap %d: %d", i - 1, get_isFree(id, posicion_heap));
		posicion_heap = next_alloc;
		if(i < cant_alloc)	next_alloc += tamanio_alloc[i] + TAMANIO_HEAP;
		else		next_alloc = HEAP_NULL;
	}
	log_info(logger, "Valor heap %d: %d", cant_alloc + 1, get_prevAlloc(id, posicion_heap));
	log_info(logger, "Valor heap %d: %d", cant_alloc + 1, get_nextAlloc(id, posicion_heap));	// El alloc de prueba ocupa 21 bytes
	log_info(logger, "Valor heap %d: %d", cant_alloc + 1, get_isFree(id, posicion_heap));
}

void *rutina_test_carpincho(void *info_carpincho) {
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
	uint32_t dir_logica;

	// setear_condicion_inicial(carpincho->id);

	while(seguir) {
		mensaje_in = recibir_mensaje(socket);

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case MEM_ALLOC:
			log_info(logger, "Me llego un mem_alloc de tamanio %d", (int)list_get(mensaje_in, 1));
			desplazamiento_d = (int)list_get(mensaje_in, 1);
			
			if((dir_logica = mem_alloc(carpincho->id, desplazamiento_d))) {
				mensaje_out = crear_mensaje(DATA_INT);
				agregar_a_mensaje(mensaje_out, "%d", dir_logica);
			}
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_FREE:
			log_info(logger, "Me llego un mem_free para la posicion %d", (int)list_get(mensaje_in, 1));
			desplazamiento_d = (int)list_get(mensaje_in, 1);
			
			if(mem_free(carpincho->id, desplazamiento_d))
				mensaje_out = crear_mensaje(TODOOK);
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_FREE_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);
			// obtener_condicion_final(carpincho->id);
			break;
		case MEM_READ:
			log_info(logger, "Me llego un mem_read para la posicion %d", (int)list_get(mensaje_in, 1));
			desplazamiento_d = (int)list_get(mensaje_in, 1);
			
			if((marioneta = mem_read(carpincho->id, desplazamiento_d))) {
				log_info(logger, "El contenido del alloc es: %s", marioneta);
				mensaje_out = crear_mensaje(DATA_CHAR);
				agregar_a_mensaje(mensaje_out, "%s", marioneta);
			}
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_READ_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);
			break;
		case MEM_WRITE:
			log_info(logger, "Me llego un mem_write para la posicion %d", (int)list_get(mensaje_in, 1));
			log_info(logger, "El contenido es %s", (char *)list_get(mensaje_in, 2));
			marioneta = (char *)list_get(mensaje_in, 2);
			tamanio_mensaje = strlen(marioneta);
			desplazamiento_d = (int)list_get(mensaje_in, 1);

			if(mem_write(carpincho->id, desplazamiento_d, marioneta)) {
				free(marioneta);
				marioneta = obtener_bloque_paginacion(carpincho->id, desplazamiento_d, tamanio_mensaje);
				log_info(logger, "Escribí: %s", marioneta);
				free(marioneta);
				mensaje_out = crear_mensaje(TODOOK);
			}
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_WRITE_FAULT);
			}
			
			enviar_mensaje(socket, mensaje_out);
			break;
		case SUSPEND:
			log_info(logger, "Me llego una orden de suspensión");
			
			suspend(carpincho->id);
			
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case UNSUSPEND:
			log_info(logger, "Me llego un permiso para volver a memoria");
			unsuspend(carpincho->id);

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);
			break;
		case MATE_CLOSE:
		default:
			seguir = false;
			log_info(logger, "Murio el carpincho, nos vemos.");
			break;
		}
	}
	eliminar_carpincho(carpincho->id);
	// crear_movimiento_swap(EXIT_C, carpincho->id, 0, NULL);
	free(carpincho);
	
	return NULL;
}

void eliminar_carpincho(uint32_t id_carpincho) {
	crear_movimiento_swap(EXIT_C, id_carpincho, 0, NULL);

	uint32_t nro_marcos_carpincho;

	pthread_mutex_lock(&mutex_asignacion_marcos);
	t_marco **marcos_de_carpincho = obtener_marcos_proceso(id_carpincho, &nro_marcos_carpincho);	// Podría ir afuera ??
	for(int i = 0; i < nro_marcos_carpincho; i++) {
		liberar_marco(marcos_de_carpincho[i]);
	}
	pthread_mutex_unlock(&mutex_asignacion_marcos);
	free(marcos_de_carpincho);

	t_carpincho *carpincho = carpincho_de_lista(id_carpincho);

	bool mi_carpincho(void *un_carpincho) {
		if((t_carpincho *)un_carpincho == carpincho)
			return true;
		else
			return false;
	}
	
	pthread_mutex_lock(&mutex_lista_carpinchos);
	list_remove_by_condition(lista_carpinchos, mi_carpincho);
	pthread_mutex_unlock(&mutex_lista_carpinchos);

	while(list_size(carpincho->tabla_paginas)) {
		t_entrada_tp *entrada = list_remove(carpincho->tabla_paginas, 0);
		pthread_mutex_destroy(&entrada->mutex);
		free(entrada);
	}
	list_destroy(carpincho->tabla_paginas);
	flush_proceso_tlb(id_carpincho);		// NUEVO
	free(carpincho);

}

//////////////////////////////////////////////////////////
// No se usa creo
uint32_t cant_paginas_carpincho(t_carpincho *carpincho) {
	pthread_mutex_lock(&carpincho->mutex_tabla);
	uint32_t cant_paginas = list_size(carpincho->tabla_paginas);
	pthread_mutex_unlock(&carpincho->mutex_tabla);

	return cant_paginas;
}