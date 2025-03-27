#ifndef FICHEROS_H
#define FICHEROS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bloques.h" // Para funciones de lectura/escritura de bloques
#include "ficheros_basico.h" // Para funciones de bajo nivel (lectura/escritura de inodos)

// Definición de la estructura STAT (igual que struct inodo pero sin punteros)
struct STAT {
    unsigned char tipo;
    unsigned char permisos;
    unsigned int nlinks;
    unsigned int tamEnBytesLog;
    time_t atime;
    time_t mtime;
    time_t ctime;
    unsigned int numBloquesOcupados;
};

// Prototipos de las funciones
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat);
int mi_chmod_f(unsigned int ninodo, unsigned char permisos);


struct inodo {     // comprobar que ocupa 128 bytes haciendo un sizeof(inodo)!!!
    unsigned char tipo;     // Tipo ('l':libre, 'd':directorio o 'f':fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)
 
 
    /* Por cuestiones internas de alineación de estructuras, si se está utilizando
     un tamaño de palabra de 4 bytes (microprocesadores de 32 bits):
    unsigned char reservado_alineacion1 [2];
    en caso de que la palabra utilizada sea del tamaño de 8 bytes
    (microprocesadores de 64 bits): unsigned char reservado_alineacion1 [6]; */
    unsigned char reservado_alineacion1[6];
 
 
    time_t atime; // Fecha y hora del último acceso a datos
    time_t mtime; // Fecha y hora de la última modificación de datos
    time_t ctime; // Fecha y hora de la última modificación del inodo
    time_t btime; // Fecha y hora de creación del inodo (birth)
 
 
    /* comprobar que el tamaño del tipo time_t para vuestra plataforma/compilador es 8:
    printf ("sizeof time_t is: %ld\n", sizeof(time_t)); */
 
 
    unsigned int nlinks;             // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog;      // Tamaño en bytes lógicos (EOF)
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados zona de datos
 
 
    unsigned int punterosDirectos[12];  // 12 punteros a bloques directos
    unsigned int punterosIndirectos[3]; /* 3 punteros a bloques indirectos:
    1 indirecto simple, 1 indirecto doble, 1 indirecto triple */
 
 
    /* Utilizar una variable de alineación si es necesario  para vuestra plataforma/compilador   */
    char padding[INODOSIZE - 2 * sizeof(unsigned char) - 4 * sizeof(time_t) - 18 * sizeof(unsigned int) - 6 * sizeof(unsigned char)];
    // Fijarse que también se resta lo que ocupen las variables de alineación utilizadas!!!
 };

#endif // FICHEROS_H