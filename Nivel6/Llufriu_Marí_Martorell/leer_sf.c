/*
 * leer_sf.c -> test de prueba
 * Miembros: 
 *   - Joan Martorell Coll
 *   - Juan José Marí
 *   - Daniel Llufriu
*/

#include "ficheros_basico.h"

#define DEBUG2 0 //Debug nivel 2
#define DEBUG3 0 //Debug nivel 3
#define DEBUG4 0 //Debug nivel 4

int main(int argc, char const *argv[]) {
    //sintaxis correcta
    if(argc != 2) {
        fprintf(stderr, "Error sintaxis: ./leer_sf <nombre_dispositivo>\n");
        return FALLO;
    }

    //Montamos del disco
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }
    
    //Leemos el superbloque
    struct superbloque SB;
    if(bread(0, &SB) == -1) {
        return FALLO;
    }
    
    // Contenido del superbloque.
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);

    #if DEBUG2
        printf("\nsizeof struct superbloque: %ld\n", sizeof(struct superbloque));
        printf("sizeof struct inodo:  %ld\n", sizeof(struct inodo));

        printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

        struct inodo inodos[BLOCKSIZE / INODOSIZE];

        int inodo = 0;
        for(int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
            if(bread(i, &inodos) == -1) {
                return FALLO;
            }

            for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
                if((inodos[j].tipo == 'l')) {
                    inodo++;

                    if(inodo < SB.totInodos) {
                        printf("%d ", inodo);
                    }
                    
                    if(inodo == SB.totInodos) {
                        printf("-1 \n");
                    }
                }
            }
        }

    #endif

    #if DEBUG3
        printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");

        int reservado = reservar_bloque(); // Actualiza el SB
        bread(posSB, &SB);                 // Actualizar los valores del SB

        printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", reservado);
        printf("SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);

        liberar_bloque(reservado);
        bread(posSB, &SB); // Actualizar los valores del SB

        printf("Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\n", SB.cantBloquesLibres);


        printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

        int bit = leer_bit(posSB);
        printf("posSB: %i → leer_bit(%i) = %i\n", posSB, posSB, bit);
        bit = leer_bit(SB.posPrimerBloqueMB);
        printf("SB.posPrimerBloqueMB: %i → leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, bit);
        bit = leer_bit(SB.posUltimoBloqueMB);
        printf("SB.posUltimoBloqueMB: %i → leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, bit);
        bit = leer_bit(SB.posPrimerBloqueAI);
        printf("SB.posPrimerBloqueAI: %i → leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, bit);
        bit = leer_bit(SB.posUltimoBloqueAI);
        printf("SB.posUltimoBloqueAI: %i → leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, bit);
        bit = leer_bit(SB.posPrimerBloqueDatos);
        printf("SB.posPrimerBloqueDatos: %i → leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, bit);
        bit = leer_bit(SB.posUltimoBloqueDatos);
        printf("SB.posUltimoBloqueDatos: %i → leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, bit);

        printf("\nDATOS DEL DIRECTORIO RAIZ\n");

        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];

        struct inodo inodo;
        int ninodo = 0; // el directorio raiz es el inodo 0
        leer_inodo(ninodo, &inodo);

        ts = localtime(&inodo.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodo.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
        ts = localtime(&inodo.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

        printf("tipo: %c\n", inodo.tipo);
        printf("permisos: %i\n", inodo.permisos);
        printf("atime: %s \nctime: %s \nmtime: %s\n", atime, ctime, mtime);
        printf("nlinks: %i\n", inodo.nlinks);
        printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
        printf("numBloquesOcupados: %i\n", inodo.numBloquesOcupados);

    #endif

    #if DEBUG4

        int inodoReservado = reservar_inodo('f', 6);

        struct inodo inodo;
        leer_inodo(inodoReservado, &inodo); 

        bread(posSB, &SB);

        printf("\nINODO %d. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n", inodoReservado);
        traducir_bloque_inodo(&inodo, 8, 1);
        traducir_bloque_inodo(&inodo, 204, 1);
        traducir_bloque_inodo(&inodo, 30004, 1);
        traducir_bloque_inodo(&inodo, 400004, 1);
        traducir_bloque_inodo(&inodo, 468750, 1);

        printf("\nDATOS DEL INODO RESERVADO %d\n", inodoReservado);

        struct tm *ts;
        char atime[80];
        char mtime[80];
        char ctime[80];
        

        ts = localtime(&inodo.atime);
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

        ts = localtime(&inodo.mtime);
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

        ts = localtime(&inodo.ctime);
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

        printf("tipo: %c\n", inodo.tipo);
        printf("permisos: %i\n", inodo.permisos);
        printf("atime: %s \nctime: %s \nmtime: %s\n", atime, ctime, mtime);
        printf("nlinks: %i\n", inodo.nlinks);
        printf("tamEnBytesLog: %i\n", inodo.tamEnBytesLog);
        printf("numBloquesOcupados: %i\n", inodo.numBloquesOcupados);

        printf("\nSB.posPrimerInodoLibre = %d\n",SB.posPrimerInodoLibre);

    #endif


    // Liberación
    if(bumount() == -1) {
        return FALLO;
    }
    
    return EXITO;
}
