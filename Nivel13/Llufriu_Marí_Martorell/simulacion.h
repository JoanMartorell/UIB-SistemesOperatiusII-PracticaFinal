/*
 * simulacion.h -> declaración de funciones y constantes
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "directorios.h"
#include <sys/wait.h>
#include <signal.h>


#define NUMPROCESOS 100
#define NUMESCRITURAS 50
#define REGMAX 500000


struct REGISTRO { //sizeof(struct REGISTRO): 24 bytes
    time_t fecha; //Precisión segundos
    pid_t pid; //PID del proceso que lo ha creado
    int nEscritura; //Entero con el número de escritura, de 1 a 50 (orden por tiempo)
    int nRegistro; //Entero con el número del registro dentro del fichero (orden por posición)
};

