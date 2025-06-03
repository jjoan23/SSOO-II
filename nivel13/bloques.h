//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//bloques.h
//Fichero de cabecera para la gestión de bloques en un dispositivo
#include <stdio.h>  
#include <fcntl.h> 
#include <sys/stat.h> 
#include <stdlib.h>  
#include <unistd.h> 
#include <errno.h>  
#include <string.h> 


#define BLOCKSIZE 1024 // bytes


#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores

//Colores para debugging
#define BLACK      "\x1B[30m"
#define RED           "\x1b[31m"
#define GREEN      "\x1b[32m"
#define YELLOW    "\x1b[33m"
#define BLUE          "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN          "\x1b[36m"
#define WHITE        "\x1B[37m"
#define ORANGE     "\x1B[38;2;255;128;0m"
#define ROSE          "\x1B[38;2;255;151;203m"
#define LBLUE        "\x1B[38;2;53;149;240m"
#define LGREEN     "\x1B[38;2;17;245;120m"
#define GRAY          "\x1B[38;2;176;174;174m"
#define RESET        "\x1b[0m"


#define NEGRITA "\x1b[1m"

//Funciones de bloques.c
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);

// Declaración de funciones de semáforo del nivel 11
void mi_waitSem(void);
void mi_signalSem(void);