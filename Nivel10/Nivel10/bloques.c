/*
 * Bloques.c -> funciones básicas para montar y desmontar el dispositivo virtual
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "bloques.h"

//Descriptor del fichero
static int descriptor = 0;

/*
 * bmount
 ---------------------------------------------------------
 * monta el dispositivo virtual
 * camino: puntero a la cadena
 * returns: -1 caso de error, o descriptor si va bien
*/
int bmount(const char *camino) {

    //Abre el fichero
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    //Caso de error
    if(descriptor == -1){
        fprintf(stderr, ROJO "Error al abrir el fichero.\n" RESET);
        return FALLO;
    }

    //Retorna descriptor
    return descriptor;
}

/*
 * bumount
 ---------------------------------------------------------
 * desmonta el dispositivo virtual
 * returns: -1 caso de error, o EXITO si va bien
*/
int bumount() {

    //Liberamos el descriptor
    int c = close(descriptor);

    //Caso de error
    if(c == -1) {
        fprintf(stderr, ROJO "Error al cerrar el fichero.\n" RESET);
        return FALLO;
    }

    //Se ha cerrado correctamente
    return EXITO;

}

/*
 * bwrite
 ---------------------------------------------------------
 * escribe 1 bloque en el dispositivo virtual
 * nbloque: nº de bloque
 * buf: puntero al contenido del buffer
 * returns: -1 caso de error, o nº de bytes escritos
*/
int bwrite(unsigned int nbloque, const void *buf) {

    //Calculamos desplazamiento
    off_t desp = nbloque * BLOCKSIZE;

    //Movemos puntero
    if(lseek(descriptor, desp, SEEK_SET) == -1) {
        fprintf(stderr, ROJO "Error al posicionarse en el fichero.\n" RESET);
        return FALLO;
    }

    //Volcamos contenido
    int w = write(descriptor, buf, BLOCKSIZE);

    //Caso de error
    if(w < 0) {
        fprintf(stderr, ROJO "Error al escribir el bloque.\n" RESET);
        return FALLO;
    }

    //Retorna numero de bytes escritos
    return w;
}

/*
 * bread
 ---------------------------------------------------------
 * lee 1 bloque del dispositivo virtual
 * nbloque: nº de bloque
 * buf: puntero al contenido del buffer
 * returns: -1 caso de error, o nº de bytes leídos
*/
int bread(unsigned int nbloque, void *buf) {

    //Calculamos desplazamiento
    off_t desp = nbloque * BLOCKSIZE;

    //Movemos puntero
    if(lseek(descriptor, desp, SEEK_SET) == -1) {
        fprintf(stderr, ROJO "Error al posicionarse en el fichero.\n" RESET);
        return FALLO;
    }

    //Leemos contenido
    int r = read(descriptor, buf, BLOCKSIZE);

    //Caso de error
    if(r < 0) {
        fprintf(stderr, ROJO "Error al leer el bloque.\n" RESET);
        return FALLO;
    }
    
    //Retorna numero bytes leidos
    return r;
}