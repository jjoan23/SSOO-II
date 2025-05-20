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
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];memset(final,0,strlen(camino_parcial));
    char tipo;
    int n_entradas, i_entrada_inodo;

    memset(inicial,0,sizeof(inicial));
    memset(final,0,sizeof(final));
    memset(&entrada, 0, sizeof(entrada));

    struct superbloque SB;
    if(bread(posSB, &SB) == -1){
        fprintf(stderr, "Error al leer el superbloque\n");
        return -1;
    }


    if (!strcmp(camino_parcial,"/")){  //camino parcial es "/"
        *p_inodo = SB.posInodoRaiz; //Raiz asociada siempre al inodo 0
        *p_entrada = 0;
        return 0;
    }

    if (extraer_camino(camino_parcial,inicial,final,&tipo) == -2) {//Si la extraccion de camino falla, abortamos la busqueda 
        return ERROR_CAMINO_INCORRECTO;
    }
    #if DEBUGN7
        fprintf(stderr, GRAY "buscar_entrada()-->inicial: %s, final: %s, reservar: %d\n" RESET, inicial, final, reservar);
    #endif


    leer_inodo(*p_inodo_dir, &inodo_dir);

    if ((inodo_dir.permisos & 4) != 4){
        return ERROR_PERMISO_LECTURA;
    }

    memset(&entrada,0,sizeof(entrada));


    //Calcular cantidad de entradas que contiene el inodo
    n_entradas = inodo_dir.tamEnBytesLog/sizeof(struct entrada);
    i_entrada_inodo = 0; //nº de entrada inicial

    struct entrada entradas[NUM_ENTRADAS];
    memset(entradas, 0, BLOCKSIZE);
    
    if (n_entradas > 0){
        mi_read_f(*p_inodo_dir, &entrada, i_entrada_inodo, sizeof(struct entrada));

        while ((i_entrada_inodo < n_entradas) && (strcmp(inicial, entradas[i_entrada_inodo % NUM_ENTRADAS].nombre))!=0){
            i_entrada_inodo++;
        }
    }
    
    if ( (i_entrada_inodo == n_entradas) && (strcmp(inicial, entradas[i_entrada_inodo % NUM_ENTRADAS].nombre)!=0) ){//La entrada no existe
        switch (reservar){
            case 0: //modo consulta. Como no existe retornamos error
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
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
                    strcpy(entradas[i_entrada_inodo % NUM_ENTRADAS].nombre, inicial);
                    if (tipo == 'd'){
                        if (strcmp(final, "/") == 0){               
                            entradas[i_entrada_inodo % NUM_ENTRADAS].ninodo = reservar_inodo('d', permisos);
                        }
                        #if DEBUGN7
                            fprintf(stderr, GRAY "buscar_entrada()-->reservado inodo %d tipo %c con permisos %d para %s\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);       
                        #endif                 
                        }else{ //cuelgan más diretorios o ficheros
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    
                    #if DEBUGN7
                        fprintf(stderr, GRAY "buscar_entrada()-->reservado inodo %d tipo %c con permisos %d para %s\n" RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                        fprintf(stderr, GRAY "buscar_entrada()-->creada entrada: %s, %d\n" RESET, entrada.nombre, entrada.ninodo);
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
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}
