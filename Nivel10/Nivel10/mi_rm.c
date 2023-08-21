/*
 * mi_rm.c -> borra un fichero o directorio
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"


int main(int argc, char **argv) {

    //Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_rm disco /ruta\n" RESET);
        return FALLO;
    }
    
    //Montamos disco
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    if(mi_unlink(argv[2]) < 0) {
        return FALLO;
    }

    bumount();
    
    return EXITO;
}