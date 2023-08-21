/*
 * permitir.c -> programa externo ficticio, sólo para probar funciones
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros.h"

int main(int argc, char *argv[]) {

    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    if(argc != 4) {
        fprintf(stderr, "Sintaxis errónea: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n");
        return FALLO;
    }

    //Montamos el dispositivo
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }

    //Cambiamos permisos
    if(mi_chmod_f(ninodo, permisos)) {
        return FALLO;
    }

    // Desmonta el dispositivo
    if(bumount() == -1) {
        return FALLO;
    }

    return EXITO;
}

