/*
 * ficheros_basico.c -> funciones de tratamiento básico del sistema de ficheros
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros_basico.h"


/*
 * tamMB
 ---------------------------------------------------------
 * calcula el tamaño en bloques necesario para el mapa de bits
 * nbloques: nº de bloques
 * returns: tamaño del mapa de bits en bloques
*/
int tamMB(unsigned int nbloques) {

    int tamMB = (nbloques / 8) / BLOCKSIZE;

    if((nbloques / 8) % BLOCKSIZE != 0) {
        tamMB += 1;
    }

    return tamMB;
}

/*
 * tamAI
 ---------------------------------------------------------
 * calcula el tamaño en bloques del array de inodos
 * ninodos: nº de inodos
 * returns: tamaño del array de inodos en bloques
*/
int tamAI(unsigned int ninodos) {

    int tamAI = (ninodos * INODOSIZE) / BLOCKSIZE;

    if((ninodos * INODOSIZE) % BLOCKSIZE != 0) {
        tamAI += 1;
    }

    return tamAI;
}

/*
 * initSB
 ---------------------------------------------------------
 * inicializa los datos del superbloque
 * nbloques: nº de bloques
 * ninodos: nº de inodos
 * returns: -1 caso de error, o EXITO si va bien
*/
int initSB(unsigned int nbloques, unsigned int ninodos) {

    // Creacion del superbloque
    struct superbloque SB;

    //Información
    SB.posPrimerBloqueMB = posSB + tamSB; //posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;

    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    //Escribimos la estructura en el bloque posSB
    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }

    return EXITO;
}

/*
 * initMB
 ---------------------------------------------------------
 * inicializa el mapa de bits
 * returns: -1 caso de error, o EXITO si va bien
*/
int initMB() {

    //Buffer
    unsigned char buff[BLOCKSIZE];
    memset(buff, 0, BLOCKSIZE);  //Todo a 0

    // Leemos el superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    // Tamaño MB
    int tamMB = SB.posUltimoBloqueMB - SB.posPrimerBloqueMB;

    //Inicializamos el mapa de bits
    for(int i = SB.posPrimerBloqueMB; i <= tamMB + SB.posPrimerBloqueMB; i++) {
        if(bwrite(i, buff) == -1) {
            return FALLO;
        }
    }

    // Ponemos a 1 los bits correspondientes
    for(unsigned int i = posSB; i < SB.posPrimerBloqueDatos; i++) {

        reservar_bloque();
    }

    return EXITO;
}

/*
 * initAI
 ---------------------------------------------------------
 * inicializa la lista de inodos libres
 * returns: -1 caso de error, o EXITO si va bien
*/
int initAI() {

    //Buffer
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);   //Todo a 0

    //Inicializamos la estructura de inodos.
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Leemos el superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    int acabar = 0;
    int contInodos = SB.posPrimerInodoLibre + 1;

    for(int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && acabar == 0; i++) {  //para cada bloque del AI

        for(int j = 0; j < (BLOCKSIZE / INODOSIZE); j++) { //para cada inodo del AI

            inodos[j].tipo = 'l'; //libre

            if (contInodos < SB.totInodos) {  //si no hemos llegado al último inodo

                inodos[j].punterosDirectos[0] = contInodos;   //enlazamos con el siguiente
                contInodos++;

            } else { //hemos llegado al último inodo
            
                inodos[j].punterosDirectos[0] = UINT_MAX;

                //Hay que salir
                acabar = 1;

                break;
            }
        }

        //Escribimos el bloque de inodos en el dispositivo virtual
        if(bwrite(i, &inodos) == -1) {
            return FALLO;
        }
    }

    return EXITO;;
}

/*
 * escribir_bit
 ---------------------------------------------------------
 * escribe el valor de bit en un bit del MB
 * nbloque: nº de bloque del MB
 * bit: valor del bit (1 o 0)
 * returns: -1 caso de error, o EXITO si va bien
*/
int escribir_bit(unsigned int nbloque, unsigned int bit) {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    int posbyte = nbloque / 8;  //Byte que contiene el bit
    int posbit = nbloque % 8;   //Bit dentro de ese byte

    //Bloque donde se encuentra ese byte
    int nbloqueMB = posbyte / BLOCKSIZE;

    //Posicion donde se encuentra ese bloque
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs, bufferMB) == -1) {
        return FALLO;
    }

    //Localizamos posicion del byte en el array
    posbyte = posbyte % BLOCKSIZE;
    
    //Mascara
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit; // desplazamiento de bits a la derecha

    //Poner bit a 1:
    if(bit == 1) {
        bufferMB[posbyte] |= mascara; //  operador OR para bits

    } else if(bit == 0) { //Poner bit a 0:
        bufferMB[posbyte] &= ~mascara; // operadores AND y NOT para bits
    
    }

    //Escribimos el contenido del buffer en el bloque
    if(bwrite(nbloqueabs, bufferMB) == -1) {
        return FALLO;
    }

    return EXITO;
}

/*
 * leer_bit
 ---------------------------------------------------------
 * lee un determinado bit del MB
 * nbloque: nº de bloque del MB
 * returns: -1 caso de error, o el valor del bit leído
*/
char leer_bit(unsigned int nbloque) {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    int posbyte = nbloque / 8;  //Byte que contiene el bit
    int posbit = nbloque % 8;   //Bit dentro de ese byte

    //Bloque donde se encuentra ese byte
    int nbloqueMB = posbyte / BLOCKSIZE;

    //Posicion donde se encuentra ese bloque
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    unsigned char bufferMB[BLOCKSIZE];
    if(bread(nbloqueabs, bufferMB) == -1) {
        return FALLO;
    }

    //localizar posicion del byte en el array
    posbyte = posbyte % BLOCKSIZE;
    
    //Mascara
    unsigned char mascara = 128;  // 10000000
    mascara >>= posbit;           // desplazamiento de bits a la derecha
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha

    #if DEBUG3
        printf(GRIS "[leer_bit(%i) → posbyte:%i, posbit:%i, nbloqueMB:%i, nbloqueabs:%i)]\n" RESET, nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
    #endif

    return mascara;
}

/*
 * reservar_bloque
 ---------------------------------------------------------
 * encuentra el primer bloque libre y lo ocupa
 * returns: -1 caso de error, o nº de bloque que hemos reservado
*/
int reservar_bloque() {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    if (SB.cantBloquesLibres == 0) {
        return FALLO;
    }

    int nbloquesABS = SB.posPrimerBloqueMB;
    
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAUX[BLOCKSIZE];

    //Rellenamos de 1s
    memset(bufferAUX, 255, BLOCKSIZE);

    //Leemos el primer bloque del MB
    if(bread(nbloquesABS, bufferMB) == -1) {
        return FALLO;
    }

    //Mientras los buffers sean iguales lee un bloque
    if(bread(nbloquesABS++, bufferMB) == -1) {
        return FALLO;
    }

    int i = memcmp(bufferMB, bufferAUX, BLOCKSIZE);
    while(i == 0) {

        if(bread(nbloquesABS, bufferMB) == -1) {
            return FALLO;
        }

        i = memcmp(bufferMB, bufferAUX, BLOCKSIZE);
        nbloquesABS++;
    }

    nbloquesABS--;
    int posByte = 0;

    //Para encontrar el byte 0
    while(bufferMB[posByte] == 255) {
        posByte++;
    }

    //Para encontrar el bit 0
    unsigned char mascara = 128;
    int posBit = 0;
    while(bufferMB[posByte] & mascara) {
        bufferMB[posByte] <<= 1;
        posBit++;
    }


    //Bloque
    int nbloque = ((nbloquesABS - SB.posPrimerBloqueMB) * BLOCKSIZE + posByte) * 8 + posBit;
    if(escribir_bit(nbloque, 1) == -1) {
        return FALLO;
    }
    SB.cantBloquesLibres--;

    //Ponemos a 0 el bloque que reservamos
    unsigned char buffer0s[BLOCKSIZE];
    memset(buffer0s, 0, BLOCKSIZE);
    if(bwrite(SB.posPrimerBloqueDatos + nbloque - 1, buffer0s) == -1) {
        return FALLO;
    }

    //Guardamos el superbloque
    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }

    // Devolvemos el bloque que hemos reservado
    return nbloque;

}

/*
 * liberar_bloque
 ---------------------------------------------------------
 * libera un bloque determinado
 * nbloque: nº bloque a liberar
 * returns: -1 caso de error, o nº de bloque liberado
*/
int liberar_bloque(unsigned int nbloque) {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }
    
    //Ponemos a 0 el bit del MB
    if(escribir_bit(nbloque, 0) == -1) {
        return FALLO;
    }

    //Incrementamos la cantidad de bloques libres
    SB.cantBloquesLibres++;

    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }

    //Devolver numero de bloque
    return nbloque;
}

/*
 * escribir_inodo
 ---------------------------------------------------------
 * escribe un inodo en un array de inodos
 * returns: -1 caso de error, o EXITO
*/
int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    
    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    //Operación para obtener el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));

    //Array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Leemos el bloque del array de inidos correspondiente
    if(bread(posBloqueInodo, inodos) == -1) {
        return FALLO;
    }

    //Escribimos el inodo en el lugar correspondiente del array
    inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = *inodo;

    //El bloque modificado lo escribimos en el dispositivo virtual
    if(bwrite(posBloqueInodo, inodos) == -1) {
        return FALLO;
    }

    return EXITO;
}

/*
 * leer_inodo
 ---------------------------------------------------------
 * lee un determinado inodo del array de inodos
 * ninodo: nº de inodo
 * inodo: puntero a variable tipo inodo
 * returns: -1 caso de error, o EXITO
*/
int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    
    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    //Operación para obtener el bloque donde se encuentra un inodo dentro del AI
    unsigned int posBloqueInodo = SB.posPrimerBloqueAI + (ninodo / (BLOCKSIZE / INODOSIZE));
    
    //Array de inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    //Leemos el bloque del array de inidos correspondiente
    if(bread(posBloqueInodo, inodos) == -1) {
        return FALLO;
    }

    //Escribimos el inodo en el lugar correspondiente del array
    *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];

    return EXITO;
}

/*
 * reservar_inodo
 ---------------------------------------------------------
 * encuentra el primer inodo libre
 * tipo: tipo de inodo
 * permisos: permisos del inodo
 * returns: -1 caso de error, o posición inodo reservado
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos) {

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    //Caso hay inodos libres
    if(SB.cantBloquesLibres == 0) {
        return FALLO;
    }

    //Actualizamos la lista enlazada de inodos libres
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;

    SB.posPrimerInodoLibre++;
    SB.cantInodosLibres--;

    struct inodo inodoAUX;

    //Inicialización
    inodoAUX.tipo = tipo;
    inodoAUX.permisos = permisos;
    inodoAUX.nlinks = 1;
    inodoAUX.tamEnBytesLog = 0;
    inodoAUX.atime = time(NULL);
    inodoAUX.mtime = time(NULL);
    inodoAUX.ctime = time(NULL);
    inodoAUX.numBloquesOcupados = 0;

    for(int i = 0; i < 12; i++) {
        for(int j = 0; j < 3; j++) {
            inodoAUX.punterosIndirectos[j] = 0;
        }
        inodoAUX.punterosDirectos[i] = 0;
    }

    //Escribimos el inodo
    if(escribir_inodo(posInodoReservado, &inodoAUX) == -1) {
        return FALLO;
    }

    //Guardamos el superbloque
    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }

    return posInodoReservado;
}

/*
 * obtener_nRangoBL
 ---------------------------------------------------------
 * obtiene el rango de punteros
 * inodo: puntero a inodo
 * nblogico: nº de bloque lógico
 * ptr: puntero
 * returns: -1 caso de error, o dirección almacenada en el inodo
*/
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    
    if(nblogico < DIRECTOS) {  // <12
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;

    } else if(nblogico < INDIRECTOS0) {  // <268
        *ptr = inodo->punterosIndirectos[0];
        return 1;

    } else if(nblogico < INDIRECTOS1) {  // <56.804
        *ptr = inodo->punterosIndirectos[1];
        return 2;

    } else if(nblogico < INDIRECTOS2) {  // <16.843.020
        *ptr = inodo->punterosIndirectos[2];
        return 3;

    } else {
        *ptr = 0;
        fprintf(stderr, ROJO "Bloque lógico fuera de rango\n" RESET);
        return FALLO;
    }
}

/*
 * obtener_indice
 ---------------------------------------------------------
 * obtiene los índices de los bloques de punteros
 * nblogico: nº de bloque lógico
 * nivel_punteros: nivel de punteros
 * returns: -1 caso de error, o índice de los bloques de punteros
*/
int obtener_indice(unsigned int nblogico, int nivel_punteros) {

    if(nblogico < DIRECTOS) {
        return nblogico;
    
    } else if(nblogico < INDIRECTOS0) {
        return (nblogico - DIRECTOS);

    } else if(nblogico < INDIRECTOS1) {

        if(nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS0) / NPUNTEROS);

        } else if(nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS0) % NPUNTEROS);
        }

    } else if(nblogico < INDIRECTOS2) {

        if(nivel_punteros == 3) {
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));

        } else if(nivel_punteros == 2) {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);

        } else if(nivel_punteros == 1) {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }
    }

    return FALLO;
}

/*
 * traducir_bloque_inodo
 ---------------------------------------------------------
 * obtiene el nº de bloque físico correspondiente a un bloque lógico
 * inodo: puntero al inodo
 * nblogico: nº de bloque lógico
 * reservar: para consultar o reservar
 * returns: -1 caso de error, o nº de bloque físico correspondiente a un bloque lógico
*/
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar) {
    
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    ptr = 0, ptr_ant = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); //0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL; //el nivel_punteros +alto es el que cuelga directamente del inodo

    while(nivel_punteros > 0) { //iterar para cada nivel de punteros indirectos
        if(ptr == 0) { //no cuelgan bloques de punteros

            if(reservar == 0) { //bloque inexistente
                return FALLO;

            } else { //reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos de punteros
                
                ptr = reservar_bloque();
                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL); //fecha actual

                if(nivel_punteros == nRangoBL) { //el bloque cuelga directamente del inodo

                    inodo->punterosIndirectos[nRangoBL-1] = ptr;

                    #if DEBUG4
                        printf(GRIS "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n" RESET, nRangoBL-1, ptr, ptr, nivel_punteros);
                    #endif

                } else { //el bloque cuelga de otro bloque de punteros

                    buffer[indice] = ptr;
                    if(bwrite(ptr_ant, buffer) == -1) { //salvamos en el dispositivo el buffer de punteros modificado
                        return FALLO;
                    }

                    #if DEBUG4
                        printf(GRIS "[traducir_bloque_inodo()→ punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n" RESET, nivel_punteros+1, indice, ptr, ptr, nivel_punteros);
                    #endif
                    
                }

                memset(buffer, 0, BLOCKSIZE); //ponemos a 0 todos los punteros del buffer 
            }

        } else { //leemos del dispositivo el bloque de punteros ya existente
            if(bread(ptr, buffer) == -1) {
                return FALLO;
            }
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr; //guardamos el puntero actual
        ptr = buffer[indice]; //y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } //al salir de este bucle ya estamos al nivel de datos

    if(ptr == 0) { //no existe bloque de datos

        if(reservar == 0) { //error lectura ∄ bloque
            return FALLO;

        } else {
            ptr = reservar_bloque(); //de datos
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);

            if(nRangoBL == 0) { //si era un puntero Directo 
                inodo->punterosDirectos[nblogico] = ptr; //asignamos la direción del bl. de datos en el inodo

                #if DEBUG4
                    printf(GRIS "[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n" RESET, nblogico, ptr, ptr, nblogico);
                #endif

            } else {
                buffer[indice] = ptr; //asignamos la dirección del bloque de datos en el buffer

                #if DEBUG4
                    printf(GRIS "[traducir_bloque_inodo()→ punteros_nivel%i[%i] = %i (reservado BF %i para BL %i)]\n" RESET, nivel_punteros+1, indice, ptr, ptr, nblogico);
                #endif

                //salvamos en el dispositivo el buffer de punteros modificado
                if(bwrite(ptr_ant, buffer) == -1) {
                    return FALLO;
                }
            }
            
        }
    }

    //mi_write_f() se encargará de salvar los cambios del inodo en disco
    return ptr; //nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

/*
 * liberar_inodo
 ---------------------------------------------------------
 * tal inodo pasará a la cabeza de la lista de inodos libres
 * ninodo: nº de inodo
 * returns: -1 caso de error, o nº del inodo liberado
*/
int liberar_inodo(unsigned int ninodo) {

    //Leemos inodo
    struct inodo inodo;
    if(leer_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Liberamos todos los bloques del inodo
    int bloquesLiberados = liberar_bloques_inodo(0, &inodo);
    inodo.numBloquesOcupados -= bloquesLiberados;

    //Marcamos el inodo como tipo libre y tamEnBytesLog=0
    inodo.tipo = LIBRE;
    inodo.tamEnBytesLog = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) == -1) {
        return FALLO;
    }

    //Incluimos el inodo que queremos liberar en la lista de inodos libres
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;

    //Incrementamos la cantidad de inodos libres
    SB.cantInodosLibres++;

    //Escribimos el inodo actualizado
    if(escribir_inodo(ninodo, &inodo) == -1) {
        return FALLO;
    }

    //Guardamos el superbloque
    if(bwrite(posSB, &SB) == -1) {
        return FALLO;
    }


    //Devolvemos el nº del inodo liberado
    return ninodo;
}

/*
 * liberar_bloques_inodo
 ---------------------------------------------------------
 * libera todos los bloques ocupados
 * primerBL: primer bloque lógico
 * inodo: puntero al inodo
 * returns: -1 caso de error, o nº de bloques liberados
*/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo) {

    //Declaraciones
    unsigned int nivel_punteros, indice, ptr = 0, nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS];  //array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE]; //para llenar de 0s y comparar
    int ptr_nivel[3];  //punteros a bloques de punteros de cada nivel
    int indices[3];  //indices de cada nivel
    int liberados = 0;  // nº de bloques liberados

    int breads = 0, bwrites = 0;

    if(inodo->tamEnBytesLog == 0) { //el fichero está vacío
        return 0;
    }

    if((inodo->tamEnBytesLog % BLOCKSIZE) == 0) {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - 1;

    } else {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);

    #if DEBUG6
        printf(GRIS "[liberar_bloques_inodo()→ primerBL: %d, ultimoBL: %d]\n" RESET, primerBL, ultimoBL);
    #endif
    
    for(nBL = primerBL; nBL <= ultimoBL; nBL++) {  //recorrido BLs

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); //0:D, 1:I0, 2:I1, 3:I2
        if(nRangoBL < 0) {
            return FALLO;
        }
        nivel_punteros = nRangoBL; //el nivel_punteros +alto cuelga del inodo

        while(ptr > 0 && nivel_punteros > 0) {  //cuelgan bloques de punteros

            indice = obtener_indice(nBL, nivel_punteros);
            if(indice == 0 || nBL == primerBL) {
                //solo hay que leer del dispositivo si no está ya cargado previamente en un buffer    
                if(bread(ptr, bloques_punteros[nivel_punteros - 1]) < 0) {
                    return FALLO;
                }

                breads++;
            }             

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if(ptr > 0) { //si existe bloque de datos
            liberar_bloque(ptr);
            liberados++;

            if(nRangoBL == 0) { //es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;

                #if DEBUG6
                    printf(GRIS "[liberar_bloques_inodo()→ liberado BF %i de datos para BL %i]\n" RESET, ptr, nBL);
                #endif

            } else {
                nivel_punteros = 1;

                #if DEBUG6
                    printf(GRIS "[liberar_bloques_inodo()→ liberado BF %i de datos para BL: %i]\n" RESET, ptr, nBL);
                #endif

                while(nivel_punteros <= nRangoBL) {

                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel [nivel_punteros - 1]; 

                    if(memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) {  
                        //No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        if(liberar_bloque(ptr) == -1) {
                            return FALLO;
                        }
                        liberados++;

                        #if DEBUG6
                            printf(GRIS "[liberar_bloques_inodo()→ liberado BF %i de punteros_nivel%i correspondiente al BL: %i]\n" RESET, ptr, nivel_punteros, nBL);
                        #endif

                        if(nivel_punteros == nRangoBL) {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }

                        nivel_punteros++;
                        
                    } else {  //escribimos en el dispositivo el bloque de punteros modificado
                        if(bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == -1) {
                            return FALLO;
                        }
                        bwrites++;

                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }

    #if DEBUG6
        printf(GRIS "[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %i, total_bwrites: %i]\n" RESET, liberados, breads, bwrites);
    #endif

    return liberados;
}

