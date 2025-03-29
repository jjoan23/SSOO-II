#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ficheros.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (bmount(argv[1]) == -1) {
        perror("Error montando el dispositivo");
        return EXIT_FAILURE;
    }

    struct superbloque SB;
    if (bread(0, &SB) == -1) {
        perror("Error leyendo el superbloque");
        bumount();
        return EXIT_FAILURE;
    }

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %u\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %u\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %u\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %u\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %u\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %u\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %u\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %u\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %u\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %u\n", SB.cantInodosLibres);
    printf("totBloques = %u\n", SB.totBloques);
    printf("totInodos = %u\n", SB.totInodos);

    printf("\nINODO 1. TRADUCCIÓN DE LOS BLOQUES LÓGICOS 8, 204, 30004, 400004 y 468750\n");
    unsigned int bloques[] = {8, 204, 30004, 400004, 468750};
    int bf;
    for (int i = 0; i < 5; i++) {
        bf = traducir_bloque_inodo(1, bloques[i], 0); // Cambiado reservar = 0 para solo traducir
        if (bf != FALLO) {
            printf("[traducir_bloque_inodo() -> BF %d para BL %u]\n", bf, bloques[i]);
        } else {
            printf("[traducir_bloque_inodo() -> No asignado para BL %u]\n", bloques[i]);
        }
    }

    struct inodo inodo;
    if (leer_inodo(1, &inodo) == FALLO) {
        perror("Error leyendo el inodo");
        bumount();
        return EXIT_FAILURE;
    }

    struct tm *ts;
    char atime[80], mtime[80], ctime[80];
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("\nDATOS DEL INODO 1\n");
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);

    bumount();
    return EXIT_SUCCESS;
}