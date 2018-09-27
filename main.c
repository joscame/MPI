#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "mpi.h"

int get_random_number(upper_limit){
    srand(time(NULL));
    return rand() % upper_limit + 1;
}

main(int argc, char* argv[]) {
    int         my_rank;       /* rank del proceso     */
    int         p;             /* número de procesos   */
    int         m;             /* largo y ancho de la matriz A   */
    int         a;             /* num aleatorio   */
    int         b;             /* num aleatorio   */
    int         c;             /* num aleatorio   */
    int         d;             /* num aleatorio   */
    int         source;        /* rank de quien envía  */
    int         dest;          /* rank del receptor     */
    int         tag = 0;       /* etiqueta para mensajes */
    char        message[100];  /* almacenamiemto del mensaje  */
    char        user_confirmation;   /* confirmation answear */
    //MPI_Status  status;        /* return status para receptor  */

    do {
        printf("Please type the number of processes that you want to use to run this program. The number must be even.\n");
        scanf("%d", &p);
        printf("Now provide a value for the dimensions of A, which is a square matrix. This value should be a multiple of the number of processes\n");
        scanf("%d", &m);
        printf("\n\nThis is the current configuration:\n\n");
        printf("%19s: %d\n", "Number of processes", p);
        printf("%19s: %d x %d\n\n\n", "Dimensions", m, m);
        printf("Do you want to modify these values? (Y/N)");
        fflush(stdin);
        scanf("%c", &user_confirmation);
    } while (user_confirmation == 'Y' || user_confirmation == 'y');

    /* Inicializa MPI */
    //MPI_Init(&argc, &argv);

    /* Quién soy  */
    //MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Cuántos somos */
    //MPI_Comm_size(MPI_COMM_WORLD, &p);

    //Genera nums aleatorios
    a = get_random_number(5);
    printf("\nEl valor de a es: %d\n", a);


    /* Termina de usarse MPI */
    //MPI_Finalize();
}/* main */
