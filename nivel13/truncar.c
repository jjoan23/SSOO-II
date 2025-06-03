//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//truncar.c: programa que trunca un fichero a un tamaño determinado o lo libera si se le pasa 0 como tamaño.
#include "ficheros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

int main(int argc, char **argv) {
    // Comprobamos los parámetros
    if (argc != 4) {
        fprintf(stderr, "Sintaxis errónea: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return FALLO;
    }

    // Obtenemos parámetros
    const char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    // Montamos dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        perror("Error montando el dispositivo");
        return FALLO;
    }

    int bloques_liberados;
    // Si nbytes = 0, liberar_inodo(), si no, mi_truncar_f()
    if (nbytes == 0) {
        bloques_liberados = liberar_inodo(ninodo);
        if (bloques_liberados == -1) {
            perror("Error liberando el inodo");
            bumount();
            return FALLO;
        }
        printf("Inodo %d liberado. Bloques liberados: %d\n", ninodo, bloques_liberados);
    } else {
        bloques_liberados = mi_truncar_f(ninodo, nbytes);
        if (bloques_liberados == -1) {
            perror("Error truncando el inodo");
            bumount();
            return FALLO;
        }
        printf(GRAY"Inodo %d truncado a %d bytes. Bloques liberados: %d\n"RESET, ninodo, nbytes, bloques_liberados);
    }

    // Llamamos a mi_stat_f()
    struct STAT p_stat;
    if (mi_stat_f(ninodo, &p_stat) == -1) {
        perror("Error obteniendo información del inodo");
        bumount();
        return FALLO;
    }
    
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    char btime[80];

    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    ts = localtime(&p_stat.btime);
    strftime(btime, sizeof(btime), "%a %Y-%m-%d %H:%M:%S", ts);

    // Mostramos información
    printf("\nDATOS INODO %d:\n", ninodo);
    printf("tipo=%c\n", p_stat.tipo);
    printf("permisos=%d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("btime: %s\n", btime);
    printf("nLinks=%d\n", p_stat.nlinks);
    printf("tamEnBytesLog=%d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados=%d\n", p_stat.numBloquesOcupados);

    // Desmontamos dispositivo
    if (bumount() == -1) {
        perror("Error desmontando el dispositivo");
        return FALLO;
    }

    return EXITO;
}




