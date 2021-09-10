/*
 *  utils-sockets.h
 *
 *  Created on: 21 jun. 2021
 *  Author: cualquier-cosa
 */

#ifndef _UTILS_SOCKETS_H_
#define _UTILS_SOCKETS_H_

#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include<stdio.h>   // sprintf
#include <arpa/inet.h>
#include<commons/log.h>
#include<commons/collections/list.h>

/**
* @NAME: puerto_desde_socket
* @DESC: Toma un socket y devuelve el puerto en el que está activo
* @OUTP: Devuelve un puerto
*/
int puerto_desde_socket(int socket);

/**
* @NAME: validar_socket
* @DESC: Recibe la salida de un crear_conexion_* e informa si hubo errores
* @OUTP: Devuelve true si el socket es válido
*/
bool validar_socket(int socket, t_log* logger);

/**
* @NAME: data_socket
* @DESC: Muestra en pantalla la información del socket recibido por parámetro
*/
void data_socket(int socket, t_log* logger);

#endif /* _UTILS_SOCKETS_H_ */