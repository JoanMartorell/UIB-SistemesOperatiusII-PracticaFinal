/*
 * mi_escribir.c -> permite escribir texto en una posición de un fichero
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"

int main(int argc, char const *argv[]) {
    
    //Comprobamos sintaxis
    if(argc != 5) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    //Comprobamos que sea un fichero
    if((argv[2][strlen(argv[2]) - 1]) == '/') {
        fprintf(stderr, ROJO "Error: No es un fichero.\n" RESET);
        return FALLO;
    }

    int bytesEscritos;

    //Montamos el dispositivo
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    fprintf(stderr, "longitud texto: %ld\n", strlen(argv[3]));

    bytesEscritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if(bytesEscritos < 0) {
        mostrar_error_buscar_entrada(bytesEscritos);
        bytesEscritos = 0;
    }

    fprintf(stderr, "Bytes escritos: %d\n\n", bytesEscritos);

    bumount();

    return EXITO;
}