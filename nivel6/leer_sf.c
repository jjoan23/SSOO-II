#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "bloques.h"
#include "ficheros_basico.h"

#define DEBUG4 0

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
     // Traducir bloques lógicos
     int ninodo = reservar_inodo('f', 6); // Tipo 'f' (fichero), permisos 6 (lectura y escritura)
     unsigned int bloques_logicos[] = {8, 204, 30004, 400004, 468750};
     unsigned int nblogico, nbfisico;
     for (int i = 0; i < sizeof(bloques_logicos) / sizeof(bloques_logicos[0]); i++) {
         nblogico = bloques_logicos[i];
         nbfisico = traducir_bloque_inodo(ninodo, nblogico, 1); // Reservar si no existe
         if (nbfisico == -1) {
             perror("Error traduciendo bloque lógico");
             bumount();
             return EXIT_FAILURE;
         }
         printf("Bloque lógico %u traducido a bloque físico %u\n", nblogico, nbfisico);
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
    
    bumount();
    return EXIT_SUCCESS;
}
