/*
 * mi_mkfs.c -> utiliza las funciones de bolques.c
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros_basico.h"

int main(int argc, char **argv) {

    //Comprobamos los argumentos
    if(argc != 3) {
        fprintf(stderr, "Sintaxis errónea: ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return FALLO;
    }

    //Obtenemos los parámetros de argv
    char *camino = argv[1];
    int nbloques = atoi(argv[2]);
    int ninodos = nbloques / 4;

    //Montamos dispositivo virtual
    if(bmount(camino) == -1) {
        return FALLO;
    }
    
    //Buffer tamano del bloque 
    unsigned char buf[BLOCKSIZE];

    //Llenamos buffer a 0s
    memset(&buf, 0, BLOCKSIZE);

    //Escribimos el buffer en todos los bloques del fichero
    for(int i = 0; i < nbloques; i++) {
        if(bwrite(i, buf) == -1) {
            return FALLO;
        }
    }

    //Inicializamos metadatos
    if(initSB(nbloques, ninodos) == -1) {
        return FALLO;
    }
    if(initMB() == -1) {
        return FALLO;
    }
    if(initAI() == -1) {
        return FALLO;
    }

    //Creamos directorio raíz
    reservar_inodo ('d', 7);

    //Desmontamos dispositivo virtual
    if(bumount() == -1) {
        return FALLO;
    }
    
}