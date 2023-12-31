/*
 * truncar.c -> programa externo ficticio, sólo para probar funciones
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros.h"


int main(int argc, char *argv[]) {

    //Comprobamos los parametros
    if(argc != 4) {
        fprintf(stderr, "Sintaxis errónea: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return FALLO;
    }

    //Obtenemos parámetros
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    //Montamos dispositivo
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }

    //si nbytes= 0 liberar_inodo() si_no mi_truncar_f() fsi
    if(nbytes == 0) {
        if(liberar_inodo(ninodo) == -1) {
            return FALLO;
        }

    } else {
        mi_truncar_f(ninodo, nbytes);
    }

    //Llamamos a mi_stat_f()
    struct STAT p_stat;
    if(mi_stat_f(ninodo, &p_stat) == -1) {
        return FALLO;
    }

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    //Mostramos información
    printf("\nDATOS INODO %d:\n", ninodo);

    printf("tipo=%c\n", p_stat.tipo);
    printf("permisos=%d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nLinks=%d\n", p_stat.nlinks);
    printf("tamEnBytesLog=%d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados=%d\n", p_stat.numBloquesOcupados);

    //Desmontamos dispositivo
    if(bumount() == -1) {
        return FALLO;
    }

    return EXITO;
}
