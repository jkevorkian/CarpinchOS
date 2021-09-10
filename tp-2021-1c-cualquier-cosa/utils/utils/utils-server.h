/*
 *  utils-server.h
 *
 *  Created on: 1 jun. 2021
 *  Author: utnso
 */

#ifndef _UTILS_SERVER_H_
#define _UTILS_SERVER_H_

#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>   // sprintf
#include <arpa/inet.h>

/**
* @NAME: crear_conexion_servidor
* @DESC: Crea un servicio activo en el puerto e ip definidas por parámetro
* @OUTP: Devuelve el socket del servicio
* @NOTA: Si el puerto es el 0, se generará un puerto aleatorio libre
*/
int crear_conexion_servidor(char *ip, int puerto, int cola_escucha);

/**
* @NAME: esperar_cliente
* @DESC: Toma el primer socket que esté en la cola de escucha y establece la conexión
* @OUTP: Devuelve el socket del ciente que se conectó
*/
int esperar_cliente(int socket);

#endif /* _UTILS_SERVER_H_ */