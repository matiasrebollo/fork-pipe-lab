# Laboratorio de fork y pipe en C

Este trabajo práctico tiene como objetivo familiarizarse con las llamadas al sistema `fork(2)`, `pipe(2)`, `wait(2)` y `execvp(3)` implementando dos utilidades:

- **`primes`**: Criba de Eratóstenes con procesos encadenados mediante pipes.
- **`xargs`**: Versión reducida de la utilidad `xargs`, que ejecuta un comando agrupando argumentos leídos por línea.

Trabajo práctico para la materia **Sistemas Operativos** (FIUBA).

## Compilar

```bash
make
```

## Pruebas

```bash
make test
```

## Linter

```bash
make format
```

## Programas incluidos

### `primes`

Implementa la **criba de Eratóstenes** con una arquitectura de procesos.  
Cada filtro es un proceso conectado al siguiente mediante un pipe.

#### Uso

```bash
./primes <n>
```

### Ejemplo

```bash
./primes 35
```
Imprime todos los primos menores o iguales a n.

### `xargs`

Reimplementación básica de la utilidad `xargs`, agrupando entradas de `stdin` de a `NARGS` y ejecutando un comando por grupo.

#### Uso

```bash
./xargs <comando>
```

#### Ejemplo

```bash
seq 10 | ./xargs /bin/echo
```
Ejecuta /bin/echo varias veces con grupos de 4 números (valor de NARGS).
