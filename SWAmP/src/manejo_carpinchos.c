#include "manejo_carpinchos.h"


void recibir_carpincho(int id_carpincho, int n_pagina, char* mensaje_pagina, char tabla_paginas[][cantidad_total_paginas], void** particiones){//revisar nombre
	int desplazamiento;
	int marcos_disponibles;
	int n_particion;
	void* particion;

	log_info(logger, "Recibi una pagina del Carpincho %d",id_carpincho);

	desplazamiento = carpincho_existente(tabla_paginas, id_carpincho);
	if(desplazamiento == -1){ //si no existe el carpincho en ninguna particion
		particion = algoritmo_de_particiones(particiones, espacio_disponible);
		n_particion = obtener_n_particion(particiones, particion);
		desplazamiento = proximo_marco_libre(tabla_paginas, n_particion);
		desplazamiento = desplazamiento - (n_particion * pagina_size);
		guardar_pagina_en_memoria(mensaje_pagina, desplazamiento , particion);
		cargar_pagina_en_tabla(tabla_paginas, n_particion, id_carpincho, n_pagina);
		log_info(logger, "Pagina cargada con exito");
	}
	else{
		n_particion = (int)(desplazamiento / paginas_por_particion);
		printf("El Carpincho ya se encontraba en la Particion %d",n_particion);
		desplazamiento = existe_pagina(id_carpincho, n_pagina, n_particion, tabla_paginas);//Reviso si ya existia esa pagina
		if(desplazamiento == -1){
			log_info(logger, "Pagina no encontrada");
			marcos_disponibles = carpincho_tiene_marcos_disponibles(tabla_paginas, id_carpincho);
			if(marcos_disponibles > 0){
				desplazamiento = proximo_marco_libre(tabla_paginas, n_particion);
				desplazamiento = desplazamiento - (n_particion * pagina_size);
				cargar_pagina_en_tabla(tabla_paginas, n_particion, id_carpincho, n_pagina);
			}
			else{
				log_info(logger, "El carpincho no tiene mas marcos disponibles");
				return; //no puede escribir mas paginas
			}
		}
		else{
			log_info(logger, "La Pagina ya existia, sobre-escribiendola...");
		}
		guardar_pagina_en_memoria(mensaje_pagina, desplazamiento , particiones[n_particion]);

		log_info(logger, "Pagina cargada con exito");
	}
}

void eliminar_carpincho(int id_carpincho, char tabla_paginas[][cantidad_total_paginas], void** particiones){

	int n_particion;
	int desplazamiento = carpincho_existente(tabla_paginas, id_carpincho);
	if(desplazamiento == -1){
		log_info(logger, "El Carpincho %d no existe, vaquero",id_carpincho);
		return;
	}
	else{
		log_info(logger, "Finalizando Carpincho...");
		n_particion = (int)(desplazamiento / paginas_por_particion);
		for(int i = 0; i < cantidad_total_paginas; i++){
			if(tabla_paginas[1][i] == itoc(id_carpincho)){
				desplazamiento = i - (n_particion * pagina_size);
				int caracteres_por_eliminar = pagina_size;
				log_info(logger, "Eliminando pagina de memoria...");
				desplazamiento = desplazamiento*pagina_size;
				while(caracteres_por_eliminar > 0){
					memcpy(particiones[n_particion] + desplazamiento, "\0", sizeof(char));
					desplazamiento++;
					caracteres_por_eliminar--;
				}log_info(logger, "Eliminando pagina de la tabla...");
				tabla_paginas[1][i] = 'V';
				tabla_paginas[2][i] = 'V';
				espacio_disponible[n_particion] += pagina_size;
			}
		}

	}
	log_info(logger, "Carpincho eliminado completamente :C");
}

int carpincho_existente(char tabla_paginas[][cantidad_total_paginas], int id_carpincho){
	log_info(logger, "Buscando si el carpincho %d existe",id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho)){
			log_info(logger, "Carpincho encontrado");
			return i; //si lo encontro devuelve la posicion en la tabla de la primera pagina del carpincho
		}
	}
	log_info(logger, "Carpincho no encontrado");
	return -1; //si no encontro el carpincho en la tabla (primera vez en la SWAP) devuelve -1
}

int carpincho_tiene_marcos_disponibles(char tabla_paginas[][cantidad_total_paginas], int id_carpincho){
	int marcos_ocupados = 0;
	log_info(logger, "Buscando si el carpincho %d tiene marcos disponibles", id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
			if(tabla_paginas[1][i] == itoc(id_carpincho)){
				marcos_ocupados++;
			}
		}
	if(marcos_ocupados < marcos_maximos){
		log_info(logger, "Aun tiene %d lugares",(marcos_maximos - marcos_ocupados));
		return (marcos_maximos - marcos_ocupados);
	}
	else
		log_info(logger, "Ya uso todos los marcos");
		return 0;
}
