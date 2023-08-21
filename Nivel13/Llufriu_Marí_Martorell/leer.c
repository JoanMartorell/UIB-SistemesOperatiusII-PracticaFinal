/*
 * leer.c -> programa externo ficticio, sólo para probar funciones
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros.h"

#define tambuffer 1500

int main(int argc, char const *argv[]) {

    //Variables
    int ninodo;
    struct inodo inodo;
    int offset = 0;
    int bytesleidos = 0;
    char buffer[tambuffer];

    //Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./leer <nombre_dispositivo> <ninodo>\n" RESET);
        return FALLO;
    }

    memset(buffer, 0, tambuffer);
    ninodo = atoi(argv[2]);

    //Montamos dispositivo
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }

    int leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    while(leidos > 0) {

        bytesleidos = bytesleidos + leidos;
        write(1, buffer, leidos);

        memset(buffer, 0, tambuffer);
        offset = offset + tambuffer;
        
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }

    //Leemos inodo
    if(leer_inodo(ninodo, &inodo)) {
        return FALLO;
    }

    fprintf(stderr, "total_leidos: %d\n", bytesleidos);
    fprintf(stderr, "tamEnBytesLog: %d\n", inodo.tamEnBytesLog);

    //Desmontamos dispositivo
    if (bumount() == -1) {
        return FALLO;
    }

    return EXITO;
}