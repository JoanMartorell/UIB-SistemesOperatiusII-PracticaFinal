/*
 * Directorios.c -> funciones de la última capa
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"

static struct UltimaEntrada UltimaEntrada;

/*
 * extraer_camino
 ---------------------------------------------------------
 * separa el contenido del camino en dos
 * camino: puntero a la cadena
 * inicial: puntero a la porcion de *camino entre los dos primeros '/' 
 * final: puntero al resto del camino
 * returns: -1 caso de error, o EXITO si va bien
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {

    //No empieza con '/', error
    if(camino[0] != '/') {
        return FALLO;
    }

    //Localizamos segundo '/'
    char *resto = strchr((camino + 1), '/');

    //Si se ha encotrado '/'
    if(resto) {

        strcpy(tipo, "d");
        strcpy(inicial, (camino + 1));
        strtok(inicial, "/");

        //Final = resto
        strcpy(final, resto);

    } else {

        //Inicial = camino
        strcpy(inicial, (camino + 1));
        strcpy(tipo, "f");

        strcpy(final, "");
    }

    return EXITO;
}

/*
 * buscar_entrada
 ---------------------------------------------------------
 * busca una determinada entrada
 * camino_pacial: puntero a la cadena
 * p_inodo_dir: puntero directorio padre
 * p_inodo: número de inodo
 * p_entrada: número de entrada
 * reservar: 0 -> consultar
 *           1 -> crear una entrada de directorio
 * permisos: pemisos
 * returns: número de error, o EXITO si va bien
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {

    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    memset(inicial, 0, sizeof(entrada.nombre));
    memset(final, 0, strlen(camino_parcial));
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    if(!strcmp(camino_parcial, "/")) {  //camino_parcial es “/”

        //Leemos superbloque
        struct superbloque SB;
        bread(posSB, &SB);

        *p_inodo = SB.posInodoRaiz;  //nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;

        return EXITO;
    }

    if(extraer_camino(camino_parcial, inicial, final, &tipo) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }

    #if DEBUG7
        fprintf(stderr, GRIS "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n" RESET, inicial, final, reservar);
    #endif

    //buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    memset(entrada.nombre, 0, sizeof(entrada.nombre));
    struct entrada buffLecEnt[BLOCKSIZE / sizeof(struct entrada)];
    memset(buffLecEnt, 0, BLOCKSIZE);

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada); //cantidad de entradas que contiene el inodo
    num_entrada_inodo = 0;  //nº de entrada inicial

    if(cant_entradas_inodo > 0) {

        if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
            return ERROR_PERMISO_LECTURA;
        }

        memset(buffLecEnt, 0, (BLOCKSIZE / sizeof(struct entrada)) * sizeof(struct entrada));

        while(num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, entrada.nombre) != 0) {

            num_entrada_inodo++;
            memset(entrada.nombre, 0, sizeof(entrada.nombre));

            if(mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                return ERROR_PERMISO_LECTURA;
            }
        }
    }

    if((strcmp(entrada.nombre, inicial) != 0) && (num_entrada_inodo == cant_entradas_inodo)) { //la entrada no existe

        switch (reservar) {

            case 0: //modo consulta. Como no existe retornamos error
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
                break;

            case 1: //modo escritura
                //Creamos la entrada en el directorio referenciado por *p_inodo_dir
                //si es fichero no permitir escritura
                if(inodo_dir.tipo == 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                //si es directorio comprobar que tiene permiso de escritura
                if((inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;

                } else {
                    strcpy(entrada.nombre, inicial);  //copiar *inicial en el nombre de la entrada
                    
                    if(tipo == 'd') {
                        if(strcmp(final, "/") == 0) {
                            entrada.ninodo = reservar_inodo('d', permisos);

                            #if DEBUG7
                                fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para '%s'] RESET\n", entrada.ninodo, tipo, permisos, entrada.nombre);
                            #endif

                        } else { //cuelgan más diretorios o ficheros
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }

                    } else { //es un fichero
                        entrada.ninodo = reservar_inodo('f', permisos);  //reservar un inodo como fichero y asignarlo a la entrada
    
                        #if DEBUG7
                            fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                        #endif
                    }

                    #if DEBUG7
                        fprintf(stderr, GRIS "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, inicial, entrada.ninodo);
                    #endif

                    //escribir la entrada en el directorio padre
                    if(mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                        
                        if(entrada.ninodo != FALLO) {
                            liberar_inodo(entrada.ninodo);
                        }

                        return FALLO;
                    }
                }
        }
    }

    //Si hemos llegado al final del camino
    if(!strcmp(final, "/") || !strcmp(final, "")) {
        if((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
            //modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        //cortamos la recursividad
        *p_inodo = entrada.ninodo;      //asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_entrada = num_entrada_inodo; //asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene

        return EXITO;

    } else {
        *p_inodo_dir = entrada.ninodo; //asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return EXITO;
}

/*
 * mostrar_error_buscar_entrada
 ---------------------------------------------------------
 * muestra los errores de buscar_entrada()
 * error: número de error
*/
void mostrar_error_buscar_entrada(int error) {
   // fprintf(stderr, "Error: %d\n", error);
   switch (error) {
   case -2: fprintf(stderr, ROJO "Error: Camino incorrecto.\n" RESET); break;
   case -3: fprintf(stderr, ROJO "Error: Permiso denegado de lectura.\n" RESET); break;
   case -4: fprintf(stderr, ROJO "Error: No existe el archivo o el directorio.\n" RESET); break;
   case -5: fprintf(stderr, ROJO "Error: No existe algún directorio intermedio.\n" RESET); break;
   case -6: fprintf(stderr, ROJO "Error: Permiso denegado de escritura.\n\n"); break;
   case -7: fprintf(stderr, ROJO "Error: El archivo ya existe.\n" RESET); break;
   case -8: fprintf(stderr, ROJO "Error: No es un directorio.\n" RESET); break;
   }
}

/*
 * mi_creat
 ---------------------------------------------------------
 * crea un fichero o directorio
 * camino: puntero a la cadena
 * permisos: permisos
 * returns: número de error, o EXITO si va bien
*/
int mi_creat(const char *camino, unsigned char permisos) {

    mi_waitSem();

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;
    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0) {
        
        mostrar_error_buscar_entrada(error);

        mi_signalSem();

        return error;
    }

    mi_signalSem();

    return EXITO;
}

/*
 * mi_dir
 ---------------------------------------------------------
 * pone el contenido del directorio en un buffer
 * camino: puntero a la cadena
 * buffer: puntero al buffer de memoria
 * tipo: tipo
 * returns: -1 caso de error, o el número de entradas
*/
int mi_dir(const char *camino, char *buffer, char tipo) {

    struct tm *tm;

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error;
    int nEntradas = 0;

    error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    struct inodo inodo;
    if(leer_inodo(p_inodo, &inodo) < 0) {
        return FALLO;
    }

    if((inodo.permisos & 4) != 4) {
        return FALLO;
    }

    char tmp[100];
    char tamEnBytes[10];
    struct entrada entrada;

    if(inodo.tipo == 'd') {

        if(leer_inodo(p_inodo, &inodo) < 0) {
            return FALLO;
        }

        tipo = inodo.tipo;

        nEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        //Buffer de salida
        struct entrada entradas[BLOCKSIZE / sizeof(struct entrada)];
        memset(&entradas, 0, sizeof(entradas));

        int offset = 0;
        offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);

        //Leemos todos las entradas
        for(int i = 0; i < nEntradas; i++) {
            
            if(leer_inodo(entradas[i % (BLOCKSIZE / sizeof(struct entrada))].ninodo, &inodo) < 0) {
                return FALLO;
            }

            //Tipo
            if(inodo.tipo == 'd') {
                strcat(buffer, "d");

            } else {
                strcat(buffer, "f");
            }

            strcat(buffer, "\t");

            //Permisos
            if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
            if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
            if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
            strcat(buffer, "\t");

            //mTime
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
            strcat(buffer, "\t");

            //Tamaño
            sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
            strcat(buffer, tamEnBytes);
            strcat(buffer, "\t");

            //Nombre
            strcat(buffer, entradas[i % (BLOCKSIZE / sizeof(struct entrada))].nombre);
            while((strlen(buffer) % TAMFILA) != 0) {
                strcat(buffer, " ");
            }

            //Siguiente
            strcat(buffer, "\n");

            if(offset % (BLOCKSIZE / sizeof(struct entrada)) == 0) {
                offset += mi_read_f(p_inodo, entradas, offset, BLOCKSIZE);
            }
        }

    } else { //Es un archivo

        if(leer_inodo(entrada.ninodo, &inodo) < 0) {
            return FALLO;
        }

        tipo = inodo.tipo;

        mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * p_entrada, sizeof(struct entrada));

        //Tipo
        strcat(buffer, "f");
        strcat(buffer, "\t");

        //Permisos
        if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
        if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
        if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
        strcat(buffer, "\t");

        //mTime
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");

        //Tamaño
        sprintf(tamEnBytes, "%d", inodo.tamEnBytesLog);
        strcat(buffer, tamEnBytes);
        strcat(buffer, "\t");

        //Nombre
        strcat(buffer, entrada.nombre);
        while ((strlen(buffer) % TAMFILA) != 0) {
            strcat(buffer, " ");
        }

        //Siguiente
        strcat(buffer, "\n");
    }

    return nEntradas;
}

/*
 * mi_chmod
 ---------------------------------------------------------
 * cambia los permisos de un fichero o directorio
 * camino: puntero a la cadena
 * permisos: nuevos permisos
 * returns: número de error, o EXITO si va bien
*/
int mi_chmod(const char *camino, unsigned char permisos) {

    // Inicializacion de variables.
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) < 0) {
        return FALLO;
    }

    p_inodo_dir = SB.posInodoRaiz;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    mi_chmod_f(p_inodo, permisos);

    return EXITO;
}

/*
 * mi_stat
 ---------------------------------------------------------
 * muestra la información acerca del inodo de un fichero o directorio
 * camino: puntero a la cadena
 * p_stat: puntero a estructura tipo STAT
 * returns: número de error, o nº de inodo
*/
int mi_stat(const char *camino, struct STAT *p_stat) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) < 0) {
        return FALLO;
    }

    p_inodo_dir = SB.posInodoRaiz;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    if(mi_stat_f(p_inodo, p_stat) < 0) {
        return FALLO;
    }

    return p_inodo;
}

/*
 * mi_write
 ---------------------------------------------------------
 * escribe texto en una posición de un fichero
 * camino: puntero a la cadena
 * buf: puntero a buffer
 * offset: posición de ficheo
 * nbytes: bytes a escribir
 * returns: número de error, o bytes escritos
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    int error = 0;
    int bytesEscritos = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) < 0) {
        return FALLO;
    }

    p_inodo_dir = SB.posInodoRaiz;

    //Vemos si escritura es sobre el mismo inodo
    if(strcmp(UltimaEntrada.camino, camino) == 0) {
        p_inodo = UltimaEntrada.p_inodo;
        #if DEBUG9
            fprintf(stderr, GRIS "[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
        #endif

    } else {
        
        if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }

        //Guardamos camino y p_inodo
        strcpy(UltimaEntrada.camino, camino);
        UltimaEntrada.p_inodo = p_inodo;

        #if DEBUG9
            fprintf(stderr, GRIS "[mi_write() → Actualizamos la caché de escritura]\n" RESET);
        #endif
    }

    //Escribimos inodo
    bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);

    return bytesEscritos;
}

/*
 * mi_read
 ---------------------------------------------------------
 * lee los nbytes del fichero indicado por camino
 * camino: puntero a la cadena
 * buf: puntero a buffer
 * offset: posición de ficheo
 * nbytes: bytes a leer
 * returns: número de error, o bytes leídos
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    
    int error = 0;
    int bytesLeidos = 0;

    //Leemos superbloque
    struct superbloque SB;
    if(bread(posSB, &SB) < 0) {
        return FALLO;
    }

    p_inodo_dir = SB.posInodoRaiz;

    if(strcmp(UltimaEntrada.camino, camino) == 0) {
        p_inodo = UltimaEntrada.p_inodo;
        #if DEBUG9
            fprintf(stderr, GRIS "[mi_write() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
        #endif

    } else {
        
        if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
            mostrar_error_buscar_entrada(error);
            return FALLO;
        }

        strcpy(UltimaEntrada.camino, camino);
        UltimaEntrada.p_inodo = p_inodo;

        #if DEBUG9
            fprintf(stderr, GRIS "[mi_read() → Actualizamos la caché de lectura]\n" RESET);
        #endif
    }

    struct inodo inodo;
    if(leer_inodo(p_inodo, &inodo) < 0) {
        return FALLO;
    }
    
    bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);
    
    return bytesLeidos;
}

/*
 * mi_link
 ---------------------------------------------------------
 * crea un enlace entre camino1 y camino2
 * camino1: puntero a camino 1
 * camino2: puntero a camino2
 * returns: número de error, o EXITO si va bien
*/
int mi_link(const char *camino1, const char *camino2) {

    mi_waitSem();

    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;
    int error;

    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    struct inodo inodo;

    //Revisamos el camino 1
    error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    if(leer_inodo(p_inodo1, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if(inodo.tipo != 'f') {
        mi_signalSem();
        return ERROR_CAMINO_INCORRECTO;
    }

    if((inodo.permisos & 4) != 4) {
        mi_signalSem();
        return ERROR_PERMISO_LECTURA;
    }

    //Revisamos el camino 2
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    struct entrada entrada2;
    if(mi_read_f(p_inodo_dir2, &entrada2, sizeof(struct entrada) * (p_entrada2), sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    //Creamos el enlace
    entrada2.ninodo = p_inodo1;

    //Escribimos la entrada
    if(mi_write_f(p_inodo_dir2, &entrada2, sizeof(struct entrada) * (p_entrada2), sizeof(struct entrada)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    //Liberamos inodo
    if(liberar_inodo(p_inodo2) < 0) {
        mi_signalSem();
        return FALLO;
    }

    //Actualizamos metainformación
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    if(escribir_inodo(p_inodo1, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();

    return EXITO;
}

/*
 * mi_unlink
 ---------------------------------------------------------
 * borra un fichero o directorio
 * camino: puntero a camino
 * returns: número de error, o EXITO si va bien
*/
int mi_unlink(const char *camino) {

    mi_waitSem();

    struct superbloque SB;
    if(bread(posSB, &SB) < 0) {
        mi_signalSem();
        return FALLO;
    }

    unsigned int p_inodo_dir, p_inodo;
    p_inodo_dir = p_inodo = SB.posInodoRaiz;

    unsigned int p_entrada = 0;
    int error;

    if((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return FALLO;
    }

    //Inodo del archivo a borrar
    struct inodo inodo;
    if(leer_inodo(p_inodo, &inodo) < 0) {
        mi_signalSem();
        return FALLO;
    }

    if((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0)) {
        fprintf(stderr, ROJO "Error: El directorio %s no está vacío\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    //Inodo del directorio.
    struct inodo inodo_dir;
    if(leer_inodo(p_inodo_dir, &inodo_dir) < 0) {
        mi_signalSem();
        return FALLO;
    }

    int num_entrada = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

    //Si no es la última
    if(p_entrada != num_entrada - 1) {
        
        struct entrada entrada;
        if(mi_read_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (num_entrada - 1), sizeof(struct entrada)) < 0) {
            mi_signalSem();
            return FALLO;
        }

        if(mi_write_f(p_inodo_dir, &entrada, sizeof(struct entrada) * (p_entrada), sizeof(struct entrada)) < 0) {
            mi_signalSem();
            return FALLO;
        }
    }

    if(mi_truncar_f(p_inodo_dir, sizeof(struct entrada) * (num_entrada - 1)) < 0) {
        mi_signalSem();
        return FALLO;
    }

    inodo.nlinks--;

    //Si no quedan enlaces
    if(!inodo.nlinks) {
        if (liberar_inodo(p_inodo) < 0) {
            mi_signalSem();
            return FALLO;
        }

    } else  {  //Actualizamos metainformación

        inodo.ctime = time(NULL);
        if(escribir_inodo(p_inodo, &inodo) < 0) {
            mi_signalSem();
            return FALLO;
        }
    }

    mi_signalSem();

    return EXITO;
}
