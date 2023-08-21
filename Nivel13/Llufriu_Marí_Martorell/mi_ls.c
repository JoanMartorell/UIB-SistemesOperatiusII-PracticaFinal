/*
 * mi_ls.c -> lista el contenido de un directorio
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"


int main(int argc, char const *argv[]) {

    //Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n" RESET);
        return FALLO;
    }

    //Montamos el dispositivo
    if(bmount(argv[1]) == FALLO) {
        return FALLO;
    }

    char tipo = 'd';
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    int total;

    if((argv[2][strlen(argv[2]) - 1] != '/')) { //si no es un fichero
        tipo = 'f';
    }

    if((total = mi_dir(argv[2], buffer, tipo)) < 0) {
        return FALLO;
    }
    
    if(total > 0) {

        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("%s\n", buffer);
    }

    bumount();

    return EXITO;
}
