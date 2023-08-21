/*
 * mi_mkdir.c -> crea un fichero o directorio
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"


int main(int argc, char const *argv[]) {

    //Comprobamos sintaxis
    if(argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_mkdir <disco> <permisos> </ruta_directorio/>\n" RESET);
        return FALLO;
    }

    //Comprobamos permisos
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7) {
        fprintf(stderr, ROJO "Error: Modo inválido: <<%d>>\n" RESET, atoi(argv[2]));
        return FALLO;
    }

    unsigned char permisos = atoi(argv[2]);

    if((argv[3][strlen(argv[3]) - 1] == '/')) {  //si no es un fichero

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

    } else {  //si es un directorio
        fprintf(stderr, ROJO "Error: No es un directorio.\n" RESET);
    }

    return EXITO;
}
