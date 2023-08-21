/*
 * simulacion.c -> crea procesos de prueba que acceden de forma concurrente
 *                 al sistema de archivos
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "simulacion.h"

#define DEBUG 0

int acabados = 0;


//Enterrador
void reaper(){
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended=waitpid(-1, NULL, WNOHANG))>0) {
        acabados++;
    }
}


int main(int argc, char const *argv[]) {

    //Comprobamos sintaxis
    if(argc != 2) {
        fprintf(stderr, ROJO "Sintaxis: ./simulacion <disco>\n" RESET);
        return FALLO;
    }

    //Montamos el dispositivo
    if(bmount(argv[1]) == FALLO) {
        exit(0);
    }

    //Creamos el directorio
    char camino[21] = "/simul_";
    time_t time_now;
    time(&time_now);
    struct tm *tm = localtime(&time_now);
    sprintf(camino + strlen(camino), "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

    if(mi_creat(camino, 6) == FALLO) {
        fprintf(stderr, ROJO "Error al crear el directorio '%s'\n" RESET, camino);
        exit(0);
    }

    fprintf(stderr, "*** SIMULACIÓN DE %i PROCESOS REALIZANDO CADA UNO %i ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);

    //Asociamos la señal sigchld al enterrador
    signal(SIGCHLD, reaper);

    pid_t pid;
    for(int proceso = 1; proceso <= NUMPROCESOS; proceso++) {

        pid = fork();

        if(pid == 0) {  //Hijo

            if(bmount(argv[1]) < 0) {
                return FALLO;
            }

            //Creamos directorio del proceso hijo
            char nombreDir[38];
            sprintf(nombreDir, "%sproceso_%d/", camino, getpid());
            if(mi_creat(nombreDir, 6) < 0) {
                fprintf(stderr, ROJO "Error al crear el directorio del proceso\n" RESET);
                exit(0);
            }

            //Creamos el fichero prueba.dat
            char nombreFich[48];
            sprintf(nombreFich, "%sprueba.dat", nombreDir);
            if(mi_creat(nombreFich, 6) < 0) {
                fprintf(stderr, ROJO "Error al crear el fichero prueba.dat del proceso\n" RESET);
                bumount();
                exit(0);
            }
            
            //Inicializamos numeros aleatorios
            srand(time(NULL) + getpid());

            for(int nescritura = 0; nescritura < NUMESCRITURAS; nescritura++) {

                //Inicializamos registro
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura + 1;
                registro.nRegistro = rand() % REGMAX;

                mi_write(nombreFich, &registro, (registro.nRegistro * sizeof(struct REGISTRO)), sizeof(struct REGISTRO));

                #if DEBUG
                    fprintf(stderr, "[simulación.c → Escritura %i en %s]\n", nescritura + 1, nombreFich);
                #endif

                usleep(50000); //0,05 seg
            }

            #if DEBUG12
                fprintf(stderr, GRIS "[Proceso %d: Completadas %d escrituras en %s]\n" RESET, proceso, NUMESCRITURAS, nombreFich);
            #endif

            //Desmontamos dispositivo hijo
            if(bumount() < 0) {
                return FALLO;
            }
            exit(0);  //Necesario para que se emita la señal SIGCHLD
        }

        usleep(200000); //0,15 seg
    }

    while(acabados < NUMPROCESOS) {
        pause();
    }

    //Desmontamos dispositivo padre
    if(bumount() == FALLO) {
        return FALLO;
    }

    return FALLO;

}

