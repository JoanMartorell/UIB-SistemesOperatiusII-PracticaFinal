/*
 * escribir.c -> programa externo ficticio, sólo para probar funciones
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

//#include <stdlib.h>
#include "ficheros.h"


int main(int argc, char *argv[]) {

    if(argc != 4) {
        fprintf(stderr, "Error Sintaxis: ./escribir <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n");
        fprintf(stderr, "Offsets: 9000, 209000, 30725000, 409605000, 480000000\n");
        fprintf(stderr, "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n");
        return FALLO;
    }

    //Montamos el dispositivo
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }

    int diferentes_inodos = atoi(argv[3]);
    int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};
    int longitud = strlen(argv[2]);
    unsigned char buffer[BLOCKSIZE];
    int bytesEscritos;

    //Reservamos inodo
    int ninodo = reservar_inodo('f', 6);
    if(ninodo == -1) {
        return FALLO;
    }

    printf("longitud texto: %ld\n\n", strlen(argv[2]));

    for(int i = 0; i < (sizeof(offsets) / sizeof(int)); i++) {

        printf("Nº inodo reservado: %i\n", ninodo);
        printf("offset: %i\n", offsets[i]);
        
        memset(buffer, 0, BLOCKSIZE); 
        bytesEscritos = mi_write_f(ninodo, argv[2], offsets[i], longitud);
        
        printf("Bytes escritos: %i\n", bytesEscritos);

        struct STAT stat;
        if(mi_stat_f(ninodo, &stat) == -1) {
            return FALLO;
        }
        
        printf("stat.tamEnBytesLog=%i\n", stat.tamEnBytesLog);
        printf("stat.numBloquesOcupados=%i\n\n", stat.numBloquesOcupados);

        if(diferentes_inodos != 0 && i != 4) {
            ninodo = reservar_inodo('f',6);
            if(ninodo == -1) {
                return FALLO;
            }
        }
    }

    //Desmontamos el dispositivo
    if(bumount() == -1) {
        return FALLO;
    }

    return EXITO;
}

