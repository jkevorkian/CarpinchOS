#include "manejo_carpinchos.h"


int recibir_carpincho(int id_carpincho, int n_pagina, char* mensaje_pagina, char tabla_paginas[][cantidad_total_paginas], void** particiones){//revisar nombre
	int desplazamiento;
	int marcos_disponibles;
	int n_particion;

	log_info(logger, "Recibi una pagina del Carpincho %d",id_carpincho);
		desplazamiento = carpincho_ya_existia(tabla_paginas, id_carpincho);
		n_particion = (int)(desplazamiento / paginas_por_particion);
		desplazamiento = existe_pagina(id_carpincho, n_pagina, n_particion, tabla_paginas);//Reviso si ya existia esa pagina
		if(desplazamiento == -1){
			log_info(logger, "Pagina no encontrada");
			marcos_disponibles = carpincho_tiene_marcos_disponibles(tabla_paginas, id_carpincho);
			if(marcos_disponibles > 0){
				desplazamiento = prox_marco_carpincho(tabla_paginas, id_carpincho);
				//desplazamiento = desplazamiento - (n_particion * pagina_size);
				//log_info(logger,"El desplazamiento es %d",desplazamiento);
				cargar_pagina_en_tabla(tabla_paginas, n_particion, id_carpincho, n_pagina);
			}
			else{
				log_info(logger, "El carpincho no tiene mas marcos disponibles");
				return -1; //no puede escribir mas paginas
			}

		}
		else{
			log_info(logger, "La Pagina ya existia, sobre-escribiendola...");
		}
		//log_info(logger,"El desplazamiento es %d",desplazamiento);
		desplazamiento = desplazamiento - (n_particion * paginas_por_particion);
		//log_info(logger,"El desplazamiento es %d",desplazamiento);
		guardar_pagina_en_memoria(mensaje_pagina, desplazamiento , particiones[n_particion]);

		log_info(logger, "Pagina cargada con exito");
		return 1;
	}

void eliminar_carpincho(int id_carpincho, char tabla_paginas[][cantidad_total_paginas], void** particiones){
	int n_pagina;

		log_info(logger, "Finalizando Carpincho...");
		for(int i = 0; i < cantidad_total_paginas; i++){
			if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] != 'V'){
				n_pagina = ctoi(tabla_paginas[2][i]);
				eliminar_pagina(tabla_paginas, id_carpincho, n_pagina, particiones);
			}
			if(tabla_paginas[1][i] == itoc(id_carpincho)){
				tabla_paginas[1][i] = 'V';
			}
	}
	log_info(logger, "Carpincho eliminado completamente :C");
}

int carpincho_existente(char tabla_paginas[][cantidad_total_paginas], int id_carpincho){
	log_info(logger, "Buscando si el carpincho %d existe",id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
		if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] != 'V'){
			log_info(logger, "Carpincho encontrado");
			return i; //si lo encontro devuelve la posicion en la tabla de la primera pagina del carpincho
		}
	}
	log_info(logger, "Carpincho no encontrado");
	return -1; //si no encontro el carpincho en la tabla (primera vez en la SWAP) devuelve -1
}

int carpincho_ya_existia(char tabla_paginas[][cantidad_total_paginas], int id_carpincho){
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
	int marcos_libres = 0;
	log_info(logger, "Buscando si el carpincho %d tiene marcos disponibles", id_carpincho);
	for(int i = 0; i < cantidad_total_paginas; i++){
			if(tabla_paginas[1][i] == itoc(id_carpincho) && tabla_paginas[2][i] == 'V'){
				marcos_libres++;
			}
		}
	if(marcos_libres > 0){
		log_info(logger, "Aun tiene %d lugares",marcos_libres);
		return marcos_libres;
	}
	else
		log_info(logger, "Ya uso todos los marcos");
		return 0;
}

