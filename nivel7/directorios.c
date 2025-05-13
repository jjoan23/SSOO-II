int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    if (camino[0] != '/') { // CAMINO_INCORRECTO
        return -1;
    }

    const char *primer_char = camino + 1; // Skip first '/'
    const char *siguiente_barra = strchr(primer_char, '/');

    if (siguiente_barra) { // Es un directorio
        strncpy(inicial, primer_char, siguiente_barra - primer_char);
        inicial[siguiente_barra - primer_char] = '\0';
        strcpy(final, siguiente_barra);
        *tipo = 'd';
    } else { // Es un fichero
        strcpy(inicial, primer_char);
        strcpy(final, "");
        *tipo = 'f';
    }

    return 0;
}


int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo,num_entrada_inodo;
    
  
    if (strcmp(camino_parcial, "/") == 0) {
        struct superbloque SB;
        bread(posSB, &SB);
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return 0;
    }
    if(extraer_camino(camino_parcial, inicial, final, &tipo)=FALLO){
        return ERROR_CAMINO_INCORRECTO;
    }
    //AQUEST NO SE SI SE COMPROVA AIXI
    if(leer_inodo(*p_inodo_dir, &inodo_dir)==FALLO){
        return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
    }
      // Buffer para optimizar la lectura
    struct entrada buffer_entradas[BLOCKSIZE/sizeof(struct entrada)];
    memset(buffer_entradas, 0, BLOCKSIZE);
    
    // Calcular cantidad de entradas del inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0;
    
    if (cant_entradas_inodo > 0) {
        int offset = 0;
        int indice_buffer = BLOCKSIZE/sizeof(struct entrada);
        
        while (num_entrada_inodo < cant_entradas_inodo && strcmp(inicial, buffer_entradas[indice_buffer].nombre) != 0) {
            // Si hemos llegado al final del buffer, leer siguiente bloque
            if (indice_buffer == BLOCKSIZE/sizeof(struct entrada)) {
                memset(buffer_entradas, 0, BLOCKSIZE);
                mi_read_f(*p_inodo_dir, buffer_entradas, offset, BLOCKSIZE)                
                indice_buffer = 0;
                offset += BLOCKSIZE;
            }
            num_entrada_inodo++;
            indice_buffer++;
        }
    }
    if ((strincmp(inicial, entrada.nombre) == 0) && (num_entrada_inodo == cant_entradas_inodo)) {
        swicth(reservar){
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            case 1:
                if (inodo_dir.tipo == 'f') {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }
                if ((inodo_dir.permisos & 2) != 2) {
                    return ERROR_PERMISO_ESCRITURA;
                }
                strcpy(entrada.nombre, inicial);
                
                if (tipo == 'd') {
                    if (strcmp(final, "/") == 0) {
                        // Reservar inodo como directorio
                        entrada.ninodo = reservar_inodo('d', permisos);
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else {
                    // Reservar inodo como fichero
                    entrada.ninodo = reservar_inodo('f', permisos);
                }
                
                // Escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                    if (entrada.ninodo != FALLO) {
                        liberar_inodo(entrada.ninodo);
                    }
                    return FALLO;
                }
        }
    
    }
    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    
    return EXITO;
}
void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case -2: fprintf(stderr, "Error: Camino incorrecto.\n"); break;
   case -3: fprintf(stderr, "Error: Permiso denegado de lectura.\n"); break;
   case -4: fprintf(stderr, "Error: No existe el archivo o el directorio.\n"); break;
   case -5: fprintf(stderr, "Error: No existe algún directorio intermedio.\n"); break;
   case -6: fprintf(stderr, "Error: Permiso denegado de escritura.\n"); break;
   case -7: fprintf(stderr, "Error: El archivo ya existe.\n"); break;
   case -8: fprintf(stderr, "Error: No es un directorio.\n"); break;
   }
}
