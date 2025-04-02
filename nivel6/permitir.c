#include <stdio.h>
#include <stdlib.h>
#include "bloques.h" // Asegúrate de que esta cabecera contiene las funciones necesarias para montar/desmontar
#include "ficheros.h" // Asegúrate de que esta cabecera contiene mi_chmod_f()

int main(int argc, char *argv[]) {
    // Validación de sintaxis
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos> Offsets: 9000, 209000, 30725000, 409605000, 480000000 Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets \033[0m \n");
        return EXIT_FAILURE;
    }
    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    if (permisos < 0 || permisos > 7) {
        fprintf(stderr, "Error: Los permisos deben estar entre 0 y 7.\n");
        return EXIT_FAILURE;
    }

    // Montar dispositivo
    if (bmount(nombre_dispositivo) == -1) {
        perror("Error al montar el dispositivo");
        return EXIT_FAILURE;
    }

    // Llamada a mi_chmod_f()
    if (mi_chmod_f(ninodo, permisos) == -1) {
        perror("Error al cambiar los permisos del inodo");
        bumount();
        return EXIT_FAILURE;
    }

    // Desmontar dispositivo
    if (bumount() == -1) {
        perror("Error al desmontar el dispositivo");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}