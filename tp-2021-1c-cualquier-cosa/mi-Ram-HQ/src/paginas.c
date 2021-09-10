#include "paginas.h"

bool iniciar_memoria_paginada(t_config* config) {
	memoria_ram.esquema_memoria = PAGINACION;
	memoria_ram.tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
	memoria_ram.tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");

	uint32_t frames_en_memoria = memoria_ram.tamanio_memoria / TAMANIO_PAGINA;
	uint32_t frames_totales = (/*memoria_ram.tamanio_memoria + */memoria_ram.tamanio_swap) / TAMANIO_PAGINA;
	if(memoria_ram.tamanio_memoria % TAMANIO_PAGINA + memoria_ram.tamanio_swap % TAMANIO_PAGINA > 0)
		return false;
	
	log_info(logger, "Estoy en paginación, con entrada valida. Nro frames: %d:%d", frames_en_memoria, frames_totales);
	memoria_ram.mapa_fisico = calloc(frames_en_memoria, memoria_ram.tamanio_pagina);
	memoria_ram.mapa_logico = calloc(frames_totales, memoria_ram.tamanio_pagina);
	for(int i = 0; i < frames_totales; i++) {
		t_marco* marco_auxiliar = malloc(sizeof(t_marco));
		memoria_ram.mapa_logico[i] = marco_auxiliar;
		marco_auxiliar->nro_virtual = i;
		if(i < frames_en_memoria) {
			memoria_ram.mapa_fisico[i] = marco_auxiliar;
			marco_auxiliar->nro_real = i;
			marco_auxiliar->presencia = true;
		}
		else {
			marco_auxiliar->nro_real = 0;
			marco_auxiliar->presencia = false;
		}
		marco_auxiliar->duenio = 0;
		marco_auxiliar->modificado = false;
        marco_auxiliar->bit_uso = false;
		sem_init(&marco_auxiliar->semaforo_mutex, 0, 1);
	}
	if(!strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO"), "LRU"))
		memoria_ram.algoritmo_reemplazo = LRU;
	if(!strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO"), "CLOCK")) {
		memoria_ram.algoritmo_reemplazo = CLOCK;
		memoria_ram.puntero_clock = 0;
	}
		  
	memoria_ram.fd_swap = open(config_get_string_value(config, "PATH_SWAP"), O_RDWR);
	ftruncate(memoria_ram.fd_swap, memoria_ram.tamanio_swap);
    sem_init(&mutex_incorporar_marco, 0, 1);
    // memoria_ram.inicio_swap = mmap(NULL, memoria_ram.tamanio_swap, PROT_WRITE, MAP_PRIVATE, memoria_ram.fd_swap, 0);

	// char memoria_vacia[memoria_ram.tamanio_swap];
	// memset(memoria_vacia, 0, memoria_ram.tamanio_swap);
	// fwrite(memoria_vacia, 1, memoria_ram.tamanio_swap, memoria_ram.fd_swap);
	return true;
}

uint32_t marcos_logicos_disponibles() {
    div_t cantidad_compleja = div(memoria_ram.tamanio_swap, memoria_ram.tamanio_pagina);
    uint32_t cantidad_marcos_totales = cantidad_compleja.quot;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_logico[i]->duenio == 0)
            cantidad_marcos_disponibles++;
    }
    return cantidad_marcos_disponibles;
}

uint32_t marcos_reales_disponibles() {
    div_t cantidad_compleja = div(memoria_ram.tamanio_memoria, memoria_ram.tamanio_pagina);
    uint32_t cantidad_marcos_totales = cantidad_compleja.quot;
    uint32_t cantidad_marcos_disponibles = 0;
    
    for(int i = 0; i < cantidad_marcos_totales; i++) {
        if(memoria_ram.mapa_fisico[i]->duenio == 0)
            cantidad_marcos_disponibles++;
    }
    return cantidad_marcos_disponibles;
}

t_marco* obtener_marco_libre_fisico() {
    t_marco* marco_disponible;
    for(int i = 0; i < memoria_ram.tamanio_memoria / TAMANIO_PAGINA; i++) {
        if (memoria_ram.mapa_fisico[i]->duenio == 0) {
            marco_disponible = memoria_ram.mapa_fisico[i];
            // sem_wait(&marco_disponible->semaforo_mutex);
            break;
        }
    }
    return marco_disponible;
}

t_marco* obtener_marco_libre_virtual() {
    t_marco* marco_disponible;
    for(int i = 0; i < memoria_ram.tamanio_swap / TAMANIO_PAGINA; i++) {
        if (memoria_ram.mapa_logico[i]->duenio == 0) {
            marco_disponible = memoria_ram.mapa_logico[i];
            // sem_wait(&marco_disponible->semaforo_mutex);
            break;
        }
    }
    // log_info(logger, "Obtengo marco libre logico. Nros: %d/%d", marco_disponible->nro_virtual, marco_disponible->nro_real);
    return marco_disponible;
}

uint32_t frames_necesarios(uint32_t memoria_libre_ultimo_frame, uint32_t tamanio) {
    uint32_t cant_frames = (div(tamanio, TAMANIO_PAGINA)).quot;
    if((div(tamanio, TAMANIO_PAGINA)).rem > memoria_libre_ultimo_frame)
        cant_frames++;
    return cant_frames; 
}

void asignar_frames(uint32_t id_patota, uint32_t cant_frames) {
    if(cant_frames == 0)    return ;

    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    if(mi_patota->cant_frames > 0)
        mi_patota->frames = realloc(mi_patota->frames, (cant_frames + mi_patota->cant_frames) * sizeof(t_marco *));
    else
        mi_patota->frames = calloc(cant_frames, sizeof(t_marco *));
    
    uint32_t nro_frames_asignados = 0;

    while(cant_frames > nro_frames_asignados) {
        t_marco* nuevo_marco;
       if(marcos_reales_disponibles()) {
            log_info(logger, "Obtuve marco libre fisico");
            nuevo_marco = obtener_marco_libre_fisico();
        }
        else {
            log_info(logger, "Obtuve marco libre virtual");
            nuevo_marco = obtener_marco_libre_virtual();
        }
        if(nuevo_marco == NULL) log_warning(logger, "Obtuve marco NULL");
        log_info(logger, "Marcos libres: reales %d/ virtuales%d", marcos_reales_disponibles(), marcos_logicos_disponibles());
        nuevo_marco->duenio = id_patota;
        nro_frames_asignados++;
        mi_patota->frames[mi_patota->cant_frames] = nuevo_marco->nro_virtual;
        mi_patota->cant_frames++;
        log_info(logger, "Asigno frame. Cant frames de la patota: %d", mi_patota->cant_frames);
    }
    log_info(logger, "Asigne frames");
}

void borrar_marco(uint32_t nro_marco) {
    t_marco* marco_descartable = memoria_ram.mapa_logico[nro_marco];
    marco_descartable->modificado = false;
    marco_descartable->duenio = 0;
}

void* inicio_marco(uint32_t nro_marco) {
    t_marco* mi_marco = memoria_ram.mapa_logico[nro_marco];
    return memoria_ram.inicio + mi_marco->nro_real * TAMANIO_PAGINA;
}

uint32_t inicio_marco_logico(uint32_t nro_marco) {
    t_marco* mi_marco = memoria_ram.mapa_logico[nro_marco];
    return mi_marco->nro_real * TAMANIO_PAGINA;
}

void hacer_backup_marco(uint32_t nro_marco_fisico) {
    t_marco* mi_marco = memoria_ram.mapa_fisico[nro_marco_fisico];
    lseek(memoria_ram.fd_swap, mi_marco->nro_virtual * TAMANIO_PAGINA, SEEK_SET);
    write(memoria_ram.fd_swap, inicio_marco(mi_marco->nro_virtual), TAMANIO_PAGINA);
}

void incorporar_marco(uint32_t nro_marco) {
    t_marco* marco_incorporado = memoria_ram.mapa_logico[nro_marco];
    t_marco* marco_descartable;
    if(marco_incorporado->presencia == true) {  // EL MARCO YA ESTA EN MEMORIA
        return ;
    }
    sem_wait(&mutex_incorporar_marco);
    log_info(logger, "Incorporo marco");
    if(marcos_reales_disponibles()) {   // HAY MARCO LIBRE EN MEMORIA
        marco_descartable = obtener_marco_libre_fisico();
    }
    else {  // HAY QUE HACER REEMPLAZO PARA INCORPORAR MARCO A MEMORIA
        if(memoria_ram.algoritmo_reemplazo == LRU) {
            log_info(logger, "Entro a reemplazo por LRU");
            marco_descartable = reemplazo_por_lru(nro_marco);
        }
        if(memoria_ram.algoritmo_reemplazo == CLOCK) {
            log_info(logger, "Entro a reemplazo por clock");
            marco_descartable = reemplazo_por_clock(nro_marco);
        }
    }
    log_info(logger, "Marco a quitar. Nro virtual: %d, nro Real: %d.", marco_descartable->nro_virtual, marco_descartable->nro_real);
    if(marco_descartable->modificado) {
        log_info(logger, "Hago backup del marco");
        hacer_backup_marco(marco_descartable->nro_virtual);
    }
    log_info(logger, "Modifico marco viejo y actualizo nuevo");
    marco_descartable->presencia = false;
    marco_descartable->modificado = false;
    marco_incorporado->nro_real = marco_descartable->nro_real;
    marco_incorporado->presencia = true;
    sem_post(&marco_descartable->semaforo_mutex);

    lseek(memoria_ram.fd_swap, marco_incorporado->nro_virtual * TAMANIO_PAGINA, SEEK_SET);
    read(memoria_ram.fd_swap, inicio_marco(marco_incorporado->nro_virtual), TAMANIO_PAGINA);

    sem_post(&mutex_incorporar_marco);
    // loggear_marcos_logicos(NULL);
}

t_marco* reemplazo_por_lru(uint32_t nro_marco) {
    return NULL;
}

t_marco* reemplazo_por_clock(uint32_t nro_marco) {
    bool encontre_marco = false;
    t_marco* marco_encontrado;
    while(!encontre_marco) {
        marco_encontrado = memoria_ram.mapa_fisico[memoria_ram.puntero_clock];
        sem_wait(&marco_encontrado->semaforo_mutex);
        if(marco_encontrado->bit_uso) {
            marco_encontrado->bit_uso = 0;
            sem_post(&marco_encontrado->semaforo_mutex);
        }
        else {
            encontre_marco = true;
        }
        memoria_ram.puntero_clock++;
        if(memoria_ram.puntero_clock == memoria_ram.tamanio_memoria / memoria_ram.tamanio_pagina)
            memoria_ram.puntero_clock = 0;
    }
    return marco_encontrado;
}

// uint32_t* valor_entero;
    // char* valor_char;
    // switch(tipo) {
    //     case ENTERO:
    //         valor_entero = malloc(sizeof(uint32_t));
    //         *valor_entero = (void *)valor;
    //         bytes_necesarios = sizeof(uint32_t);
    //         data = valor_entero;
    //         break;
    //     case CARACTER:
    //         valor_char = malloc(sizeof(char));
    //         *valor_char = (void *)valor;
    //         bytes_necesarios = sizeof(char);
    //         data = valor_char;
    //         break;
    //     case BUFFER:
    //         // data = valor;
    //         bytes_necesarios = strlen(valor);
    //         data = realloc(valor, bytes_necesarios);
    //         break;
    // }

void actualizar_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento, uint32_t valor) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, TAMANIO_PAGINA);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = sizeof(uint32_t);

    uint32_t data = valor;
    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        
        sem_wait(&marco_auxiliar->semaforo_mutex);
        incorporar_marco(mi_patota->frames[pagina_actual]);
        memcpy(inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, &data, bytes_disponibles);
        marco_auxiliar->modificado = true;
        marco_auxiliar->bit_uso = true;
        sem_post(&marco_auxiliar->semaforo_mutex);
        log_warning(logger, "Incorpore marco. Obtengo dato paginacion");
        log_info(logger, "Data: %d", obtener_entero_paginacion(id_patota, desplazamiento));
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
    // log_info(logger, "Dato nuevo %d. Obtenido por funcion %d", data, obtener_entero_paginacion(id_patota, desplazamiento));
}

uint32_t obtener_entero_paginacion(uint32_t id_patota, uint32_t desplazamiento) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, TAMANIO_PAGINA);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = sizeof(uint32_t);
    // log_info(logger, "obtener_entero. Pagina %d, inicio_pagina: %d", pagina_actual, posicion_compuesta.rem);

    uint32_t data;

    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        // log_info(logger, "Bytes cargados %d, Bytes necesarios %d", bytes_cargados, bytes_necesarios);
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        sem_wait(&marco_auxiliar->semaforo_mutex);
        incorporar_marco(mi_patota->frames[pagina_actual]);
        memcpy(&data, inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, bytes_disponibles);
        marco_auxiliar->bit_uso = true;
        sem_post(&marco_auxiliar->semaforo_mutex);
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
    // log_info(logger, "Dato obtenido: %d", data);
    return data;
}

void* obtener_bloque_paginacion(uint32_t id_patota, uint32_t desplazamiento, uint32_t tamanio) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, TAMANIO_PAGINA);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = tamanio;
    log_info(logger, "actualizar_entero. Pagina %d, inicio_pagina: %d", pagina_actual, posicion_compuesta.rem);

    void* data = malloc(tamanio);
    log_info(logger, "Tamanio bloque %d, tamanio data %d", tamanio, sizeof(*data));
    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        log_info(logger, "Bytes cargados %d, Bytes necesarios %d", bytes_cargados, bytes_necesarios);
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        // log_info(logger, "Pagina actual %d. Frames de patota %d. Tamanio vector %d", pagina_actual, mi_patota->cant_frames, sizeof(mi_patota->frames[0]));
        
        sem_wait(&marco_auxiliar->semaforo_mutex);
        incorporar_marco(mi_patota->frames[pagina_actual]);
        memcpy(data, inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, bytes_disponibles);
        marco_auxiliar->modificado = true;
        marco_auxiliar->bit_uso = true;
        sem_post(&marco_auxiliar->semaforo_mutex);
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
    return data;
}

void actualizar_bloque_paginacion(uint32_t id_patota, uint32_t desplazamiento, void* data, uint32_t tamanio) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    div_t posicion_compuesta = div(desplazamiento, TAMANIO_PAGINA);
    uint32_t bytes_cargados = 0;
    uint32_t bytes_disponibles = TAMANIO_PAGINA - posicion_compuesta.rem;
    uint32_t pagina_actual = posicion_compuesta.quot;
    uint32_t bytes_necesarios = tamanio;
    // log_info(logger, "actualizar_entero. Pagina %d, inicio_pagina: %d", pagina_actual, posicion_compuesta.rem);

    // void* data = malloc(tamanio);
    uint32_t inicio_pagina = posicion_compuesta.rem;
    while(bytes_cargados < bytes_necesarios) {
        t_marco* marco_auxiliar = memoria_ram.mapa_logico[mi_patota->frames[pagina_actual]];
        // log_info(logger, "Bytes cargados %d, Bytes necesarios %d", bytes_cargados, bytes_necesarios);
        if(bytes_disponibles > bytes_necesarios - bytes_cargados)
            bytes_disponibles = bytes_necesarios - bytes_cargados;
        // log_info(logger, "Pagina actual %d. Frames de patota %d. Tamanio vector %d", pagina_actual, mi_patota->cant_frames, sizeof(mi_patota->frames[0]));
        
        sem_wait(&marco_auxiliar->semaforo_mutex);
        incorporar_marco(mi_patota->frames[pagina_actual]);
        // log_info(logger, "Inicio de marco que actualizo %p. Data: %d. Bytes_disponibles: %d", inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, data, bytes_disponibles);
        // log_info(logger, "Itero entero. Pagina %d, inicio_pagina: %d. Bytes a cargar: %d", pagina_actual, inicio_pagina, bytes_disponibles);
        memcpy(inicio_marco(mi_patota->frames[pagina_actual]) + inicio_pagina, data, bytes_disponibles);
        marco_auxiliar->modificado = true;
        marco_auxiliar->bit_uso = true;
        sem_post(&marco_auxiliar->semaforo_mutex);
        
        bytes_cargados += bytes_disponibles;
        bytes_disponibles = TAMANIO_PAGINA;
        inicio_pagina = 0;
        pagina_actual++;
    }
}

void reasignar_frames(uint32_t id_patota) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, id_patota - 1);
    while(frames_necesarios(0, mi_patota->memoria_ocupada) < mi_patota->cant_frames) {
		borrar_marco(mi_patota->frames[mi_patota->cant_frames - 1]);
		mi_patota->cant_frames--;
		mi_patota->frames = realloc(mi_patota->frames, mi_patota->cant_frames * sizeof(uint32_t *));
	}
}

uint32_t nro_pagina_de_patota(uint32_t patota, uint32_t nro_marco) {
    patota_data* mi_patota = (patota_data *)list_get(lista_patotas, patota - 1);
    for(int i = 0; i < mi_patota->cant_frames; i++) {
        if(mi_patota->frames[i] == memoria_ram.mapa_fisico[nro_marco]->nro_virtual)
            return i + 1;
    }
    return 0;
}