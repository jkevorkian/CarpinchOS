#include "manejo_paginas.h"

void crear_tabla(char tabla_paginas[][cantidad_total_paginas]){ //guarda los numeros de pagina guardadas en cada marco de cada archivo, se crea 1 por archivo
	log_info(logger, "Inicializando tabla de paginas");

		printf("La tabla de paginas es:\n");
		for(int i = 0; i < cantidad_total_paginas; i++){ //inicializo la tabla de paginas en 0 (todas las pag desocupadas)
			tabla_paginas[0][i] = 'V';//N°Particion(CAMBIAR POR \0, V = VACIO solo para pruebas)
			tabla_paginas[1][i] = 'V';//N°Carpincho
			tabla_paginas[2][i] = 'V';//N°Pagina
			//printf("%c",tabla_paginas[0][i]);

		}//solo de prueba
		imprimir_tabla(tabla_paginas);
		log_info(logger, "Inicializacion de tabla de paginas completada");
}
void imprimir_tabla(char tabla_paginas[][cantidad_total_paginas]){
	printf("La tabla de paginas es:\n");
	for(int i = 0; i < cantidad_total_paginas; i++){
			printf("%c",tabla_paginas[0][i]);
	}printf("\n");

	for(int i = 0; i < cantidad_total_paginas; i++){
			printf("%c",tabla_paginas[1][i]);
	}printf("\n");

	for(int i = 0; i < cantidad_total_paginas; i++){
			printf("%c",tabla_paginas[2][i]);
	}printf("\n");
}

void cargar_particion_en_tabla(char tabla_paginas[][paginas_por_particion], int n_particion){
	log_info(logger, "Cargando Particion %d en la tabla", n_particion);
	int desplazamiento = (n_particion * paginas_por_particion);
	for(int i = desplazamiento; i < paginas_por_particion + desplazamiento; i++){
		tabla_paginas[0][i] = itoc(n_particion);
	}

	imprimir_tabla(tabla_paginas);
}


void cargar_pagina_en_tabla(char tabla_paginas[][cantidad_total_paginas],int n_particion, int id_carpincho, int n_pagina){
	int lugar_libre;
	log_info(logger, "Cargando Pagina %d del Carpincho %d en la tabla", n_pagina, id_carpincho);
	for(lugar_libre = (n_particion * paginas_por_particion); (tabla_paginas[0][lugar_libre] == itoc(n_particion) && tabla_paginas[1][lugar_libre] != 'V'); lugar_libre++){
		if((lugar_libre + 1) == ((n_particion + 1) * paginas_por_particion)){
			log_warning(logger, "No hay mas lugar en la Particion");
			return;
		}

	}
	tabla_paginas[1][lugar_libre] = itoc(id_carpincho);
	tabla_paginas[2][lugar_libre] = itoc(n_pagina);
	espacio_disponible[n_particion] -= pagina_size;
}

int proximo_marco_libre(char tabla_paginas[][cantidad_total_paginas],int n_particion){
	int lugar_libre;

	for(lugar_libre = (n_particion * paginas_por_particion); (tabla_paginas[0][lugar_libre] == itoc(n_particion) && tabla_paginas[1][lugar_libre] != 'V'); lugar_libre++){
		if((lugar_libre + 1) == ((n_particion + 1) * paginas_por_particion)){
			log_warning(logger, "No hay mas lugar en la Particion");
			return -1;
		}
	}
	return lugar_libre;
}

void guardar_pagina_en_memoria(char* mensaje, int desplazamiento, void* particion){
	log_info(logger, "Guardando pagina en Particion");
	memcpy(particion + (desplazamiento * pagina_size),mensaje,pagina_size);
	log_info(logger, "Pagina guardada correctamente");

}

void leer_pagina(void* particion,int desplazamiento){
	char* mensaje = malloc(pagina_size);
	log_info(logger, "Leyendo pagina de Particion");
		memcpy(mensaje,(particion + (desplazamiento * pagina_size)),pagina_size);
	log_info(logger, "El mensaje es: %s",mensaje);
	free(mensaje);

}

int existe_pagina(int id_carpincho, int n_pagina, int n_particion, char tabla_paginas[][cantidad_total_paginas]){ //devuelve el desplazamiento a la pag ya existente o -1 si no existe
	int desplazamiento = -1;
	log_info(logger, "Buscando si la Pagina %d del Carpincho %d ya existe",n_pagina, id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] == itoc(n_pagina)){
			desplazamiento = (i - (n_particion * paginas_por_particion));
		}
	}
	return desplazamiento;
}
