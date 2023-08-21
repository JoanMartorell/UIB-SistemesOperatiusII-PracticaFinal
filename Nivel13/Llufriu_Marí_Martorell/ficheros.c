/*
 * ficheros.c -> funciones de tratamiento de ficheros
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros.h"

/*
 * mi_write_f
 ---------------------------------------------------------
 * escribe el contenido de un buffer en un fichero/directorio
 * ninodo: nº de inodo (fichero/directorio)
 * buf_original: buffer de memoria a leer
 * offset: posición de escritura inicial
 * nbytes: nº de byes a escribir
 * returns: -1 caso de error, o cantidad de bytes escritos
*/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {   

    mi_waitSem();
    
    //Declaraciones
    int primerBL, ultimoBL, desp1, desp2, nbfisico, bytesEscritos = 0;
    unsigned char buf_bloque[BLOCKSIZE];
    
    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) < FALLO) {
        mi_signalSem();
        return FALLO;
    }

    //Comprobamos que tenga los permisos para escribir
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr, ROJO "No hay permisos de escritura\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;


    if((nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1)) < FALLO) {
        mi_signalSem();
        return FALLO;
    }

    if(bread(nbfisico, buf_bloque) < FALLO) {
        mi_signalSem();
        return FALLO;
    }

    //Caso en el que el buffer cabe en un bloque fisico
    if(primerBL == ultimoBL) {

        memcpy(buf_bloque+desp1, buf_original, nbytes);

        if(bwrite(nbfisico, buf_bloque) < FALLO) {
            mi_signalSem();
            return FALLO;
        }

        bytesEscritos += nbytes;

    } else { //Caso en el que la escritura ocupa mas de un bloque

        //1. Primer bloque lógico
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        if(bwrite(nbfisico, buf_bloque) < FALLO) {
            mi_signalSem();
            return FALLO;
        }

        bytesEscritos += BLOCKSIZE - desp1;
        
        //2. Bloques lógicos intermedios
        for(int i= (primerBL + 1); i < ultimoBL; i++) {

            if((nbfisico = traducir_bloque_inodo(&inodo, i, 1)) < FALLO) {
                mi_signalSem();
                return FALLO;
            }
            
            memcpy(buf_bloque, (buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE), BLOCKSIZE);

            if(bwrite(nbfisico, buf_bloque) < FALLO) {
                mi_signalSem();
                return FALLO;
            }

            bytesEscritos += BLOCKSIZE;
        }


        //3. Último bloque lógico
        if((nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1)) < FALLO) {
            mi_signalSem();
            return FALLO;
        }

        if(bread(nbfisico, buf_bloque) < FALLO) {
            mi_signalSem();
            return FALLO;
        }

        memcpy(buf_bloque, buf_original +(nbytes - (desp2 + 1)), desp2 + 1);

        if(bwrite(nbfisico, buf_bloque) < FALLO) {
            mi_signalSem();
            return FALLO;
        }

        bytesEscritos += (desp2 + 1);
    }
    
    //Comprobar si hemos escrito más allá del final del fichero
    if(inodo.tamEnBytesLog < (offset + bytesEscritos)) {
        inodo.tamEnBytesLog = (offset + bytesEscritos);
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);
    if(escribir_inodo(ninodo, &inodo) < FALLO) {
        mi_signalSem();
        return FALLO;
    }


    if(bytesEscritos != nbytes) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return bytesEscritos;
}

/*
 * mi_read_f
 ---------------------------------------------------------
 * lee el contenido de un fichero/directorio en un buffer
 * ninodo: nº de inodo (fichero/directorio)
 * buf_original: buffer de memoria a escribir
 * offset: posición de lectura inicial
 * nbytes: nº de byes a leer
 * returns: -1 caso de error, o cantidad de bytes leídos
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {

    mi_waitSem();

    //Declaraciones
    int primerBL, ultimoBL, desp1, desp2, nbfisico;
    int bytesLeidos = 0;

    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    
    //Leemos inodo
    if(leer_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }

    //Compobamos permisos
    if((inodo.permisos & 4) != 4) {
        fprintf(stderr, ROJO "No hay permisos de lectura\n" RESET);
        mi_signalSem();
        return FALLO;
    }
    
    if(offset >= inodo.tamEnBytesLog) {
        mi_signalSem();
        return bytesLeidos;
    }
    
    if(offset + nbytes >= inodo.tamEnBytesLog) {  //pretende leer más allá de EOF
        nbytes = inodo.tamEnBytesLog - offset;
        //leemos sólo los bytes que podemos desde el offset hasta EOF
    }

    primerBL = offset / BLOCKSIZE;
    ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;

    desp1 = offset % BLOCKSIZE;
    desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    //Obtenemos el numero de bloque fisico
    nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
    if(nbfisico != -1) {
        if(bread(nbfisico, buf_bloque) == -1) {
            mi_signalSem();
            return FALLO;
        }
    }        
    
    if(primerBL == ultimoBL) {
        if(nbfisico != -1) {
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }

        bytesLeidos = nbytes;

    } else {
        
        if(nbfisico != -1) {
            memcpy (buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }

        bytesLeidos = BLOCKSIZE - desp1;
        
        for(int i = primerBL + 1; i < ultimoBL; i++) {

            nbfisico = traducir_bloque_inodo(&inodo, i, 0);
            if(nbfisico != -1) {
                if (bread(nbfisico, buf_bloque) == -1) { // error
                    mi_signalSem();
                    return FALLO;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
            }

            bytesLeidos += BLOCKSIZE;
        }

        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if(nbfisico != -1) {
            if(bread(nbfisico, buf_bloque) == -1) {
                mi_signalSem();
                return FALLO;
            }
            memcpy(buf_original + (nbytes - (desp2 + 1)), buf_bloque, desp2 + 1);
        }        

        bytesLeidos += desp2 + 1;
    }

    inodo.atime = time(NULL);

    if(escribir_inodo(ninodo, &inodo) == -1) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return bytesLeidos;

}

/*
 * mi_stat_f
 ---------------------------------------------------------
 * devuelve la metainformación de un fichero/directorio
 * ninodo: nº de inodo
 * p_stat: puntero a struct STAT
 * returns: -1 caso de error, o EXITO
*/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {

    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo)) {
        return FALLO;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;

    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;

    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

/*
 * mi_chmod_f
 ---------------------------------------------------------
 * cambia los permisos de un fichero/directorio
 * ninodo: nº de inodo
 * permisos: nuevos permisos
 * returns: -1 caso de error, o EXITO
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {

    mi_waitSem();
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo)) {
        mi_signalSem();
        return FALLO;
    }

    //Cambiamos los permisos
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);

    if(escribir_inodo(ninodo, &inodo)) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return EXITO;
}

/*
 * mi_truncar_f
 ---------------------------------------------------------
 * trunca un fichero/directorio
 * ninodo: nº de inodo
 * nbytes: nº de bytes
 * returns: -1 caso de error, o EXITO
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {

    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Comprobamos que el inodo tenga permisos de escritura
    if((inodo.permisos & 2) != 2) {
        fprintf(stderr, ROJO "No tiene permiso de escritura.\n" RESET);
        return EXIT_FAILURE;
    }

    //Comprobamos que no se trunquen mas allá del tamaño de bytes lógicos
    if(nbytes > inodo.tamEnBytesLog) {
        return FALLO;
    }

    //Obtenemos el bloque lógico
    int primerBL;
    if(nbytes % BLOCKSIZE == 0) {
        primerBL = nbytes / BLOCKSIZE;

    } else {
        primerBL = (nbytes / BLOCKSIZE) + 1;
    }

    //Liberamos bloques
    int liberados = liberar_bloques_inodo(primerBL, &inodo);

    //Actulizamos información
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= liberados;

    //Guardamos inodo
    if(escribir_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    return liberados;
}

