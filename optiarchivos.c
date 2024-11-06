/* 
 * Nombre del equipo: S.O. AGREVAL
 * Fecha: 06/11/2024
 * Versión: 1.1
 * Descripción:
 * crea un sistema de archivos virtual simple, permite realizar escrituras asíncronas para mejorar 
 * el rendimiento de E/S y evitar la fragmentación de archivos. La documentación se ha añadido en forma 
 * de comentarios para cada función y estructura
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define BLOCK_SIZE 4096
#define NUM_BLOCKS 1024
#define FILE_SYSTEM_SIZE (BLOCK_SIZE * NUM_BLOCKS)

typedef struct {
    int fd;
    char *file_name;
    char *file_data;
} File;

/**
 * Realiza una escritura síncrona en el archivo especificado.
 * @param file Puntero a la estructura File.
 * @param data Los datos a escribir.
 * @param size El tamaño de los datos a escribir.
 * @param offset El desplazamiento en el archivo donde se debe iniciar la escritura.
 */
void sync_write(File *file, const char *data, size_t size, off_t offset) {
    if (lseek(file->fd, offset, SEEK_SET) == -1) {
        perror("lseek");
        return;
    }

    ssize_t written = write(file->fd, data, size);
    if (written == -1) {
        perror("write");
    } else if ((size_t)written != size) {
        fprintf(stderr, "Partial write: expected %zu bytes, wrote %zd bytes\n", size, written);
    }
}

/**
 * Crea un sistema de archivos virtual.
 * @param file_name El nombre del archivo que actuará como sistema de archivos.
 */
void create_file_system(const char *file_name) {
    int fd = open(file_name, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, FILE_SYSTEM_SIZE) == -1) {
        perror("ftruncate");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}

/**
 * Crea un archivo en el sistema de archivos virtual.
 * @param file_system_name El nombre del sistema de archivos.
 * @param file_name El nombre del archivo a crear.
 * @return Puntero a la estructura File.
 */
File *create_file(const char *file_system_name, const char *file_name) {
    int fd = open(file_system_name, O_RDWR);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    File *file = malloc(sizeof(File));
    file->fd = fd;
    file->file_name = strdup(file_name);
    file->file_data = calloc(1, FILE_SYSTEM_SIZE);

    return file;
}

/**
 * Escribe datos en el archivo especificado.
 * @param file Puntero a la estructura File.
 * @param data Los datos a escribir.
 * @param size El tamaño de los datos a escribir.
 */
void write_file(File *file, const char *data, size_t size) {
    sync_write(file, data, size, 0);
}

/**
 * Cierra y libera los recursos del archivo especificado.
 * @param file Puntero a la estructura File.
 */
void close_file(File *file) {
    close(file->fd);
    free(file->file_name);
    free(file->file_data);
    free(file);
}

/**
 * Función principal.
 * Crea un sistema de archivos virtual, escribe en un archivo y lo cierra.
 */
int main() {
    create_file_system("virtual_fs.dat");

    File *file = create_file("virtual_fs.dat", "example.txt");
    if (!file) {
        fprintf(stderr, "Failed to create file\n");
        return EXIT_FAILURE;
    }

    const char *data = "Hello, World!";
    write_file(file, data, strlen(data));

    close_file(file);
    return 0;
}