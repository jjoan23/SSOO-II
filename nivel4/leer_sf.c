#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bloques.h"
#include "ficheros_basico.h"

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
    
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int bloque = reservar_bloque();
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", bloque);
    printf("SB.cantBloquesLibres = %u\n", --SB.cantBloquesLibres);
    liberar_bloque(bloque);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %u\n", ++SB.cantBloquesLibres);
    
    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    unsigned int posiciones[] = {0, SB.posPrimerBloqueMB, SB.posUltimoBloqueMB, SB.posPrimerBloqueAI, SB.posUltimoBloqueAI, SB.posPrimerBloqueDatos, SB.posUltimoBloqueDatos};
    for (int i = 0; i < 7; i++) {
        int bit = leer_bit(posiciones[i]);
        printf("SB.pos %u → leer_bit(%u) = %d\n", posiciones[i], posiciones[i], bit);
    }
    
    struct inodo inodo;
    leer_inodo(0, &inodo);
    struct tm *ts;
    char atime[80], mtime[80], ctime[80], btime[80];
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %u\n", inodo.permisos);
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);
    printf("btime: %s\n", btime);
    printf("nlinks: %u\n", inodo.nlinks);
    printf("tamEnBytesLog: %u\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", inodo.numBloquesOcupados);
    
    bumount();
    return EXIT_SUCCESS;
}