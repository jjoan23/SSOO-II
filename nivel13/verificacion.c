//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//verificacion.c: programa que verifica los registros de escritura de varios procesos en un sistema de archivos simulado.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "verificacion.h"

#define REGISTROS_BUFFER 256

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <disco> <directorio_simulacion>\n", argv[0]);
        return FALLO;
    }

    if (bmount(argv[1]) < 0) {
        perror("bmount");
        return FALLO;
    }

    char *dir_sim = argv[2];
    struct STAT stat;
    if (mi_stat(dir_sim, &stat) < 0) {
        fprintf(stderr, "Error al obtener stat de %s\n", dir_sim);
        bumount();
        return FALLO;
    }

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);
    printf("dir_sim: %s\n", dir_sim);
    printf("numentradas: %d NUMPROCESOS: %d\n", numentradas, NUMPROCESOS);

    if (numentradas != NUMPROCESOS) {
        fprintf(stderr, "ERROR: numentradas != NUMPROCESOS\n");
        bumount();
        return FALLO;
    }

    char path_informe[300];
    sprintf(path_informe, "%sinforme.txt", dir_sim);
    if (mi_creat(path_informe, 6) < 0) {
        fprintf(stderr, "Error al crear informe.txt\n");
        bumount();
        return FALLO;
    }

    struct entrada entradas[NUMPROCESOS];
    mi_read(dir_sim, entradas, 0, sizeof(entradas));

    for (int i = 0; i < NUMPROCESOS; i++) {
        struct INFORMACION info;
        char *nombre = entradas[i].nombre;
        char *guion = strchr(nombre, '_');
        if (!guion) continue;
        info.pid = atoi(guion + 1);
        info.nEscrituras = 0;

        char path_fichero[300];
        sprintf(path_fichero, "%s%s/prueba.dat", dir_sim, nombre);

        int offset = 0, leidos;
        struct REGISTRO buffer[REGISTROS_BUFFER];
        memset(buffer, 0, sizeof(buffer));
        int primera = 1;

        while ((leidos = mi_read(path_fichero, buffer, offset, sizeof(buffer))) > 0) {
            int nregs = leidos / sizeof(struct REGISTRO);
            for (int j = 0; j < nregs; j++) {
                if (buffer[j].pid == info.pid) {
                    if (primera) {
                        info.PrimeraEscritura = buffer[j];
                        info.UltimaEscritura = buffer[j];
                        info.MenorPosicion = buffer[j];
                        info.MayorPosicion = buffer[j];
                        primera = 0;
                    } else {
                        if (buffer[j].nEscritura < info.PrimeraEscritura.nEscritura) info.PrimeraEscritura = buffer[j];
                        if (buffer[j].nEscritura > info.UltimaEscritura.nEscritura) info.UltimaEscritura = buffer[j];
                        if (buffer[j].nRegistro < info.MenorPosicion.nRegistro) info.MenorPosicion = buffer[j];
                        if (buffer[j].nRegistro > info.MayorPosicion.nRegistro) info.MayorPosicion = buffer[j];
                    }
                    info.nEscrituras++;
                }
            }
            offset += leidos;
        }

        char salida[1024];
        sprintf(salida,
            "\nPID: %d\nNumero de escrituras: %d\n"
            "Primera Escritura\t%u\t%u\t%s"
            "Ultima Escritura\t%u\t%u\t%s"
            "Menor Posicion\t%u\t%u\t%s"
            "Mayor Posicion\t%u\t%u\t%s\n",
            info.pid, info.nEscrituras,
            info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)),
            info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)),
            info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)),
            info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha))
        );

        mi_write(path_informe, salida, strlen(salida), 0);

        printf("[%d) %d escrituras validadas en %s]\n", i+1, info.nEscrituras, path_fichero);
    }

    bumount();
    return EXITO;
}
