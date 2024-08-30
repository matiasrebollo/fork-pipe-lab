#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>


void recursion(int fds_lectura){
	int p;
	// Si hay numeros en la secuencia
	if (read(fds_lectura, &p, sizeof(int)) > 0){
		printf("primo %d\n", p); // Si hay numero, lo imprimimos ya que es primo

		
		pid_t pid;
		int fds[2];

		// Creamos otro pipe para poder conectar este proceso con el nuevo ("hermano" derecho)
		int r = pipe(fds);
		if (r < 0){
			printf("Error al crear el pipe\n");
			exit(EXIT_FAILURE);
		}

		// Creamos un proceso nuevo ("hermano" derecho)
		pid = fork();
		if (pid < 0){
			printf("Error en el fork %d\n", pid);
			exit(EXIT_FAILURE);
		}

		// En el proceso padre ("hermano" izquierdo) mientras hayan numeros en la secuencia,
		// y no sean multiplos del numero que printeamos, los escribimos en el descriptor de escritura.
		if (pid > 0){

			close(fds[0]); // Cerramos descriptor de lectura

			int n;
			while (read(fds_lectura, &n, sizeof(int)) > 0){
				if (n % p != 0){ // Filtramos multiplos
					write(fds[1], &n, sizeof(int));
				}
			}
			close(fds[1]); // Cerramos descriptor de escritura
			close(fds_lectura);
			wait(NULL); // Esperamos que nuestro hijo ("hermano" derecho) termine

		// En el proceso hijo ("hermano" derecho), vamos a repetir lo que hicimos	
		} else {

			close(fds[1]); // Cerramos descriptor de escritura
			close(fds_lectura);
			recursion(fds[0]); // Llamamos a la recursion para seguir leyendo del pipe
			exit(0); // Termina el proceso hijo

		}
	// Si ya la secuencia esta vacia
	} else {
		close(fds_lectura); // Cerramos descriptor de lectura
	}
}

int
main(int argc, char *argv[])
{

	// Chequeamos que el usuario ingrese solo 1 cosa
	if (argc != 2) {
		printf("Por favor usar bien la interfaz\n");
		exit(EXIT_FAILURE);
	}
	// La transformamos a entero
	int numero = atoi(argv[1]);


	pid_t pid;
	int fds[2];

	// Creamos el pipe por el que se conectaran padre e hijo
	int r = pipe(fds);
	if (r < 0){
		printf("Error al crear el pipe\n");
		exit(EXIT_FAILURE);
	}

	// Creamos el nuevo proceso (hijo)
	pid = fork();
	if (pid < 0){
		printf("Error en el fork %d\n", pid);
		exit(EXIT_FAILURE);
	}

	// En el proceso padre escribimos todos los numeros de 2 a N
	if (pid > 0){ 
		
		close(fds[0]); // Cerramos descriptor de lectura
		for(int i = 2; i <= numero; i++){
			write(fds[1], &i, sizeof(int));
		}
		close(fds[1]); // Una vez escrito, cerramos el descriptor de escritura
		wait(NULL); // Esperamos que nuestro hijo termine

	// En el proceso hijo vamos a de forma "recursiva", filtrar los numeros multiplos del primer valor que se recibe.
	} else {

		close(fds[1]); // Cerramos descriptor de escritura
		recursion(fds[0]); // Llamamos a la funcion recursiva
		exit(0); // Termina el proceso hijo

	}
	return 0;
}