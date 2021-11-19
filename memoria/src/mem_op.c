#include "mem_op.h"

bool mem_free(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;

	// Coloco el bit esFree del heap en 1 (ahora está libre)
	set_isFree(id_carpincho, dir_logica_heap);

	// UNIFICO EL NUEVO FREE CON LOS ADYACENTES
	uint32_t pos_final_free = dir_logica;	// posicion final del nextAlloc

	// BUSCO EL ALLOC SIGUIENTE y, si corresponde, actualizo el nextAlloc del primero
	uint32_t pos_alloc_siguiente = get_nextAlloc(id_carpincho, dir_logica_heap);
	if(HEAP_NULL != pos_alloc_siguiente) {

		if(get_isFree(id_carpincho, pos_alloc_siguiente)) {
			pos_final_free = get_nextAlloc(id_carpincho, pos_alloc_siguiente);
			set_nextAlloc(id_carpincho, dir_logica_heap, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}

	// BUSCO EL ALLOC ANTERIOR y, si corresponde, le actualizo el nextAlloc
	uint32_t pos_alloc_anterior = get_prevAlloc(id_carpincho, dir_logica_heap);
	if(HEAP_NULL != pos_alloc_anterior) {

		if(get_isFree(id_carpincho, pos_alloc_anterior)) {
			set_nextAlloc(id_carpincho, pos_alloc_anterior, pos_final_free);
		}
		else {
			// Hacer nada, creeo
		}
	}
	return true;
}

uint32_t heap_header(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento);
uint32_t heap_footer(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento, uint32_t alloc_sig);

// TODO: cambiar bit modificado de pagina cuando escribo un heap metadata
uint32_t get_prev_alloc(t_carpincho* carpincho, uint32_t desplazamiento);
uint32_t get_next_alloc(t_carpincho* carpincho, uint32_t desplazamiento);
uint32_t get_is_free(t_carpincho* carpincho, uint32_t desplazamiento);
void set_prev_alloc(t_carpincho* carpincho, uint32_t desplazamiento, uint32_t valor);
void set_next_alloc(t_carpincho* carpincho, uint32_t desplazamiento, uint32_t valor);
void set_is_free(t_carpincho* carpincho, uint32_t desplazamiento, uint8_t valor);

uint32_t get_prev_alloc(t_carpincho* carpincho, uint32_t desplazamiento){
	uint32_t prev_alloc;
	memcpy(&prev_alloc, carpincho->heap_metadata + desplazamiento, sizeof(uint32_t));
	return prev_alloc;
}

uint32_t get_next_alloc(t_carpincho* carpincho, uint32_t desplazamiento){
	uint32_t next_alloc;
	memcpy(&next_alloc, carpincho->heap_metadata + desplazamiento + 4, sizeof(uint32_t));
	return next_alloc;
}

uint32_t get_is_free(t_carpincho* carpincho, uint32_t desplazamiento){
	uint8_t is_free;
	memcpy(&is_free, carpincho->heap_metadata + desplazamiento + 8, sizeof(uint8_t));
	return is_free;
}

void set_prev_alloc(t_carpincho* carpincho, uint32_t desplazamiento, uint32_t valor){
	uint32_t prev_alloc = valor;
	memcpy(carpincho->heap_metadata + desplazamiento, &prev_alloc, sizeof(uint32_t));
}

void set_next_alloc(t_carpincho* carpincho, uint32_t desplazamiento, uint32_t valor){
	uint32_t next_alloc = valor;
	memcpy(carpincho->heap_metadata + desplazamiento + 4, &next_alloc, sizeof(uint32_t));
}

void set_is_free(t_carpincho* carpincho, uint32_t desplazamiento, uint8_t valor){
	uint8_t is_free = valor;
	memcpy(carpincho->heap_metadata + desplazamiento + 8, &is_free, sizeof(uint8_t));
}

uint32_t mem_alloc(uint32_t id_carpincho, uint32_t tamanio) {
	log_info(logger, "El proceso #%d solicito %d bytes de memoria.", id_carpincho, tamanio);

	uint32_t nro_frames_necesarios = cant_marcos_necesarios(tamanio + 2*TAMANIO_HEAP);
	uint32_t dir_logica = 0;
	
	// esto esta mal porque podria tener un monton de marcos en swap
	if(config_memoria.tipo_asignacion == FIJA_LOCAL) {
		if(nro_frames_necesarios > config_get_int_value(config, "MARCOS_POR_CARPINCHO")) {
			return 0;
		}
	}

	t_carpincho* carpincho = carpincho_de_lista(id_carpincho);

	if(carpincho->heap_metadata == NULL) {
		carpincho->heap_metadata = dir_fisica_proceso(carpincho->tabla_paginas);
		dir_logica = heap_header(carpincho, tamanio, 0);
		heap_footer(carpincho, tamanio, dir_logica + tamanio, HEAP_NULL);
	}
	else {
		uint32_t desplazamiento = 0;
		uint32_t alloc_sig;
		uint8_t is_free;		

		while(true){
			alloc_sig = get_next_alloc(carpincho, desplazamiento);
			is_free = get_is_free(carpincho, desplazamiento);

			if(alloc_sig == HEAP_NULL){
				if(is_free) {
					if (desplazamiento + 2*TAMANIO_HEAP > config_get_int_value(config, "MARCOS_POR_CARPINCHO") * config_memoria.tamanio_pagina) {
						//no entra al final
						//con asig global asigno pagina de cualquier marco
						//con asig fija reemplazo de mis paginas. lo que hago seria:
						//*el marco victima mio pasa a swap
						//*escribo en una nueva pagina los nuevos datos usando el marco victima
					}
					dir_logica = heap_header(carpincho, tamanio, desplazamiento);
					heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
					break;
				};
				// siempre voy a tener un metadata free al final de todo o no?
			}
			else {
				if(is_free) {
					dir_logica = heap_header(carpincho, tamanio, desplazamiento);

					//no entra en ese espacio alocado, buscar otro
					if(dir_logica == 0) {
						desplazamiento = alloc_sig;
						continue;
					};

					heap_footer(carpincho, tamanio, dir_logica + tamanio, alloc_sig);
					break;
				};

			desplazamiento = alloc_sig;

			}
		}
	}	

	log_info(logger, "Direccion logica asignada al carpincho #%d: %d", id_carpincho, dir_logica);
	return dir_logica; 
}

uint32_t heap_header(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento){
	bool tiene_footer = get_next_alloc(carpincho, desplazamiento) != 0 || get_next_alloc(carpincho, desplazamiento) != HEAP_NULL;
	uint32_t bytes_allocados = get_next_alloc(carpincho, desplazamiento) - desplazamiento - TAMANIO_HEAP;

	// tiene que entrar justo en el tamaño de aloc libre o tiene que sobrar espacio para un nuevo heap footer
	if(tiene_footer){
		if(bytes_allocados == tam){
			//printf("Entra entero pero sin footer intermedio");
		}
		else if(bytes_allocados >= tam + TAMANIO_HEAP){
			//printf("Entra con footer intermedio");
		}
		else {
			//printf("No entra, buscar otro libre.");
			return 0;
		}
	}

	set_is_free(carpincho, desplazamiento, 0);
	set_next_alloc(carpincho, desplazamiento, TAMANIO_HEAP + tam + desplazamiento);

	log_info(logger, "Heap header creado en direccion logica: %d.", desplazamiento);
	log_info(logger, "%d %d %d", get_prev_alloc(carpincho, desplazamiento), get_next_alloc(carpincho, desplazamiento), get_is_free(carpincho, desplazamiento));

	return desplazamiento + TAMANIO_HEAP;
}

uint32_t heap_footer(t_carpincho* carpincho, uint32_t tam, uint32_t desplazamiento, uint32_t alloc_sig){
	set_is_free(carpincho, desplazamiento, 1);
	set_next_alloc(carpincho, desplazamiento, alloc_sig);
	set_prev_alloc(carpincho, desplazamiento, desplazamiento - tam - TAMANIO_HEAP);

	log_info(logger, "Heap footer creado en direccion logica: %d.", desplazamiento);
	log_info(logger, "%d %d %d", get_prev_alloc(carpincho, desplazamiento), get_next_alloc(carpincho, desplazamiento), get_is_free(carpincho, desplazamiento));

	return desplazamiento + TAMANIO_HEAP;
}

void *mem_read(uint32_t id_carpincho, uint32_t dir_logica) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;
	
	return obtener_bloque_paginacion(id_carpincho, dir_logica, tamanio_alocado);
}

bool mem_write(uint32_t id_carpincho, uint32_t dir_logica, void* contenido) {
	uint32_t dir_logica_heap = dir_logica - TAMANIO_HEAP;

	// Verifico que el free es válido
	if(get_isFree(id_carpincho, dir_logica_heap))
		return false;
	
	uint32_t tamanio_alocado = get_nextAlloc(id_carpincho, dir_logica_heap) - dir_logica;
	uint32_t tamanio_data = strlen(contenido);
	int32_t diferencia_tamanios = tamanio_alocado - strlen(contenido);

	if(diferencia_tamanios < 0)
		return false;

	void *data = malloc(tamanio_alocado);
	memcpy(data, contenido, tamanio_data);
	char relleno = '\0';
	
	if(diferencia_tamanios > 0) {
		while(diferencia_tamanios > 0) {
			memcpy(data + tamanio_alocado - diferencia_tamanios, &relleno, 1);
			diferencia_tamanios--;
		}
	}
	actualizar_bloque_paginacion(id_carpincho, dir_logica, data, tamanio_alocado);
	return true;
}
