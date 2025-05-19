#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bloques.h"
#include "directorios.h"

#define DEBUG4 0

void mostrar_buscar_entrada(char *camino, char reservar){
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
}

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

    // PRUEBAS de buscar_entrada()
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);  //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2

    // desmontar el dispositivo al final
    bumount();
    return EXIT_SUCCESS;
}
