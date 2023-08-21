/*
 * mi_chmod.c -> cambia los permisos de un fichero o directorio
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"

int main(int argc, char const *argv[]) {

    unsigned char permisos;
    int r;

    //Comporbamos sintaxis
    if(argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    //Compobamos permisos
    permisos = atoi(argv[2]);
    if(permisos > 7) {
        fprintf(stderr, ROJO "Error de sintaxis: permisos incorrectos.\n" RESET);
        return FALLO;
    }

    //Montamos dispositivo
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    r = mi_chmod(argv[3], permisos);
    if(r < 0) {
        mostrar_error_buscar_entrada(r);
        return FALLO;
    }

    bumount();

    return EXITO;
}