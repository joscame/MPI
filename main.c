#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "mpi.h"

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

//devuelve la posición con respecto a la matriz A, de la primera fila de un proceso
int get_first_row_pos (int rank, int rows_p_process){
    return rank * rows_p_process;
}

//devuelve la posición con respecto a la matriz A, de la ultima fila de un proceso
int get_last_row_pos (int rank, int rows_p_process){
    return ((rank+1) * rows_p_process) - 1;
}

int get_index_of_row(int desired_matrix_a_row, int first_matrix_a_row){
    int index = 0;
    while (first_matrix_a_row != desired_matrix_a_row){
        index++;
        first_matrix_a_row++;
    }
    return index;
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
    int         my_first_row_pos; /* posicion que tiene la primera fila de cada proceso en la matriz A   */
    int         my_last_row_pos; /* posicion que tiene la ultima fila de cada proceso en la matriz A   */
    int         tag = 0;       /* etiqueta para mensajes */
    int         **matrix_a;
    int         **matrix_b;
    int         **assigned_rows;
    int         *sum_of_cols;
    int         *elements_of_col0;
    int         *empty_row;
    char        user_confirmation;   /* confirmation answear */
    //MPI_Status  status;        /* return status para receptor  */

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
    //MPI_Init(&argc, &argv);

    /* Quién soy  */
    //MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Cuántos somos */
    //MPI_Comm_size(MPI_COMM_WORLD, &p);

    //Se inicializa la matriz de datos que va a tener cada proceso
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
            //MPI_Send(matrix_a[i], m, MPI_INT, process, tag, MPI_COMM_WORLD);
        }
    }else{ //Procesos que no son el 0 reciven las filas
        for (int i=0; i<rows_p_process; i++){
            //MPI_Recv(assigned_rows[i], m, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }
    }
        
    //Se inicializa el vector que contendra la suma de las columnas que le toca a cada proceso
    sum_of_cols = (int *)calloc(rows_p_process, sizeof(int));

    //Se inicializa el vector que contendra los elementos de la columna 0 que le toca a cada proceso
    elements_of_col0 = (int *)calloc(rows_p_process, sizeof(int));

    //Todos los procesos recorren sus datos una vez (Dentro de este ciclo aninado se deben hacer todos los cálculos)
    for (int i=0; i<rows_p_process; i++){
        for (int j=0; j<m; j++){
            if (j == 0) //(fila 3 de matriz B)gather de los elementos de la columna 0
                elements_of_col0[i] = assigned_rows[i][j];

            if (i == 0 && my_rank == 0) //(fila 2 de matriz B) el proceso 0 copia la fila 0 de la matriz A
                matrix_b[i][j] = matrix_a[i][j];

            //(fila 7 Matriz B)Se calcula la suma de columnas para cada una de las filas y se almacena en sum_of_cols
            sum_of_cols[i] += assigned_rows[i][j];
        }
    }

    //(fila 3 de matriz B)gather de los elementos de la columna 0
    //MPI_Gather( elements_of_col0, rows_p_process, MPI_INT, matrix_b[3], 3, MPI_INT, 0, MPI_COMM_WORLD);

    //(fila 7 Matriz B) se hace gather para recolectar las sumas de las columnas de todos los procesos
    //MPI_Gather( sum_of_cols, rows_p_process, MPI_INT, matrix_b[7], 3, MPI_INT, 0, MPI_COMM_WORLD);

    //(fila 5 Matriz B) los dos procesos correspondientes suman la fila 1 y m-2 usando reduce
    my_first_row_pos = get_first_row_pos(my_rank, rows_p_process);
    my_last_row_pos = get_last_row_pos(my_rank, rows_p_process);
    if ( my_first_row_pos <= 1 && my_last_row_pos >= 1){} //si el proceso tiene la fila 1
        //MPI_Reduce(assigned_rows[get_index_of_row(1, my_first_row_pos)], matrix_b[5], m, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    else if (my_first_row_pos <= m-2 && my_last_row_pos >= m-2){}
        //MPI_Reduce(assigned_rows[get_index_of_row(m-2, my_first_row_pos)], matrix_b[5], m, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


    /* Termina de usarse MPI */
    //MPI_Finalize();
}/* main */
