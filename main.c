#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

int get_random_number(upper_limit){
    return rand() % upper_limit + 1;
}

void print_matrix(int** matrix, int rows, int columns){

    for (int i=0; i<rows; i++){
        printf("\n");
        for (int j=0; j<columns; j++)
            printf("%d ", matrix[i][j]);
    }

    printf("\n");
}

main(int argc, char* argv[]) {
    int         my_rank;       /* rank del proceso     */
    int         p;             /* número de procesos   */
    int         m;             /* largo y ancho de la matriz A   */
    int         rows_p_process;
    int         a;             /* num aleatorio   */
    int         b;             /* num aleatorio   */
    int         c;             /* num aleatorio   */
    int         d;             /* num aleatorio   */
    int         tag = 0;       /* etiqueta para mensajes */
    int         **matrix_a;
    int         **matrix_b;
    int         **assigned_rows;
    char        user_confirmation;   /* confirmation answear */
    MPI_Status  status;        /* return status para receptor  */

    //Se setea la semilla para generar nums aleatorios
    srand(time(0));

    //calcula la cantidad de filas por proceso
    rows_p_process = m / p;


    //Pide los valores al usuario
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

    //Crea la matriz A
    matrix_a = (int **)malloc(m * sizeof(int *));
    for (int i=0; i<m; i++)
         matrix_a[i] = (int *)malloc(m * sizeof(int));

    //llena la matriz A
    for (int i=0; i<m; i++)
        for (int j=0; j<m; j++)
            matrix_a[i][j] = get_random_number(5);

    //imprime la matriz A
    print_matrix(matrix_a, m, m);

    //Crea la matriz B
    matrix_b = (int **)malloc(9 * sizeof(int *));
    for (int i=0; i<9; i++)
         matrix_b[i] = (int *)malloc(m * sizeof(int));

    //Genera nums aleatorios
    a = get_random_number(m-1);
    printf("\nEl valor de a es: %d\n", a);

    b = get_random_number(m-1);
    printf("\nEl valor de b es: %d\n", b);

    c = get_random_number(m-1);
    printf("\nEl valor de c es: %d\n", c);

    d = get_random_number(m-1);
    printf("\nEl valor de d es: %d\n", d);

    /* Inicializa MPI */
    MPI_Init(&argc, &argv);

    /* Quién soy  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Cuántos somos */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    //Se inicializa la matriz que va a tener cada proceso
    if (rows_p_process > 1)
        assigned_rows = (int **)malloc(rows_p_process * sizeof(int *));
    for (int i=0; i<rows_p_process; i++)
         assigned_rows[i] = (int *)malloc(m * sizeof(int));

    //Se envian los vectores a los procesos
    int sent_rows = 0;
    int process = 0;

    if (my_rank == 0){ //el proceso padre envía las filas a los otros procesos
        for (int i=0; i<m; i++){
            if (sent_rows == rows_p_process){
                sent_rows = 0;
                process++;
            }
            MPI_Send(matrix_a[i], m, MPI_INT, process, tag, MPI_COMM_WORLD);
        }
    }else{ //Procesos que no son el 0 reciven las filas
        for (int i=0; i<rows_p_process; i++){
            MPI_Recv(assigned_rows[i], m, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }
    }
        
    //Se rellena la fila 3 de la matriz B
    for (int i=0; i<rows_p_process; i++)
        MPI_Gather( assigned_rows[i][0], 1, MPI_INT, matrix_b[3], 1, MPI_INT, 0, MPI_COMM_WORLD);


    /* Termina de usarse MPI */
    MPI_Finalize();
}/* main */
