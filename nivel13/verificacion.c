// AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
// verificacion.c: Verificación de registros escritos por múltiples procesos.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "verificacion.h"

#define BLOQUE_REG 256
#define MAX_ENTRADAS 500000
#define DEBUG13 1

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <disco> <directorio_simulacion>\n", argv[0]);
        return FALLO;
    }

    char *disco = argv[1];
    char *directorio = argv[2];

    if (bmount(disco) == -1)
    {
        fprintf(stderr, "Error montando disco\n");
        return FALLO;
    }

    struct STAT info;
    if (mi_stat(directorio, &info) == -1)
    {
        fprintf(stderr, "No se pudo obtener información del directorio\n");
        return FALLO;
    }

    int total = info.tamEnBytesLog / sizeof(struct entrada);
    struct entrada lista[total];

    if (mi_read(directorio, lista, 0, sizeof(lista)) == -1)
    {
        fprintf(stderr, "Fallo al leer entradas del directorio\n");
        return FALLO;
    }

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    if (buscar_entrada(directorio, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0) == FALLO)
    {
        fprintf(stderr, "Error al buscar entrada del directorio\n");
        return FALLO;
    }

    printf("dir_sim: %s\n", directorio);
    printf("Nº de inodo: %d\n", p_inodo);
    printf("numentradas: %d NUMPROCESOS: %d\n", total, NUMPROCESOS);

    char informe[128];
    sprintf(informe, "%sinforme.txt", directorio);

    if (mi_creat(informe, 7) == -1)
    {
        fprintf(stderr, "No se pudo crear informe.txt\n");
        return FALLO;
    }

    int bytes_escritos = 0;

    for (int i = 0; i < total; i++)
    {
        pid_t pid = atoi(strchr(lista[i].nombre, '_') + 1);

        char fichero[128];
        sprintf(fichero, "%s%s/%s", directorio, lista[i].nombre, "prueba.dat");

        struct INFORMACION datos;
        struct REGISTRO bloque[(BLOCKSIZE / sizeof(struct REGISTRO)) * 200];
        int desplazamiento = 0;
        int contador = 0;

        while (mi_read(fichero, bloque, desplazamiento, sizeof(bloque)) > 0)
        {
            for (int j = 0; j < (BLOCKSIZE / sizeof(struct REGISTRO)) * 200; j++)
            {
                if (bloque[j].pid == pid)
                {
                    if (contador == 0)
                    {
                        datos.PrimeraEscritura = datos.UltimaEscritura = datos.MenorPosicion = datos.MayorPosicion = bloque[j];
                    }
                    else
                    {
                        if (difftime(datos.PrimeraEscritura.fecha, bloque[j].fecha) > 0 ||
                            (difftime(datos.PrimeraEscritura.fecha, bloque[j].fecha) == 0 &&
                             bloque[j].nEscritura < datos.PrimeraEscritura.nEscritura))
                        {
                            datos.PrimeraEscritura = bloque[j];
                        }

                        if (difftime(datos.UltimaEscritura.fecha, bloque[j].fecha) < 0 ||
                            (difftime(datos.UltimaEscritura.fecha, bloque[j].fecha) == 0 &&
                             bloque[j].nEscritura > datos.UltimaEscritura.nEscritura))
                        {
                            datos.UltimaEscritura = bloque[j];
                        }

                        if (bloque[j].nRegistro < datos.MenorPosicion.nRegistro)
                        {
                            datos.MenorPosicion = bloque[j];
                        }

                        if (bloque[j].nRegistro > datos.MayorPosicion.nRegistro)
                        {
                            datos.MayorPosicion = bloque[j];
                        }
                    }
                    contador++;
                }
            }
            memset(bloque, 0, sizeof(bloque));
            desplazamiento += sizeof(bloque);
        }

#if DEBUG13
        fprintf(stderr, GRAY "[%d) %d escrituras validadas en %s]\n" RESET, i + 1, contador, fichero);
#endif

        char salida[BLOCKSIZE];
        memset(salida, 0, BLOCKSIZE);

        // Formato de fecha
        char fecha[30];
        struct tm *tm_info;

        sprintf(salida, "PID: %d\nNumero de escrituras: %d\n", pid, contador);

        tm_info = localtime(&datos.PrimeraEscritura.fecha);
        strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(salida + strlen(salida), "Primera escritura: %d %d %s\n",
                datos.PrimeraEscritura.nEscritura,
                datos.PrimeraEscritura.nRegistro,
                fecha);

        tm_info = localtime(&datos.UltimaEscritura.fecha);
        strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(salida + strlen(salida), "Ultima escritura: %d %d %s\n",
                datos.UltimaEscritura.nEscritura,
                datos.UltimaEscritura.nRegistro,
                fecha);

        tm_info = localtime(&datos.MenorPosicion.fecha);
        strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(salida + strlen(salida), "Menor posicion: %d %d %s\n",
                datos.MenorPosicion.nEscritura,
                datos.MenorPosicion.nRegistro,
                fecha);

        tm_info = localtime(&datos.MayorPosicion.fecha);
        strftime(fecha, sizeof(fecha), "%Y-%m-%d %H:%M:%S", tm_info);
        sprintf(salida + strlen(salida), "Mayor posicion: %d %d %s\n\n",
                datos.MayorPosicion.nEscritura,
                datos.MayorPosicion.nRegistro,
                fecha);

        if (mi_write(informe, salida, bytes_escritos, strlen(salida)) == -1)
        {
            fprintf(stderr, "Error escribiendo en el informe\n");
        }

        bytes_escritos += strlen(salida);
    }

    bmount(disco);
    return EXITO;
}
