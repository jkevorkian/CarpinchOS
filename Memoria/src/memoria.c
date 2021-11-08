#include "memoria.h"

bool iniciar_memoria(t_config* config){
	config_memoria.tamanio_memoria = config_get_int_value(config, "TAMANIO");
	config_memoria.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");

	if(config_memoria.tamanio_memoria % config_memoria.tamanio_pagina > 0) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

    memoria_ram.inicio = (void*) malloc(config_memoria.tamanio_memoria);   
    if (memoria_ram.inicio == NULL) {
        log_error(logger, "Hubo un error al crear la memoria.");
        return false;
    }

	memset(memoria_ram.inicio, 0, config_memoria.tamanio_memoria);

	config_memoria.cant_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	memoria_ram.mapa_fisico = calloc(config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro marcos: %d. Tamaño marco: %d bytes", config_memoria.cant_marcos, config_memoria.tamanio_pagina);
	
	char * algoritmo_reemplazo_mmu = config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU");
	if(!strcmp(algoritmo_reemplazo_mmu, "LRU"))
		config_memoria.algoritmo_reemplazo = LRU;
	if(!strcmp(algoritmo_reemplazo_mmu, "CLOCK-M")) {
		config_memoria.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}

	char * tipo_asignacion = config_get_string_value(config, "TIPO_ASIGNACION");
	if(!strcmp(tipo_asignacion, "FIJA"))
		config_memoria.tipo_asignacion = FIJA_LOCAL;
	if(!strcmp(tipo_asignacion, "DINAMICA")) {
		config_memoria.tipo_asignacion = DINAMICA_GLOBAL;
	}

	iniciar_marcos(config_memoria.cant_marcos);
	//iniciar_tlb(config);  

	return true;
}

void iniciar_marcos(uint32_t cant_marcos){
	for(int i = 0; i < cant_marcos; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_fisico[i] = marco_auxiliar;

		marco_auxiliar->libre = true;
		marco_auxiliar->nro_real = i;
	}
}

t_entrada_tp* crear_nueva_pagina(uint32_t nro_marco, t_carpincho* carpincho){
		t_entrada_tp* pagina = malloc(sizeof(t_entrada_tp));
		list_add(carpincho->tabla_paginas, pagina);
		
		pagina->nro_marco = nro_marco;
		pagina->presencia = true; 
		pagina->modificado = false;
        pagina->uso = true;

		log_info(logger, "Asigno frame. Cant marcos del carpincho #%d: %d", carpincho->id, list_size(carpincho->tabla_paginas));
		log_info(logger, "Datos pagina. Marco:%d P:%d M:%d U:%d", pagina->nro_marco,pagina->presencia,pagina->modificado,pagina->uso);

		return pagina;
}

t_marco* obtener_marco_libre() {
    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			marco->libre = false; //mutex
			return marco; 
		}
    }

    /* 
    t_marco* marco = pedir_frame_swap(); 
    */

	return NULL;
}

uint32_t cant_marcos_necesarios(uint32_t tamanio) {
	div_t nro_marcos = div(tamanio, config_memoria.tamanio_pagina); 
	uint32_t nro_marcos_q = nro_marcos.quot;

	if(nro_marcos.rem > 0) nro_marcos_q++;
	return nro_marcos_q;
}

bool tengo_marcos_suficientes(uint32_t necesarios){
	// no entiendo si yo aca devuelvo que tengo marcos necesarios pero despues mientras le asigno me quedo sin marcos porque los uso otro proceso que pasa?
    uint32_t contador_necesarios = necesarios;

    for(int i = 0; i < config_memoria.cant_marcos; i++) {
        t_marco* marco = memoria_ram.mapa_fisico[i];
		if (marco->libre) {
			contador_necesarios--;
		}
        if(contador_necesarios == 0) return true;
    }

    //seguir buscando en swap

    return false;
}

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t nro_pagina = pagina_segun_posicion(dir_logica - TAMANIO_HEAP);
	uint32_t offset_main = offset_segun_posicion(dir_logica - TAMANIO_HEAP);

	t_marco* marco = obtener_marco(id_carpincho, nro_pagina);

	// Verifico que el free es válido
	if(get_isFree(marco->nro_real, offset_main))
		return false;

	// Coloco el bit esFree del heap en 1 (ahora está libre)
	set_isFree(marco->nro_real, offset_main);

	// UNIFICO EL NUEVO FREE CON LOS ADYACENTES
	uint32_t pos_final_free = dir_logica;	// posicion final del nextAlloc

	// BUSCO EL ALLOC SIGUIENTE y, si corresponde, actualizo el nextAlloc del primero
	uint32_t pos_alloc_siguiente = get_nextAlloc(marco->nro_real, offset_main);
	if(HEAP_NULL != pos_alloc_siguiente) {
		t_marco* marco_siguiente = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_siguiente));
		uint32_t offset_next = offset_segun_posicion(pos_alloc_siguiente);

		if(get_isFree(marco_siguiente->nro_real, offset_next)) {
			pos_final_free = get_nextAlloc(marco_siguiente->nro_real, offset_next);
			// debería obtener el marco inicial de vuelta porque la incorporación del segundo
			// pudo deberse a la salida del primero
			marco = obtener_marco(id_carpincho, nro_pagina);
			set_nextAlloc(marco->nro_real, offset_main, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}


	// BUSCO EL ALLOC ANTERIOR y, si corresponde, le actualizo el nextAlloc
	uint32_t pos_alloc_anterior = get_prevAlloc(marco->nro_real, offset_main);
	if(HEAP_NULL != pos_alloc_anterior) {
		t_marco* marco_anterior = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_anterior));
		uint32_t offset_prev = offset_segun_posicion(pos_alloc_siguiente);

		if(get_isFree(marco_anterior->nro_real, offset_prev)) {
			// debería obtener el marco inicial de vuelta porque la incorporación del segundo
			// pudo deberse a la salida del primero
			marco = obtener_marco(id_carpincho, pagina_segun_posicion(pos_alloc_anterior));
			set_nextAlloc(marco_anterior->nro_real, offset_prev, pos_final_free);
			// soltar_marco()
		}
		else {
			// Hacer nada, creeo
		}
	}
	return true;
}

uint32_t pagina_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.quot;
}

uint32_t offset_segun_posicion(uint32_t posicion) {
	div_t div_posicion = div(posicion, config_memoria.tamanio_pagina);
	return div_posicion.rem;
}

t_marco *obtener_marco(uint32_t id_carpincho, uint32_t nro_pagina) {
	// el marco hay que reservarlo para que no lo saquen a memoria secundaria
	// durante el proceso
	// lo mismo con los marcos que se pidan para inificar el heapMetaData

	// uint32_t nro_marcos = config_memoria.tamanio_memoria / config_memoria.tamanio_pagina;
	t_marco* marco;
	t_carpincho* mi_carpincho = carpincho_de_lista(id_carpincho);
	// TODO: cambiar a lista
	t_entrada_tp *entrada_tp = mi_carpincho->tabla_paginas[nro_pagina];
	if(entrada_tp->id_carpincho == id_carpincho && entrada_tp->nro_pagina == nro_pagina)
		marco = memoria_ram.mapa_fisico[entrada_tp->nro_marco];
	else {
		// Page fault
		// t_marco* marco_a_quitar = realizar_algoritmo_reemplazo(id_carpincho);
		// TODO marco = reasignar_marco(id_carpincho, nro_pagina, marco_a_quitar);
	}
	return marco;
}

t_carpincho *carpincho_de_lista(uint32_t id_carpincho) {
	bool mi_carpincho(void *un_carpincho) {
		if(((t_carpincho *)un_carpincho)->id == id_carpincho)
			return true;
		else
			return false;
	}

	return list_find(lista_carpinchos, mi_carpincho);
}

bool get_isFree(uint32_t nro_marco, uint32_t desplazamiento) {
	uint8_t bit_esFree;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(&bit_esFree, memoria_ram.inicio + desplazamiento_memoria + 8, 1);
	if(bit_esFree)
		return true;
	else
		return false;
}

void set_isFree(uint32_t nro_marco, uint32_t desplazamiento) {
	uint8_t bit_esFree = 1;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(memoria_ram.inicio + desplazamiento_memoria + 8, &bit_esFree, 1);
}

void reset_isFree(uint32_t nro_marco, uint32_t desplazamiento) {
	uint8_t bit_esFree = 0;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(memoria_ram.inicio + desplazamiento_memoria + 8, &bit_esFree, 1);
}

uint32_t get_prevAlloc(uint32_t nro_marco, uint32_t desplazamiento) {
	uint32_t pos_alloc;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(&pos_alloc, memoria_ram.inicio + desplazamiento_memoria + 0, sizeof(uint32_t));

	return pos_alloc;
}

uint32_t get_nextAlloc(uint32_t nro_marco, uint32_t desplazamiento) {
	uint32_t pos_alloc;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(&pos_alloc, memoria_ram.inicio + desplazamiento_memoria + 4, sizeof(uint32_t));

	return pos_alloc;
}

void set_prevAlloc(uint32_t nro_marco, uint32_t desplazamiento, uint32_t nuevo_valor) {
	uint32_t data = nuevo_valor;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(memoria_ram.inicio + desplazamiento_memoria + 0, &data, sizeof(uint32_t));
}

void set_nextAlloc(uint32_t nro_marco, uint32_t desplazamiento, uint32_t nuevo_valor) {
	uint32_t data = nuevo_valor;
	uint32_t desplazamiento_memoria = nro_marco * config_memoria.tamanio_pagina + desplazamiento;
	memcpy(memoria_ram.inicio + desplazamiento_memoria + 4, &data, sizeof(uint32_t));
}

// TODO soltar_marco()
// TODO reservar_marco()

