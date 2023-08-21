/*
 * simulacion.c -> recorre secuencialmente el fichero "prueba.dat" de cada proceso
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "verificacion.h"

int main(int argc, char const *argv[]) {

    //Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
        return FALLO;
    }

    //Montamos dispositivo
    if(bmount(argv[1]) == FALLO) {
        return FALLO;
    }

    //Calculamos nº de entradas del directorio
    struct STAT stat;
    mi_stat(argv[2], &stat);

    fprintf(stderr, "dir_sim: %s\n", argv[2]);

    int numEntradas = (stat.tamEnBytesLog / sizeof(struct entrada));
    if(numEntradas != NUMPROCESOS) {
        fprintf(stderr, ROJO "Error en el número de entradas (%i).\n" RESET, numEntradas);
        //Desmontamos dispositivo
        if(bumount() < 0) {
            return FALLO;
        }
        return FALLO;
    }

    fprintf(stderr, "numentradas: %i NUMPROCESOS: %i\n", numEntradas, NUMPROCESOS);

    //Creamos el fichero "informe.txt"
    char resultado[100];
    sprintf(resultado, "%s%s", argv[2], "informe.txt");
    if(mi_creat(resultado, 6) < 0) {
        //Desmontamos dispositivo
        if(bumount() < 0) {
            return FALLO;
        }
        return FALLO;
    }

    //Leemos los directorios correspondientes a los procesos
    //MEJORA IMPLEMENTADA: Se leen todas las entradas al principio del bucle 
    struct entrada entradas[NUMPROCESOS * sizeof (struct entrada)];
    memset(entradas, 0, sizeof(entradas)); //Rellenamos con 0s

    int error = mi_read(argv[2], entradas, 0, sizeof(entradas));
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    int nbytes = 0;
    for(int entrada = 0; entrada < numEntradas; entrada++) {

        //Extraemos PID
        pid_t pid = atoi(strchr(entradas[entrada].nombre, '_') + 1);

        //Guardamos en registro
        struct INFORMACION info;
        info.pid = pid;
        info.nEscrituras = 0;

        char pruebaFich[128];
        sprintf(pruebaFich, "%s%s/%s", argv[2], entradas[entrada].nombre, "prueba.dat");

        //Recorremos secuencialmente el fichero prueba.dat utilizando buffer de N registros de escrituras
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras [cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));

        int offset = 0;
        int leidos = 0;
        while((leidos = mi_read(pruebaFich, buffer_escrituras, offset, sizeof(buffer_escrituras))) > 0) {
            
            offset += leidos;
            int registro = 0;

            while(registro < cant_registros_buffer_escrituras) {
                
                if(buffer_escrituras[registro].pid == info.pid) {  //Escritura es vàlida

                    if(info.nEscrituras == 0) {  //Primera escritura validada

                        //Inicializamos registros significativos
                        info.MenorPosicion = buffer_escrituras[registro];
                        info.MayorPosicion = buffer_escrituras[registro];
                        info.PrimeraEscritura = buffer_escrituras[registro];
                        info.UltimaEscritura = buffer_escrituras[registro];
                        info.nEscrituras++;

                    } else {
                        //Comparamos nº de escritura
                        if((difftime(buffer_escrituras[registro].fecha, info.PrimeraEscritura.fecha)) <= 0 &&
                            buffer_escrituras[registro].nEscritura < info.PrimeraEscritura.nEscritura) {

                            info.PrimeraEscritura = buffer_escrituras[registro];
                        }

                        if((difftime(buffer_escrituras[registro].fecha, info.UltimaEscritura.fecha)) >= 0 &&
                            buffer_escrituras[registro].nEscritura > info.UltimaEscritura.nEscritura) {

                            info.UltimaEscritura = buffer_escrituras[registro];
                        }

                        if(buffer_escrituras[registro].nRegistro < info.MenorPosicion.nRegistro) {
                            info.MenorPosicion = buffer_escrituras[registro];
                        }

                        if(buffer_escrituras[registro].nRegistro > info.MayorPosicion.nRegistro) {
                            info.MayorPosicion = buffer_escrituras[registro];
                        }

                        info.nEscrituras++;
                    }
                }

                registro++;
            }

            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
        }

        #if DEBUG13
            fprintf(stderr, GRIS "[%i) %i escrituras validadas en %s]\n" RESET, entrada + 1, info.nEscrituras, pruebaFich);
        #endif

        //Añadimos información
        char tiempoPrimero[100];
        char tiempoUltimo[100];
        char tiempoMenor[100];
        char tiempoMayor[100];
        
        struct tm *tm;
        tm = localtime(&info.PrimeraEscritura.fecha);
        strftime(tiempoPrimero, sizeof(tiempoPrimero), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.UltimaEscritura.fecha);
        strftime(tiempoUltimo, sizeof(tiempoUltimo), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MenorPosicion.fecha);
        strftime(tiempoMenor, sizeof(tiempoMenor), "%a %Y-%m-%d %H:%M:%S", tm);
        tm = localtime(&info.MayorPosicion.fecha);
        strftime(tiempoMayor, sizeof(tiempoMayor), "%a %Y-%m-%d %H:%M:%S", tm);

        char buffer[BLOCKSIZE];
        memset(buffer, 0, BLOCKSIZE);

        sprintf(buffer, "PID: %i\nNumero de escrituras: %i\n", pid, info.nEscrituras);
        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Primera escritura",
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                asctime(localtime(&info.PrimeraEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Ultima escritura",
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                asctime(localtime(&info.UltimaEscritura.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Menor posicion",
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                asctime(localtime(&info.MenorPosicion.fecha)));

        sprintf(buffer + strlen(buffer), "%s %i %i %s",
                "Mayor posicion",
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                asctime(localtime(&info.MayorPosicion.fecha)));

        sprintf(buffer,
                "PID: %d\nNumero de escrituras:\t%d\n"
                "Primera escritura:\t%d\t%d\t%s\n"
                "Ultima escritura:\t%d\t%d\t%s\n"
                "Menor posición:\t\t%d\t%d\t%s\n"
                "Mayor posición:\t\t%d\t%d\t%s\n\n",
                info.pid, info.nEscrituras,
                info.PrimeraEscritura.nEscritura,
                info.PrimeraEscritura.nRegistro,
                tiempoPrimero,
                info.UltimaEscritura.nEscritura,
                info.UltimaEscritura.nRegistro,
                tiempoUltimo,
                info.MenorPosicion.nEscritura,
                info.MenorPosicion.nRegistro,
                tiempoMenor,
                info.MayorPosicion.nEscritura,
                info.MayorPosicion.nRegistro,
                tiempoMayor);

        //Escribimos en "resultado.txt"
        if(mi_write(resultado, &buffer, nbytes, strlen(buffer)) < 0) {

            fprintf(stderr, ROJO "Error al escribir el fichero: '%s'\n" RESET, resultado);
            //Desmontamos dispositivo
            if(bumount() < 0) {
                return FALLO;
            }
            return FALLO;
        }

        nbytes += strlen(buffer);

    }


    //Desmontamos dispositivo
    if(bumount() < 0) {
        return FALLO;
    }
}

