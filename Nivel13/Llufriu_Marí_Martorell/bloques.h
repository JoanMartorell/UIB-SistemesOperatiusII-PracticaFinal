/*
 * Bloques.h -> declaración de funciones y constantes
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()


#define BLOCKSIZE 1024 // bytes


#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores


//Semáforo
#define SEM_NAME "/mymutex" /* Usamos este nombre para el semáforo mutex */
#define SEM_INIT_VALUE 1 /* Valor inicial de los mutex */


//Colores
#define RESET "\033[0m"
#define ROJO "\x1b[31m"
#define GRIS "\x1b[1m"


//Debugs
#define DEBUG4 0    //traducir_bloque_inodo()
#define DEBUG6 1    //liberar_inodo(), liberar_bloques_inodo()
#define DEBUG7 0    //extraer_camino(), buscar_entrada()
#define DEBUG9 0    //mi_write(), mi_read()
#define DEBUG12 1   //Simulacion.c
#define DEBUG13 1   //Verificacion.c


int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);
void mi_waitSem();
void mi_signalSem();

