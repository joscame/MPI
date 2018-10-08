#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "mpi.h"

int get_random_number(int upper_limit){
    return rand() % upper_limit + 1;
}

void print_matrix(int** matrix, int rows, int columns){

    for (int i=0; i<rows; i++){
        printf("\n");
        for (int j=0; j<columns; j++)
            printf("%2d ", matrix[i][j]);
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

//devuelve el proceso que tiene la fila row de la matriz A
int get_process_of_row(int row, int rows_p_process, int m){
    int process = 0;
    for (int i=1; i<=m; i++){
        if (i-1 == row)
            return process;
        if (i % rows_p_process == 0)
            process++;
    }
}

//devuelve el index en el que se encuentra la fila deseada de la matriz A en la matriz de cada proceso
int get_index_of_row(int desired_matrix_a_row, int first_matrix_a_row){
    int index = 0;
    while (first_matrix_a_row != desired_matrix_a_row){
        index++;
        first_matrix_a_row++;
    }
    return index;
}

//Devuelve true si el proceso tiene la fila especificada
bool do_i_have_row(int my_first_row_pos, int my_last_row_pos, int row){
    if (my_first_row_pos <= row && my_last_row_pos >= row)
        return true;
    return false;
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
    int         index_of_row_m_2;
    int         tag = 0;       /* etiqueta para mensajes */
    int         **matrix_a;
    int         **matrix_b;
    int         **assigned_rows;
    int         *sum_of_cols;
    int         *sum_of_2_cols;
    int         *elements_of_col0;
    bool        have_row_1;
    bool        have_row_m_2;
    MPI_Status  status;        /* return status para receptor  */

    /* Inicializa MPI */
    MPI_Init(&argc, &argv);

    /* Quién soy  */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Cuántos somos */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    //Se setea la semilla para generar nums aleatorios
    srand(time(0));

    //se asigna el valor de m
    m = atoi(argv[1]);

    //Genera nums aleatorios
    a = get_random_number(m-1);
    b = get_random_number(m-1);
    c = get_random_number(m-1);
    d = get_random_number(m-1);

    //Crea la matriz B
    matrix_b = (int **)malloc(9 * sizeof(int *));
    for (int i=0; i<9; i++)
         matrix_b[i] = (int *)calloc(m, sizeof(int));

    if(my_rank == 0){
        printf("%19s: %d x %d\n\n\n", "Dimensions", m, m);

        printf("\nEl valor de a es: %d\n", a);
        printf("\nEl valor de b es: %d\n", b);
        printf("\nEl valor de c es: %d\n", c);
        printf("\nEl valor de d es: %d\n", d);

        //Crea la matriz A
        matrix_a = (int **)malloc(m * sizeof(int *));
        for (int i=0; i<m; i++)
             matrix_a[i] = (int *)malloc(m * sizeof(int));

        //llena la matriz A
        for (int i=0; i<m; i++)
            for (int j=0; j<m; j++)
                matrix_a[i][j] = get_random_number(5);

        //imprime la matriz A
        printf("matriz A:\n\n");
        print_matrix(matrix_a, m, m);
    }

    //calcula la cantidad de filas por proceso
    rows_p_process = m / p;
    //printf("rows_p_process: %d\n", rows_p_process);

    //Se inicializa la matriz de datos que va a tener cada proceso
    assigned_rows = (int **)malloc(rows_p_process * sizeof(int *));
    for (int i=0; i<rows_p_process; i++)
         assigned_rows[i] = (int *)malloc(m * sizeof(int));
    //printf("%d inicializo matriz de datos\n", my_rank);

    //Se envian los vectores a los procesos
    if (my_rank == 0){ //el proceso padre envía las filas a los otros procesos
        for (int i=0; i<rows_p_process; i++)
            for (int j=0; j<m; j++)
                assigned_rows[i][j] = matrix_a[i][j];
        //printf("el proceso 0 llena su matriz de datos\n");
        //print_matrix(assigned_rows, rows_p_process, m);

        int sent_rows = 0;
        int process = 1;

        for (int i=rows_p_process; i<m; i++){
            if (sent_rows == rows_p_process){
                sent_rows = 0;
                process++;
            }
            MPI_Send(matrix_a[i], m, MPI_INT, process, tag, MPI_COMM_WORLD);
            sent_rows++;
            //printf("Se envia fila %d al proceso %d\n", sent_rows, process);
        }
    }else{ //Procesos que no son el 0 reciven las filas
        for (int i=0; i<rows_p_process; i++){
            MPI_Recv(assigned_rows[i], m, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        }

        //printf("matriz del proceso %d:\n", my_rank);
        //print_matrix(assigned_rows, rows_p_process, m);
    }


    //Se inicializa el vector que contendra la suma de las columnas que le toca a cada proceso
    sum_of_cols = (int *)calloc(rows_p_process, sizeof(int));

    //Se inicializa el vector que contendra los elementos de la columna 0 que le toca a cada proceso
    elements_of_col0 = (int *)calloc(rows_p_process, sizeof(int));

    //(fila 5 Matriz B) El proceso que tiene la fila 1 se la envia al proceso que tiene la fila m-2
    my_first_row_pos = get_first_row_pos(my_rank, rows_p_process);
    my_last_row_pos = get_last_row_pos(my_rank, rows_p_process);
    have_row_1 = do_i_have_row(my_first_row_pos, my_last_row_pos, 1);
    have_row_m_2 = do_i_have_row(my_first_row_pos, my_last_row_pos, m-2);
    if (have_row_1){//si el proceso tiene la fila 1
        MPI_Send(assigned_rows[get_index_of_row(1, my_first_row_pos)], m, MPI_INT, get_process_of_row(m-2, rows_p_process, m), tag, MPI_COMM_WORLD);
    }
    else if (have_row_m_2){//si el proceso tiene la fila m-2
        sum_of_2_cols = (int *)calloc(m, sizeof(int));
        index_of_row_m_2 = get_index_of_row(m-2, my_first_row_pos);
        MPI_Recv(sum_of_2_cols, m, MPI_INT, get_process_of_row(1, rows_p_process, m), tag, MPI_COMM_WORLD, &status);
    }

    //Todos los procesos recorren sus datos una vez (Dentro de este ciclo aninado se deben hacer todos los cálculos)
    for (int i=0; i<rows_p_process; i++){
        for (int j=0; j<m; j++){
            if (j == 0) //(fila 3 de matriz B)gather de los elementos de la columna 0
                elements_of_col0[i] = assigned_rows[i][j];

            if (i == 0 && my_rank == 0) //(fila 2 de matriz B) el proceso 0 copia la fila 0 de la matriz A
                matrix_b[2][j] = matrix_a[i][j];

            //(fila 7 Matriz B)Se calcula la suma de columnas para cada una de las filas y se almacena en sum_of_cols
            sum_of_cols[i] = sum_of_cols[i] + assigned_rows[i][j];

            //(fila 5 Matriz B) Si el proceso tiene la matriz m-2 la suma con la 1
            if (have_row_m_2 && i == index_of_row_m_2)
                    sum_of_2_cols[j] += assigned_rows[i][j];
        }
    }

    //(fila 3 de matriz B)gather de los elementos de la columna 0
    MPI_Gather( elements_of_col0, rows_p_process, MPI_INT, matrix_b[3], rows_p_process, MPI_INT, 0, MPI_COMM_WORLD);

    //(fila 7 Matriz B) se hace gather para recolectar las sumas de las columnas de todos los procesos
    MPI_Gather( sum_of_cols, rows_p_process, MPI_INT, matrix_b[7], rows_p_process, MPI_INT, 0, MPI_COMM_WORLD);

    //(fila 5 Matriz B) Si el proceso tiene la fila m-2 envia la suma de las dos filas al proceso 0
    if (have_row_m_2)
        MPI_Send(sum_of_2_cols, m, MPI_INT, 0, tag, MPI_COMM_WORLD);
    else if(my_rank == 0)
        MPI_Recv(matrix_b[5], m, MPI_INT, get_process_of_row(m-2, rows_p_process, m), tag, MPI_COMM_WORLD, &status);

    if (my_rank == 0){
        printf("Matriz B:\n\n");
        print_matrix(matrix_b, 9, m);
    }

    /* Termina de usarse MPI */
    MPI_Finalize();
}/* main */
