#include "directorios.h"
#define DEBUGN7 1

#define NUM_ENTRADAS (BLOCKSIZE / sizeof(struct entrada))

//Obtiene primer directorio o fichero del path
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo){
    const char bar = '/';
    char *dir;
    if (camino[0] != bar){
        return ERROR_CAMINO_INCORRECTO; //El primer carácter no es el separador '/' o camino vacío
    }

    camino++;
    dir = strchr(camino, bar);
    if (dir != NULL){
        strncpy(inicial,camino,(dir-camino));
        camino = dir; 
        strcpy(final,camino);
        *tipo = 'd';
    }
    else{
        strcpy(inicial,camino);
        strcpy(final, "");
        *tipo = 'f';
    }
    return EXIT_SUCCESS;

}


int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos){
    if (strcmp(camino_parcial,"/")== 0){  //camino parcial es "/"
        *p_inodo = 0; //Raiz asociada siempre al inodo 0
        *p_entrada = 0;
        return 0;
    }
    struct entrada entrada;memset(&entrada,0,sizeof(entrada));
    char inicial[sizeof(entrada.nombre)];memset(inicial,0,sizeof(entrada.nombre));
    char final[strlen(camino_parcial)];memset(final,0,strlen(camino_parcial));
    char tipo;

    if (extraer_camino(camino_parcial,inicial,final,&tipo) == -2) {//Si la extraccion de camino falla, abortamos la busqueda 
        return ERROR_CAMINO_INCORRECTO;
    }
    #if DEBUGN7
        fprintf(stderr, "buscar_entrada()-->inicial: %s, final: %s, reservar: %d\n", inicial, final, reservar);
    #endif
    struct inodo inodo_dir;
    leer_inodo(*p_inodo_dir, &inodo_dir);

    if ((inodo_dir.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }

    //Calcular cantidad de entradas que contiene el inodo
    unsigned int n_entradas = inodo_dir.tamEnBytesLog/sizeof(struct entrada);
    unsigned int i_entrada_inodo = 0; //nº de entrada inicial

    if (n_entradas > 0){
        memset(&entrada,0,sizeof(entrada));
        mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada));

        while ((i_entrada_inodo < n_entradas) && (strcmp(inicial, entrada.nombre))!=0){
            i_entrada_inodo++;
            memset(&entrada,0,sizeof(entrada));
            mi_read_f(*p_inodo_dir, &entrada, i_entrada_inodo*sizeof(struct entrada), sizeof(struct entrada));//...leer la entrada
        }
    }
    
    if ( (i_entrada_inodo == n_entradas) && (strcmp(inicial, entrada.nombre)!=0) ){//La entrada no existe
        switch (reservar){
            case 0: //modo consulta. Como no existe retornamos error
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
                break;
            case 1: //Modo escritura
                //Creamos la entrada en el directorio referenciado por *p_inodo_dir
                //si es fichero no permitir escritura
                if (inodo_dir.tipo == 'f'){
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                //si es directorio comprobar que tiene permiso de escritura
                if ((inodo_dir.permisos & 2) != 2){
                    return ERROR_PERMISO_ESCRITURA;
                } else {
                    strcpy(entrada.nombre, inicial);
                    if (tipo == 'd'){
                        if (strcmp(final, "/") == 0){               
                            entrada.ninodo = reservar_inodo('d', permisos);
                        } else{ //cuelgan más diretorios o ficheros
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    } else{
                        entrada.ninodo = reservar_inodo('f', permisos);
                    }
                    
                    #if DEBUGN7
                        fprintf(stderr, "buscar_entrada()-->reservado inodo %d tipo %c con permisos %d para %s\n", entrada.ninodo, tipo, permisos, entrada.nombre);
                        fprintf(stderr, "buscar_entrada()-->creada entrada: %s, %d\n", entrada.nombre, entrada.ninodo);
                    #endif

                    //escribir entrada
                    if (mi_write_f(*p_inodo_dir, &entrada, i_entrada_inodo*sizeof(struct entrada), sizeof(struct entrada)) == -1) { // Error de escritura
                        if (entrada.ninodo != -1){
                            liberar_inodo(entrada.ninodo);
                        }
                        return EXIT_FAILURE;
                    }
                }
                break;
        }
    }
    if(strcmp(final, "/") == 0 || strcmp(final, "") == 0){
        if((i_entrada_inodo < n_entradas) && (reservar == 1)){
            *p_entrada = i_entrada_inodo;
            return ERROR_ENTRADA_YA_EXISTENTE;//modo escritura y la entrada ya existe
        } // cortamos la recursividad

        *p_inodo = entrada.ninodo;
        *p_entrada = i_entrada_inodo;
        return EXIT_SUCCESS;
    }else{
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);

    }    
}


void mostrar_error_buscar_entrada(int error){
    switch (error){
    case -2:
        fprintf(stderr, "\033[1;31mError: Camino incorrecto.\033[0m\n");
        break;
    case -3:
        fprintf(stderr, "\033[1;31mError: Permiso denegado de lectura.\033[0m\n");
        break;
    case -4:
        fprintf(stderr, "\033[1;31mError: No existe el archivo o el directorio.\033[0m\n");
        break;
    case -5:
        fprintf(stderr, "\033[1;31mError: No existe algún directorio intermedio.\033[0m\n");
        break;
    case -6:
        fprintf(stderr, "\033[1;31mError: Permiso denegado de escritura.\033[0m\n");
        break;
    case -7:
        fprintf(stderr, "\033[1;31mError: El archivo ya existe.\033[0m\n");
        break;
    case -8:
        fprintf(stderr, "\033[1;31mError: No es un directorio.\033[0m\n");
        break;
    }
}
