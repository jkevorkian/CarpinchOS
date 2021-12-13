#include "manejo_paginas.h"

void crear_tabla(char tabla_paginas[][cantidad_total_paginas]){ //guarda los numeros de pagina guardadas en cada marco de cada archivo, se crea 1 por archivo
	log_info(logger, "Inicializando tabla de paginas");

		printf("La tabla de paginas es:\n");
		for(int i = 0; i < cantidad_total_paginas; i++){ //inicializo la tabla de paginas en 0 (todas las pag desocupadas)
			tabla_paginas[0][i] = 'V';//N°Particion (V = VACIO)
			tabla_paginas[1][i] = 'V';//N°Carpincho
			tabla_paginas[2][i] = 'V';//N°Pagina
		}
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
    int aux = 0;
    for(lugar_libre = (n_particion * paginas_por_particion); tabla_paginas[0][lugar_libre] == itoc(n_particion) && aux == 0; lugar_libre++){
        if(tabla_paginas[1][lugar_libre] == itoc(id_carpincho) && tabla_paginas[2][lugar_libre] == 'V'){
            aux++;
            tabla_paginas[2][lugar_libre] = itoc(n_pagina);
            return;
        }
    }
}

int proximo_marco_libre(char tabla_paginas[][cantidad_total_paginas],int n_particion){
	int lugar_libre;
	for(lugar_libre = (n_particion * paginas_por_particion); (tabla_paginas[0][lugar_libre] == itoc(n_particion) && tabla_paginas[1][lugar_libre] != 'V'); lugar_libre++){
		if((lugar_libre + 1) == ((n_particion + 1) * paginas_por_particion)){
			log_warning(logger, "No hay mas lugar en la Particion");
			return -1;
		}
	}
	//log_warning(logger, "El Prox marco libre es %d",lugar_libre);
	return lugar_libre;
}

int prox_marco_carpincho(char tabla_paginas[][cantidad_total_paginas],int id_carpincho){
	int desplazamiento = -1;
	for(int i = 0; desplazamiento == -1; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] == 'V'){
			desplazamiento = i;
		}
	}
	log_warning(logger, "El Prox marco libre es %d",desplazamiento);
	return desplazamiento;
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

char* obtener_pagina(int id_carpincho, int n_pagina,char tabla_paginas[][cantidad_total_paginas], void** particiones){
	char* mensaje = malloc(pagina_size);
	int desplazamiento = carpincho_existente(tabla_paginas, id_carpincho);
	int n_particion = (int)(desplazamiento / paginas_por_particion);
	desplazamiento = existe_pagina(id_carpincho, n_pagina, n_particion, tabla_paginas);

	if(desplazamiento == -1){
		log_info(logger, "La pagina no existe");
		return "ERROR";
	}
	desplazamiento -= (n_particion * paginas_por_particion);
	//log_info(logger, "El Desplazamiento es %d",desplazamiento);
		log_info(logger, "Leyendo pagina %d del carpincho %d en la Particion %d",n_pagina,id_carpincho,n_particion);
			memcpy(mensaje,(particiones[n_particion] + (desplazamiento * pagina_size)),pagina_size);
		//log_info(logger, "El mensaje es: %s",mensaje);
	return mensaje;
}

int existe_pagina(int id_carpincho, int n_pagina, int n_particion, char tabla_paginas[][cantidad_total_paginas]){ //devuelve el desplazamiento a la pag ya existente o -1 si no existe
	int desplazamiento = -1;
	log_info(logger, "Buscando si la Pagina %d del Carpincho %d ya existe",n_pagina, id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] == itoc(n_pagina)){
			//desplazamiento = (i - (n_particion * paginas_por_particion));
			desplazamiento = i;
		}
	}
	return desplazamiento;
}

void eliminar_pagina(char tabla_paginas[][cantidad_total_paginas], int id_carpincho, int n_pagina, void** particiones){
	int desplazamiento;
	int n_particion;

	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] == itoc(n_pagina)){
			log_info(logger, "Eliminando pagina %d del carpincho %d de memoria...",n_pagina, id_carpincho);
			n_particion = (int)(i / paginas_por_particion);
			desplazamiento = (i - (n_particion * pagina_size)) * pagina_size;
			borrar_contenido_pagina(particiones[n_particion], desplazamiento);
			log_info(logger, "Eliminando pagina %d del carpincho %d de la tabla...",n_pagina, id_carpincho);
			tabla_paginas[2][i] = 'V';
			//espacio_disponible[n_particion] += pagina_size;
		}
	}
	log_info(logger, "Pagina eliminada completamente");
}

void borrar_contenido_pagina(void* particion, int desplazamiento){
	for(int caracteres_eliminados = 0; caracteres_eliminados < pagina_size; caracteres_eliminados++, desplazamiento++){
			memcpy(particion + desplazamiento, "\0", sizeof(char));
		}
}

int reservar_marcos(int id_carpincho, int cantidad_marcos, char tabla_paginas[][cantidad_total_paginas], void** particiones){
	int n_particion;
	int desplazamiento = carpincho_ya_existia(tabla_paginas, id_carpincho);
	//printf("el desplazamiento es %d",desplazamiento);
	if(desplazamiento == -1){ //si no existe el carpincho en ninguna particion
		void* particion = algoritmo_de_particiones(particiones, espacio_disponible);
		n_particion = obtener_n_particion(particiones, particion);
		log_info(logger, "Guardando al Carpincho %d en la Particion %d",id_carpincho, n_particion);
	}
	else{
		n_particion = (int)(desplazamiento / paginas_por_particion);
		printf("El Carpincho ya se encontraba en la Particion %d\n",n_particion);
	}
	if(tipo_asignacion == FIJA){
		if(cantidad_marcos > marcos_maximos){
			log_warning(logger, "Se pidieron mas marcos que el maximo");
			return -1;
		}
		reservar_marcos_fijo(id_carpincho, n_particion, tabla_paginas);
	}
	else
	{
		int i = reservar_marcos_global(id_carpincho, n_particion, cantidad_marcos, tabla_paginas);
		if(i < 0){
		return -1;
		}
	}
	return 1;
}

void reservar_marcos_fijo(int id_carpincho, int n_particion, char tabla_paginas[][cantidad_total_paginas]){
	log_info(logger, "Reservando marcos para el carpincho %d",id_carpincho);
	int lugar_libre;
	for(lugar_libre = (n_particion * paginas_por_particion); (tabla_paginas[0][lugar_libre] == itoc(n_particion) && tabla_paginas[1][lugar_libre] != 'V'); lugar_libre++){
			if((lugar_libre + 1) == ((n_particion + 1) * paginas_por_particion)){
				log_warning(logger, "No hay mas lugar en la Particion");
				return;
			}
		}
	int marcos = 0;
	//int i;
	for(int i = lugar_libre, aux = 0; aux < marcos_maximos; i++, aux++){
		tabla_paginas[1][i] = itoc(id_carpincho);
		marcos = aux;
	}
	espacio_disponible[n_particion] -= pagina_size * (marcos + 1);
	log_info(logger, "Marcos Reservados");
}

int reservar_marcos_global(int id_carpincho, int n_particion, int cantidad_marcos, char tabla_paginas[][cantidad_total_paginas]){
	log_info(logger, "Reservando %d marcos para el carpincho %d",cantidad_marcos, id_carpincho);
	int lugar_libre;

	int desplazamiento = n_particion * paginas_por_particion;
	int marcos_libres = 0;
	for(int i = desplazamiento; i < desplazamiento + paginas_por_particion; i++){
		if(tabla_paginas[1][i] == 'V'){
			marcos_libres++;
		}
	}
	log_info(logger, "Marcos disponibles %d",marcos_libres);
	if(marcos_libres < cantidad_marcos){
		log_warning(logger, "No entra en la Particion");
		return -1;
	}
	for(lugar_libre = (n_particion * paginas_por_particion); (tabla_paginas[0][lugar_libre] == itoc(n_particion) && tabla_paginas[1][lugar_libre] != 'V'); lugar_libre++);
	printf("Lugar libre = %d\n", lugar_libre);
	int marcos_asignados = 0;
	for(int i = lugar_libre, aux = 0; aux < cantidad_marcos; i++, aux++){
		if(tabla_paginas[1][i] == 'V'){
			tabla_paginas[1][i] = itoc(id_carpincho);
			marcos_asignados = aux;
		}
		else
			cantidad_marcos++;
	}
	espacio_disponible[n_particion] -= (pagina_size * (marcos_asignados + 1));
	log_info(logger, "Marcos Reservados");
	return 1;
}

int ultima_pagina_carpincho(int n_carpincho,char tabla_paginas[][cantidad_total_paginas]){
	int ultima_pagina;
	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(n_carpincho) && tabla_paginas[2][i] != 'V'){
			ultima_pagina = ctoi(tabla_paginas[2][i]);
		}
	}
	return ultima_pagina;
}



