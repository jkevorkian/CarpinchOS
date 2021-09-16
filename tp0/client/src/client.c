#include "client.h"

int main(void) {
	/*---------------------------------------------------PARTE 2-------------------------------------------------------------*/

	int conexion;
	char* ip;
	char* puerto;
	char* valor;

	t_log* logger;
	t_config* config;

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger();

	log_info(logger, "Soy un Log");


	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

	config = iniciar_config();

	valor = config_get_string_value(config, "CLAVE");
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");

	conexion = crear_conexion(ip, puerto);

	enviar_mensaje(valor, conexion);

	paquete(conexion);

//	leer_consola(logger);

	terminar_programa(conexion, logger, config);

	/*---------------------------------------------------PARTE 5-------------------------------------------------------------*/
	// Proximamente
	return EXIT_SUCCESS;
}

t_log* iniciar_logger(void) {
	t_log* nuevo_logger;

	nuevo_logger = log_create("tp0.log", "client", 1, LOG_LEVEL_DEBUG);
	if (nuevo_logger == NULL)
		printf("Falla en la creación del Logger");

	return nuevo_logger;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config;

	nuevo_config = config_create("tp0.config");

	return nuevo_config;
}


void leer_consola(t_log* logger) {
	char* leido;

	while (leido = readline(">")) {
		if (*leido == '\0')
			break;
		log_info(logger, leido);
		free(leido);
	}

}

void paquete(int conexion) {
	char* leido;
	t_paquete* paquete;
	paquete = crear_paquete();

	while (leido = readline(">")) {
		if (*leido == '\0')
			break;
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
	}
	
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);	
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
	if (logger != NULL)
		log_destroy(logger);
	if (config != NULL)
		config_destroy(config);

	liberar_conexion(conexion);
}

