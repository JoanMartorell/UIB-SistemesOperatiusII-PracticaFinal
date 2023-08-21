/*
 * mi_touch.c -> crea un fichero
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"


int main(int argc, char const *argv[]) {

    //Comprobamos sintaxis
    if(argc != 4) {
        fprintf(stderr, ROJO "Error de sintaxis: ./mi_touch <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    //Comprobamos permisos
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7) {
        fprintf(stderr, ROJO "Error: Permiso denegado de lectura.\n" RESET);
    }

    unsigned char permisos = atoi(argv[2]);

    if((argv[3][strlen(argv[3]) - 1] != '/')) { //Si no es un fichero
    
        //Montamos el dispositivo
        if(bmount(argv[1]) == FALLO) {
            return FALLO;
        }

        int error;
        if((error = mi_creat(argv[3], permisos)) < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }

        bumount();

    } else {
        fprintf(stderr, ROJO "Error: No es un directorio.\n" RESET);
    }

    return EXIT_SUCCESS;
}

