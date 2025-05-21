#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ficheros.h" // Asegúrate de incluir el archivo de cabecera adecuado

#define TAMB_BUFFER 1500 // Tamaño del buffer de lectura

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, RED "Uso: %s <nombre_dispositivo> <ninodo>\n" RESET, argv[0]);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    char buffer_texto[TAMB_BUFFER];
    int leidos = 0, total_leidos = 0;
    int offset = 0;
    int tamEnBytesLog = 0;

    // Montar el dispositivo
    if (bmount(nombre_dispositivo) < 0) {
        perror("Error al montar el dispositivo");
        return FALLO;
    }

    // Obtener el tamaño lógico del fichero
    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) < 0) {
        perror("Error al obtener el tamaño lógico del fichero");
        bumount();
        return FALLO;
    }
    tamEnBytesLog = stat.tamEnBytesLog;

    // Leer el fichero bloque a bloque
    do {
        memset(buffer_texto, 0, TAMB_BUFFER); // Limpiar el buffer
        leidos = mi_read_f(ninodo, buffer_texto, offset, TAMB_BUFFER);
        if (leidos < 0) {
            perror(RED "Error al leer el fichero" RESET);
            bumount();
            return FALLO;
        }

        if (leidos > 0) {
            write(1, buffer_texto, leidos); // Escribir en la salida estándar
            total_leidos += leidos;
            offset += leidos;
        }
    } while (leidos > 0);

    // Mostrar estadísticas
    char string[128];
    //MALAMENT
    sprintf(string, "Bytes leídos: %d\n", total_leidos);
    write(2, string, strlen(string));
    //MALAMENT
    sprintf(string, "Tamaño lógico del fichero: %d bytes\n", tamEnBytesLog);
    write(2, string, strlen(string));

    // Desmontar el dispositivo
    bumount();

    return EXITO;
}