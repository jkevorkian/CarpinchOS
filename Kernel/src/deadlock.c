#include "deadlock.h"

int iniciar_deteccion_deadlock(void* semaforosMaybe, int tiempo_deadlock){

    t_deadlock* deadlock;
    deadlock->milisegundos_entre_detecciones = tiempo_deadlock;
    deadlock->semaforosMaybe = semaforosMaybe;
    
    int retorno = pthread_create(&detector, NULL, detectar_deadlock, deadlock);
    if(retorno != 1){
        if(retorno == EAGAIN){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: Insufficient resources to create another thread");
            return retorno;
        }
        if(retorno == EINVAL){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: Invalid settings in attr");
            return retorno;
        }
        if(retorno == EPERM){
            log_error(logger, "Error al crear hilo de deteccion de deadlock: No permission to set the scheduling policy and parameters specified in attr.");
            return retorno;
        }
    }else return 0;
}

int finalizar_deteccion_deadlock(){ //TODO: implementar esta funcion donde corresponda
    return pthread_cancel(detector);
}

detectar_deadlock(t_deadlock* deadlock){

    float segundos_entre_detecciones = (deadlock->milisegundos_entre_detecciones)/100;
    while(1){
        while (algoritmo_deteccion(deadlock)){
            matar_proximo_carpincho(deadlock);
        }

        sleep(segundos_entre_detecciones);
    }
}

int algoritmo_deteccion(t_deadlock* deadlock){
    int deadlock_detectado;
    //TODO
    return deadlock_detectado;
}

int matar_proximo_carpincho(t_deadlock* deadlock){
    //TODO
}
