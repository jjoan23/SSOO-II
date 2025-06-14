//AUTORES: Joan Jiménez Rigo, Climent Alzamora Alcover, Marc Mateu Deyá
//simulacion.c: Simulación de escritura concurrente en un sistema de archivos
#define DEBUG12 0
#define DEBUG120 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "simulacion.h"


int acabados = 0;

void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char *argv[]) {
    char nombre_dir[TAMNOMBRE];
    char nombre_pid[TAMNOMBRE+100];
    char nombre_fichero[TAMNOMBRE+200];
    if (argc != 2) {
    #if DEBUG12
        fprintf(stderr, "Uso: %s <disco>\n", argv[0]);
    #endif
        return FALLO;
    }

    if (bmount(argv[1]) < 0) {
    #if DEBUG12
        perror("");
    #endif
        return FALLO;
    }

    
    struct tm *tm;
    time_t t = time(NULL);  
    tm = localtime(&t);
    // Añadir / al final del directorio raíz
    sprintf(nombre_dir,"/simul_%d%02d%02d%02d%02d%02d/",
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);

    if (mi_creat(nombre_dir, 7) < 0) {
        bumount();
        return FALLO;
    }
    #if DEBUG12
        printf("%s\n", nombre_dir);
    #endif

    signal(SIGCHLD, reaper);
    pid_t pid;
    for (int i = 0; i < NUMPROCESOS; i++) {
        pid = fork();

        if (pid == 0) {
            int num_pid = getpid();
            if (bmount(argv[1]) < 0) exit(FALLO);

            // Añadir / antes y después del proceso
            sprintf(nombre_pid,"%sproceso_%d/", nombre_dir, num_pid);
            if (mi_creat(nombre_pid, 7) < 0) {
                fprintf(stderr, "Error al crear directorio %s\n", nombre_pid);
                exit(FALLO);
            }

            // El archivo va dentro del directorio del proceso
            sprintf(nombre_fichero,"%sprueba.dat", nombre_pid);
            if (mi_creat(nombre_fichero, 7) < 0) {
                fprintf(stderr, "Error al crear archivo %s\n", nombre_fichero);
                exit(FALLO);
            }
            
            srand(time(NULL) + getpid());
            struct REGISTRO reg;
            for (int j = 0; j < NUMESCRITURAS; j++) {
                reg.fecha = time(NULL);
                reg.pid = getpid();
                reg.nEscritura = j+1;
                reg.nRegistro = rand() % REGMAX;
                int resultado = mi_write(nombre_fichero, &reg, reg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                if (resultado == -1) {
                    fprintf(stderr, "Error en mi_write para %s (registro %d, offset %d)\n", nombre_fichero, reg.nEscritura, reg.nRegistro);
                    return -1;
                }
            #if DEBUG12
                fprintf(stdout, GRAY"[simulacion.c → Escritura %d en %s]\n"RESET, j, nombre_fichero);
            #endif
                usleep(50000);
            }
            #if DEBUG120
              fprintf(stdout, GRAY"[Proceso %d: Completadas %d escrituras en %s]\n"RESET, i+1, NUMESCRITURAS, nombre_fichero);
            #endif
            if(bumount()==-1){
              fprintf(stderr, "Error al cerrar el fichero");
              return -1;
            } 
            exit(0);
        } 
        usleep(150000);
    }

    while (acabados < NUMPROCESOS){
        pause();
    } 
    if(bumount()==-1){
        fprintf(stderr, "Error al cerrar el fichero");
        return -1;
    }
    printf("%s\n", nombre_dir); 
    exit (0);
}
