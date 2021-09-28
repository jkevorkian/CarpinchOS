#include "kernel.h"

int main(void)
{   
    t_config* config = config_create("../kernel.config");
    char* puertoMem = "mipana";
    char* linea = readline("--");
    printf("la data es: %s",linea);
    printf("la data es: %s",puertoMem);


    char* puertoMemoria = config_get_string_value(config,"PUERTO_MEMORIA");



    return 0;
}
