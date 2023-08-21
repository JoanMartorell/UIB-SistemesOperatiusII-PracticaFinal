/*
 * mi_cat.c -> muestra todo el contenido de un fichero
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"

int main(int argc, char **argv) {

    //Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }
    //Montamos dispositivo
    if(bmount(argv[1]) < 0 ) {
        return FALLO;
    }

    //Obtenemos parámetros
    int tambuffer = BLOCKSIZE * 4;
    int bytesLeidos = 0;
    int offset = 0;
    
    char buffer[tambuffer];
    memset(buffer, 0, sizeof(buffer));

    //Leemos
    int bytesLeidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    while(bytesLeidosAux > 0) {

        bytesLeidos += bytesLeidosAux;

        write(1, buffer, bytesLeidosAux); //imprime

        memset(buffer, 0, sizeof(buffer));
        offset += tambuffer;

        bytesLeidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    }

    fprintf(stderr, " \n");

    //Error
    if(bytesLeidos < 0) {
        mostrar_error_buscar_entrada(bytesLeidos);
        bytesLeidos = 0;
    } 

    fprintf(stderr, "\nTotal_leidos %d\n", bytesLeidos);

    bumount();

    return EXITO;
}