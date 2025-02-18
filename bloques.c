#include "bloques.h"

//hs


static int descriptor = 0;

int bmount(const char *camino){

descriptor = open(camino,O_RDWR | O_CREAT,0666); 

if(descriptor ==-1){
    perror("");
    return FALLO;
}

return descriptor;
}

int bumount(){
int i = close(descriptor);
if(i==-1){
    perror("");
    return FALLO;
}
return EXITO;

}


int bwrite(unsigned int nbloque, const void *buf){

 if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) == -1) {
        perror("");
        return FALLO;
    }
ssize_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if (bytes_escritos == -1) {
        perror("Error en write");
        return FALLO;
    }

    return bytes_escritos;

}

int bread(unsigned int nbloque, void *buf){

}