#include "carpincho.h"

bool imprimo = true;

void *rutina_carpincho(void *info_carpincho) {
	bool seguir = true;
	data_carpincho* carpincho = (data_carpincho *)info_carpincho;
	int socket = esperar_cliente(carpincho->socket);
	close(carpincho->socket);
	data_socket(socket, logger);

	t_list *mensaje_in;
	t_mensaje* mensaje_out;
	void* contenido;
	uint32_t dir_logica;
	uint32_t tamanio;

	while(seguir) {
		mensaje_in = recibir_mensaje(socket);

		switch((int)list_get(mensaje_in, 0)) { // protocolo del mensaje
		case MEM_ALLOC:
			tamanio = (int)list_get(mensaje_in, 1);
			log_warning(logger, "Me llego un mem_alloc de tamanio %d", tamanio);
			
			if((dir_logica = mem_alloc(carpincho->id, tamanio))) {
				mensaje_out = crear_mensaje(DATA_INT);
				agregar_a_mensaje(mensaje_out, "%d", dir_logica);
			}
			else
				mensaje_out = crear_mensaje(NO_MEMORY);
			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			break;
		case MEM_FREE:
			log_warning(logger, "Me llego un mem_free para la posicion %d", (int)list_get(mensaje_in, 1));
			dir_logica = (int)list_get(mensaje_in, 1);
			
			if(mem_free(carpincho->id, dir_logica))
				mensaje_out = crear_mensaje(TODOOK);
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_FREE_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			break;
		case MEM_READ:
			log_warning(logger, "Me llego un mem_read para la posicion %d", (int)list_get(mensaje_in, 1));
			dir_logica = (int)list_get(mensaje_in, 1);
			tamanio = (int)list_get(mensaje_in, 2);
			
			if((contenido = mem_read(carpincho->id, dir_logica, tamanio))) {
				loggear_data(contenido, tamanio);
				mensaje_out = crear_mensaje(DATA_PAGE);
				agregar_a_mensaje(mensaje_out, "%sd", tamanio, contenido);
			}
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_READ_FAULT);
			}

			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			free(contenido);
			break;
		case MEM_WRITE:
			log_warning(logger, "Me llego un mem_write para la posicion %d", (int)list_get(mensaje_in, 1));
			dir_logica = (int)list_get(mensaje_in, 1);
			tamanio = (int)list_get(mensaje_in, 2);
			contenido = list_get(mensaje_in, 3);

			if(mem_write(carpincho->id, dir_logica, contenido, tamanio)) {
				loggear_data(contenido, tamanio);
				mensaje_out = crear_mensaje(TODOOK);
			}
			else {
				log_info(logger, "SEGMENTATION FAULT");
				// seguir = false;		// Comentado para probar
				mensaje_out = crear_mensaje(SEG_FAULT);
				agregar_a_mensaje(mensaje_out, "%d", MATE_WRITE_FAULT);
			}
			
			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			break;
		case SUSPEND:
			log_warning(logger, "Me llego una orden de suspension");
			
			suspend(carpincho->id);
			
			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			break;
		case UNSUSPEND:
			log_warning(logger, "Me llego un permiso para volver a memoria (unsuspend)");
			unsuspend(carpincho->id);

			mensaje_out = crear_mensaje(TODOOK);
			enviar_mensaje(socket, mensaje_out);

			liberar_mensaje_in(mensaje_in);
			liberar_mensaje_out(mensaje_out);
			break;
		case MATE_CLOSE:
		default:
			seguir = false;
			log_warning(logger, "Murio el carpincho, nos vemos.");
			pthread_mutex_lock(&mutex_lista_carpinchos);
			if(imprimo) {
				print_marcos();
				imprimo = false;
			}
			pthread_mutex_unlock(&mutex_lista_carpinchos);

			liberar_mensaje_in(mensaje_in);
			break;
		}
	}
	eliminar_carpincho(carpincho->id);
	free(carpincho);
	
	return NULL;
}

t_carpincho* crear_carpincho(uint32_t id) {
	t_carpincho* carpincho = malloc(sizeof(t_carpincho));
	carpincho->id = id;
	carpincho->tabla_paginas = list_create();
	pthread_mutex_init(&carpincho->mutex_tabla, NULL);
	carpincho->offset = 0;
	carpincho->puntero_clock = 0;

	pthread_mutex_lock(&mutex_lista_carpinchos);
	list_add(lista_carpinchos, carpincho);
	pthread_mutex_unlock(&mutex_lista_carpinchos);

	t_hit_miss_tlb *historico_carpincho_tlb = malloc(sizeof(t_hit_miss_tlb));
	historico_carpincho_tlb->id_carpincho = id;
	historico_carpincho_tlb->cant_hit = 0;
	historico_carpincho_tlb->cant_miss = 0;

	pthread_mutex_lock(&mutex_historico_hit_miss);
	list_add(historico_hit_miss, historico_carpincho_tlb);
	pthread_mutex_unlock(&mutex_historico_hit_miss);
	return carpincho;
}

void eliminar_carpincho(uint32_t id_carpincho) {
	log_info(logger, "Elimino al carpincho %d", id_carpincho);
	// Limpio entradas de la tlb
	for(int i = 0; i < tlb.cant_entradas; i++) {
		t_entrada_tlb* entrada = tlb.mapa[i];
		if(entrada->id == id_carpincho){
			pthread_mutex_lock(&mutex_asignacion_tlb);
			pthread_mutex_lock(&entrada->mutex);
			entrada->id = 0;
			pthread_mutex_unlock(&entrada->mutex);
			pthread_mutex_unlock(&mutex_asignacion_tlb);
		}
	}

	crear_movimiento_swap(EXIT_C, id_carpincho, 0, NULL);

	uint32_t nro_marcos_carpincho;

	pthread_mutex_lock(&mutex_asignacion_marcos);
	t_marco **marcos_de_carpincho = obtener_marcos_proceso(id_carpincho, &nro_marcos_carpincho);	// Podria ir afuera ??
	for(int i = 0; i < nro_marcos_carpincho; i++) {
		pthread_mutex_lock(&marcos_de_carpincho[i]->mutex_espera_uso);
		marcos_de_carpincho[i]->duenio = 0;
		pthread_mutex_unlock(&marcos_de_carpincho[i]->mutex_espera_uso);
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
	free(carpincho);

}
