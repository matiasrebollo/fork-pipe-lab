#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define READ 0
#define WRITE 1

// Se encarga de crear un pipe y devuelve false en caso de error.
bool
crear_pipe(int fds[2])
{
	int r = pipe(fds);
	if (r < 0) {
		perror("Error al crear el pipe");
		return false;
	}
	return true;
}

// Se encarga de crear un proceso y devolver su PID, y en caso de error cierra ambos FDs.
pid_t
crear_proceso(int fds_lectura, int fds_escritura)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("Error en el fork");
		close(fds_lectura);
		close(fds_escritura);
	}
	return pid;
}

// Escribe y devuelve los bytes escritos, y en caso de error cierra el FD de escritura.
ssize_t
escribir(int fds_escritura, void *dir, size_t size)
{
	ssize_t cantidad = write(fds_escritura, dir, size);
	if (cantidad < 0) {
		perror("Error en la escritura");
		close(fds_escritura);
	}
	return cantidad;
}

// Lee y devuelve los bytes escritos, y en caso de error cierra el FD de lectura.
ssize_t
leer(int fds_lectura, void *dir, size_t size)
{
	ssize_t cantidad = read(fds_lectura, dir, size);
	if (cantidad < 0) {
		perror("Error en la lectura");
		close(fds_lectura);
	}
	return cantidad;
}

// Se encarga de recursivamente, hasta llegar al final, imprimir el primer numero
// 'p' de la secuencia y crear un nuevo proceso que continua el filtrado con los numeros que no son multiplos de 'p'.
void
recursion(int fds_lectura)
{
	int p;

	// Manejamos errores del read.
	int cantidad = leer(fds_lectura, &p, sizeof(int));
	if (cantidad < 0) {
		exit(EXIT_FAILURE);
	}

	// Si hay numeros en la secuencia:
	if (cantidad > 0) {
		printf("primo %d\n",
		       p);  // Si hay numero, lo imprimimos ya que es primo.
		fflush(stdout);

		// Creamos otro pipe para poder conectar este proceso con el nuevo ("hermano" derecho).
		int fds[2];
		if (!crear_pipe(fds)) {
			close(fds_lectura);  // En caso de error, cerramos el FD
			                     // que recibe la funcion por parametro
			                     // con los nros que nos envio el hijo izq.
			exit(EXIT_FAILURE);
		}

		// Creamos un proceso nuevo ("hermano" derecho)
		pid_t pid;
		pid = crear_proceso(fds[READ], fds[WRITE]);
		if (pid < 0) {
			close(fds_lectura);  // En caso de error, cerramos el FD
			                     // que recibe la funcion por parametro
			                     // con los nros que nos envio el hijo izq.
			exit(EXIT_FAILURE);
		}

		// En el proceso padre ("hermano" izquierdo) mientras hayan
		// numeros en la secuencia, y no sean multiplos del numero que
		// printeamos, los escribimos en el FD de escritura.
		if (pid > 0) {
			close(fds[READ]);  // Cerramos FD de lectura ya que el padre no lo usa.

			int n;
			while ((cantidad = leer(fds_lectura, &n, sizeof(int))) >
			       0) {
				if (n % p != 0) {
					if (escribir(fds[WRITE], &n, sizeof(int)) <
					    0) {
						close(fds_lectura);  // En caso
						                     // de error,
						                     // cerramos
						                     // el FD que
						                     // recibe
						                     // la funcion
						                     // por parametro
						                     // con los nros que nos envio el hijo izq.
						exit(EXIT_FAILURE);
					}
				}
			}
			if (cantidad < 0) {
				close(fds[WRITE]);  // En caso de error, cerramos
				                    // el FD que recibe la
				                    // funcion por parametro con los nros que nos envio el hijo izq.
				exit(EXIT_FAILURE);
			}
			close(fds[WRITE]);  // Una vez usado, cerramos FD de escritura.
			close(fds_lectura);  // Una vez usado, cerramos FD que recibe
			                     // la funcion por parametro con los nros que nos envio el hijo izq.
			wait(NULL);

			// En el proceso hijo ("hermano" derecho), llamamos
			// recursivamente para continuar el filtrado.
		} else {
			close(fds[WRITE]);  // Cerramos FD de escritura, ya que el hijo no lo usa.
			close(fds_lectura);  // Cerramos FD que recibe la funcion
			                     // por parametro con los nros que
			                     // nos envio el hijo izq, ya que el hijo no lo usa.
			recursion(fds[READ]);
		}

		// Si ya la secuencia esta vacia:
	} else {
		close(fds_lectura);  // Cerramos FD que recibe la funcion por
		                     // parametro con los nros que nos envio
		                     // el hijo izq, ya que llegamos al final.
	}
}

int
main(int argc, char *argv[])
{
	// Chequeamos que el usuario ingrese solo 1 numero.
	if (argc != 2) {
		perror("Por favor usar bien la interfaz");
		exit(EXIT_FAILURE);
	}
	int numero = atoi(argv[1]);

	// Creamos el pipe por el que se conectaran padre e hijo, manejando errores.
	int fds[2];
	if (!crear_pipe(fds)) {
		exit(EXIT_FAILURE);
	}

	// Creamos el nuevo proceso (hijo), manejando errores.
	pid_t pid;
	pid = crear_proceso(fds[READ], fds[WRITE]);
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	// En el proceso padre escribimos todos los numeros de 2 a N
	if (pid > 0) {
		close(fds[READ]);  // Cerramos FD de lectura, ya que el padre no lo usa.
		for (int i = 2; i <= numero; i++) {
			if (escribir(fds[WRITE], &i, sizeof(int)) < 0) {
				exit(EXIT_FAILURE);
			}
		}
		close(fds[WRITE]);  // Una vez usado, cerramos FD de escritura.
		wait(NULL);

		// En el proceso hijo vamos a de forma "recursiva", filtrar los
		// numeros multiplos del primer valor que se recibe.
	} else {
		close(fds[WRITE]);  // Cerramos FD de escritura, ya que el hijo no lo usa.
		recursion(fds[READ]);
	}

	exit(0);
}