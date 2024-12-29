//Código final proyecto Sistemas Operativos 24/25.
//Álvaro Arroyo 29/12/2024.

//Añadimos todas las bibliotecas necesarias. 
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#define LONGITUD_COMANDO 100

//Declaramos las cabeceras de las funciones.
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre, FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

//Muestra la información del superbloque.
void Info(EXT_SIMPLE_SUPERBLOCK *superblock) {
    printf("Numero total de inodos: %d\n", superblock->s_inodes_count);
    printf("Numero total de bloques: %d\n", superblock->s_blocks_count);
    printf("Bloques libres: %d\n", superblock->s_free_blocks_count);
    printf("Inodos libres: %d\n", superblock->s_free_inodes_count);
    printf("Primer bloque de datos: %d\n", superblock->s_first_data_block);
    printf("Tamaño del bloque: %d bytes\n", superblock->s_block_size);
}

//Imprime los mapas de bytes de bloques e inodos.
void Bytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    printf("Bytemap de bloques:\n");
    for (int i = 0; i < 25; i++) {
        printf("%d ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\nBytemap de inodos:\n");
    for (int i = 0; i < MAX_INODOS; i++) {
        printf("%d ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

//Muestra una lista de los archivos que se encuentran en el directorio.
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("Listado de archivos:\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            printf("Archivo: %s, Inodo: %d\n", directorio[i].dir_nfich, directorio[i].dir_inodo);
        }
    }
}

//Permite cambiar el nombre de uno de los archivos del directorio.
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo) {
    //Busca el índice del archivo original.
    int index = BuscaFich(directorio, inodos, nombreantiguo);

    //Error si BuscaFich devuelve -1.
    if (index < 0) {
        printf("Error: archivo no encontrado.\n");
        return -1;
    }

    //Comprueba que el nuevo nombre no coincide con el de un archivo existente.
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombrenuevo) == 0) {
            printf("Error: el nuevo nombre ya existe.\n");
            return -1;
        }
    }
    //Copia el nuevo nombre en la entrada del archivo original.
    strcpy(directorio[index].dir_nfich, nombrenuevo);
    printf("Archivo renombrado con éxito.\n");
    return 0;
}

//Permite eliminar un archivo.
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, char *nombre, FILE *fich) {
    //Busca el índice del archivo original.
    int index = BuscaFich(directorio, inodos, nombre);

    //Error si BuscaFich devuelve -1.
    if (index < 0) {
        printf("Error: archivo no encontrado.\n");
        return -1;
    }

    //Obtiene un puntero al inodo asociado al archivo encontrado.
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[index].dir_inodo];

    //Libera todos los bloques de datos asociados al archivo.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo->i_nbloque[i] != NULL_BLOQUE) { //Si el bloque no es nulo:
            ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0; //Marca el bloque como libre en el mapa de bloques.
            superblock->s_free_blocks_count++; //Incrementa el contador de bloques libres en el superbloque.
        }
    }

    //Limpia el inodo marcándolo como no utilizado.
    inodo->size_fichero = 0; //Establece el tamaño del archivo a 0.
    memset(inodo->i_nbloque, NULL_BLOQUE, sizeof(inodo->i_nbloque)); //Borra la asignación de bloques en el inodo.
    ext_bytemaps->bmap_inodos[directorio[index].dir_inodo] = 0; //Marca el inodo como libre en el mapa de inodos.
    superblock->s_free_inodes_count++; //Incrementa el contador de inodos libres en el superbloque.

    //Borra la entrada del archivo en el directorio.
    strcpy(directorio[index].dir_nfich, "");
    directorio[index].dir_inodo = NULL_INODO;

    printf("Archivo eliminado con éxito.\n");
    return 0;
}

//Copia un archivo existente en uno completamente nuevo.
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich) {
    //Verifica que el archivo de origen existe.
    int index = BuscaFich(directorio, inodos, nombreorigen);
    if (index < 0) {
        printf("Error: archivo de origen no encontrado.\n");
        return -1;
    }

    //Verifica que el archivo de destino no exista.
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombredestino) == 0) {
            printf("Error: el archivo de destino ya existe.\n");
            return -1;
        }
    }

    //Obtiene el inodo del archivo de origen.
    EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[index].dir_inodo];

    //Busca un inodo libre para el archivo de destino.
    int nuevo_inodo = -1;
    for (int i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            nuevo_inodo = i;
            ext_bytemaps->bmap_inodos[i] = 1; //Marca el inodo como ocupado.
            superblock->s_free_inodes_count--;
            break;
        }
    }
    if (nuevo_inodo == -1) {
        printf("Error: no hay inodos libres.\n");
        return -1;
    }

    //Asigna el nuevo inodo y copia el tamaño del archivo.
    EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[nuevo_inodo];
    inodo_destino->size_fichero = inodo_origen->size_fichero;

    //Copia los bloques de datos del archivo origen al destino.
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo_origen->i_nbloque[i] != NULL_BLOQUE) {
            int nuevo_bloque = -1;

            //Encuentra un bloque libre para asignarlo.
            for (int j = 0; j < MAX_BLOQUES_DATOS; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    nuevo_bloque = j;
                    ext_bytemaps->bmap_bloques[j] = 1; //Marca el bloque como ocupado.
                    superblock->s_free_blocks_count--;
                    break;
                }
            }
            if (nuevo_bloque == -1) {
                printf("Error: no hay bloques libres.\n");
                return -1;
            }

            //Copia los datos del bloque origen al bloque destino.
            inodo_destino->i_nbloque[i] = nuevo_bloque;
            memcpy(memdatos[nuevo_bloque].dato, memdatos[inodo_origen->i_nbloque[i]].dato, SIZE_BLOQUE);
        } else {
            inodo_destino->i_nbloque[i] = NULL_BLOQUE; //Finaliza si no hay más bloques.
        }
    }

    //Crea una nueva entrada en el directorio para el archivo copiado.
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            strcpy(directorio[i].dir_nfich, nombredestino);
            directorio[i].dir_inodo = nuevo_inodo;
            printf("Archivo copiado con éxito.\n");
            return 0;
        }
    }

    //Error si no hay espacio en el directorio.
    printf("Error: no hay entradas libres en el directorio.\n");
    return -1;
}

//Verifica que un archivo existe en el directorio y devuelve su índice si lo encuentra.
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre) {
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            return i; //Índice del archivo encontrado.
        }
    }
    return -1; //No se ha encontrado ningún archivo.
}

//Verifica que el comando introducido por el usuario es válido y extrae la orden y argumentos.
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2) {
    int num_args = sscanf(strcomando, "%s %s %s", orden, argumento1, argumento2);
    if (num_args > 0) {
        return 0;
    }
    return -1; //Comando inválido.
}

//Muestra el contenido de un archivo especifico en la terminal.
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    //Busca el archivo en el directorio.
    int index = BuscaFich(directorio, inodos, nombre);
    if (index < 0) {
        printf("Error: archivo no encontrado.\n");
        return -1; //El archivo no existe.
    }

    //Recupera los bloques de datos asociados al archivo y los imprime.
    EXT_SIMPLE_INODE inodo = inodos->blq_inodos[directorio[index].dir_inodo];
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        if (inodo.i_nbloque[i] == NULL_BLOQUE) {
            break; //Finaliza si no hay más bloques asignados.
        }
        printf("%s", memdatos[inodo.i_nbloque[i]].dato); // Imprime los datos del bloque.
    }
    return 0;
}

//Bucle principal para la gestión del sistema de archivos.
int main() {
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    //Estructuras que representan el sistema de archivos.
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

    //Abrimos el archivo binario que representa la partición.
    FILE *fent = fopen("particion.bin", "r+b");
    if (!fent) {
        printf("Error al abrir particion.bin\n");
        return -1;
    }

    //Leemos y cargamos en memoria las estructuras del sistema de archivos.
    fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&directorio, (EXT_ENTRADA_DIR *)&datosfich[3], sizeof(directorio));
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(&memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    //Bucle principal para gestionar los comandos del usuario.
    while (1) {
        printf(">>> ");
        fflush(stdin);
        fgets(comando, LONGITUD_COMANDO, stdin);

        //Verificamos la validez del comando y lo procesamos.
        if (ComprobarComando(comando, orden, argumento1, argumento2) != 0) {
            printf("Comando desconocido\n");
            continue;
        }

        if (strcmp(orden, "info") == 0) {
            Info(&ext_superblock);
        } else if (strcmp(orden, "bytemaps") == 0) {
            Bytemaps(&ext_bytemaps);
        } else if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
        } else if (strcmp(orden, "imprimir") == 0) {
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
        } else if (strcmp(orden, "rename") == 0) {
            Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
        } else if (strcmp(orden, "remove") == 0) {
            Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
        } else if (strcmp(orden, "copy") == 0) {
            Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
        } else if (strcmp(orden, "salir") == 0) {
            //Guarda los cambios y cierra el archivo.
           
            fclose(fent);
            return 0;
        } else {
            printf("Comando no implementado\n");
        }
    }
}
