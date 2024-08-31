#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef NARGS
#define NARGS 4
#endif


// Se encarga de crear un proceso y devolver su PID, manejando posibles errores.
pid_t
crear_proceso()
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("Error en el fork");
		exit(EXIT_FAILURE);
	}
	return pid;
}

// Se encarga de ejecutar el comando especificado en el primer argumento del buffer con los argumentos del buffer.
void
ejecutar_comando(char *buffer[], int contador)
{
	// Creamos el nuevo proceso (hijo).
	pid_t pid;
	pid = crear_proceso();

	// El padre espera que termine el hijo.
	if (pid > 0) {
		wait(NULL);
		// El hijo se encarga de ejecutar el comando usando execvp,
		// teniendo en cuenta cualquier error posible.
	} else {
		buffer[contador] =
		        NULL;  // Agregamos el NULL para que execvp
		               // lo reconozca como el final de los argumentos.
		execvp(buffer[0], buffer);
		perror("Error en el execvp.");
		exit(EXIT_FAILURE);
	}
}

// Se encarga de almacenar la linea leida en el buffer.
void
almacenar_linea(char *line, char *buffer[], int *contador)
{
	line[strcspn(line, "\n")] = '\0';  // Eliminamos el caracter \n.
	buffer[*contador] = strdup(line);
	(*contador)++;  // Aumentamos el contador para almacenar otra linea en la siguiente posicion.
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Por favor usar bien la interfaz");
		exit(EXIT_FAILURE);
	}

	char *buffer[NARGS + 2];  // Buffer que almacena NARGS (cantidad de argumentos
	                          // que se leeran) + 2 (para el comando y para un NULL al final)
	buffer[0] = argv[1];  // El primer elemento es el comando.

	char *line = NULL;
	size_t len = 0;
	int contador =
	        1;  // Lleva la cuenta del numero de argumentos en el buffer.

	// Mientras haya una linea que leer, la leemos y la almacenamos
	while (getline(&line, &len, stdin) != -1) {
		almacenar_linea(line, buffer, &contador);

		// Si ya leimos NARGS argumentos, ejecutamos el comando.
		if (contador == NARGS + 1) {
			ejecutar_comando(buffer, contador);
			contador =
			        1;  // Reiniciamos el contador para seguir leyendo argumentos.
		}
	}

	// Si llegamos al EOF pero quedaron lineas restantes, ejecutamos el comando
	if (contador > 1) {
		ejecutar_comando(buffer, contador);
	}

	free(line);
	return 0;
}
