#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "timer.h"

//#define DEBUG

typedef struct {
	float* v;
	int rows;
	int cols;
	long long int size;
} mat;

typedef struct {
    int start;
    int end;
} bound;

mat* product;
mat* matrix1;
mat* matrix2;

int readMatrix(mat* matrix, const char* path){
	FILE* file = fopen(path, "rb");
	if (!file){
		fprintf(stderr, "Erro de abertura de arquivo\n");
		return 2;
	}
	size_t ret;
	ret = fread(&(matrix->rows), sizeof(int), 1, file);
	if (!ret) {
		fprintf(stderr, "Erro de leitura das dimensoes da matriz\n");
		return 3;
	}
	ret = fread(&(matrix->cols), sizeof(int), 1, file);
	if (!ret) {
		fprintf(stderr, "Erro de leitura das dimensoes da matriz\n");
		return 3;
	}

	matrix->size = matrix->rows*matrix->cols;

	matrix->v = (float*) malloc(sizeof(float)*matrix->size);

	if (!matrix->v){
		fprintf(stderr, "Erro de alocação de memória para a matriz\n");
		return 3;
	}

	ret = fread(matrix->v, sizeof(float), matrix->size, file);
	if (ret < matrix->size) {
		fprintf(stderr, "Erro de leitura dos elementos da matriz\n");
	}
	return 0;
}

int writeMatrix(mat* m, char* filename){
	//escreve a matriz no arquivo
	//abre o arquivo para escrita binaria
	int ret;
	FILE* fd = fopen(filename, "wb");
	if(!fd) {
		fprintf(stderr, "Erro de abertura do arquivo\n");
		return 3;
	}
	//escreve numero de linhas e de colunas
	ret = fwrite(&m->rows, sizeof(int), 1, fd);
	ret = fwrite(&m->cols, sizeof(int), 1, fd);
	//escreve os elementos da matriz
	ret = fwrite(m->v, sizeof(float), m->size, fd);
	if(ret < m->size) {
		fprintf(stderr, "Erro de escrita no  arquivo\n");
		return 4;
	}
	//finaliza o uso das variaveis
	fclose(fd);
    return 0;
}

void printMatrix(mat* matrix){
	printf("{");
	for (int i = 0; i < matrix->rows; i++){
		printf("{");
		for (int j = 0; j < matrix->cols; j++){
			printf("%f", matrix->v[i*matrix->cols+j]);
			if (j<matrix->cols-1) printf(", ");
		}
		printf("}");
		if (i<matrix->rows-1) printf(", ");
	}
	printf("}");
	printf("\n");
}

void freeMatrix(mat* m){
	free(m->v);
	free(m);
}

void* thread_mult(void* args){
    // obs: cache miss pode ser reduzido se utilizarmos
    // uma versão transposta da matriz2 para que os elementos
    // da coluna sejam sequenciais na memória.
    bound* limit = (bound*)args;
	for(int col2 = limit->start; col2 < limit->end; col2++){
		for(int row1 = 0; row1 < matrix1->rows; row1++){
			int m1_row_start = row1*matrix1->cols;
			float dotp = 0;
			for (int i=0; i < matrix2->rows; i++){
				dotp += matrix2->v[col2+(i*matrix2->cols)] * matrix1->v[m1_row_start+i];
			}
			product->v[col2+product->cols*row1] = dotp;
		}
	}
    free(limit);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){

	if (argc < 5){
		fprintf(stderr, "Falta: %s <matriz 1> <matriz 2> <arquivo saida> <n de threads>\n", argv[0]);
		return 1;
	}
    int nthreads = atoi(argv[4]);

	matrix1 = malloc(sizeof(mat));
	matrix2 = malloc(sizeof(mat));
    product = malloc(sizeof(mat));
	int error;	
	error=readMatrix(matrix1, argv[1]);
	if (error){
		return error;
	}
	error=readMatrix(matrix2, argv[2]);
	if (error){
		return error;
	}
	
	if (matrix1->cols != matrix2->rows){
		fprintf(stderr, "A matriz 1 deve possuir a quantidade de colunas igual a quantidade de linhas da matriz 2 para que exista produto.\n");
		return 1;
	}
    product->rows = matrix1->rows;
    product->cols = matrix2->cols;
    product->size = product->rows*product->cols;
    product->v = malloc(sizeof(float)*product->size);

    //repartir tarefas e chamar threads
    pthread_t tid[nthreads];
    int start = 0;
    int end = 0;
    int remainder = matrix2->cols%nthreads;
    int partition_size = matrix2->cols/nthreads;

    // cronometrar
    double begin, finish, elapsed;
    GET_TIME(begin);
    for (int i = 0; i < nthreads; i++){
        bound* args = malloc(sizeof(bound));
        args->start = end;
        args->end = args->start + partition_size;
        if (remainder>0){
            args->end++;
            remainder--;
        }
        start = args->start;
        end = args->end;
        if (pthread_create(&tid[i], NULL, thread_mult, args)){
            fprintf(stderr, "Erro: pthread_create()\n");
            return 2;
        }
    }

    // espera por todas threads
    for (int i = 0; i < nthreads; i++){
        pthread_join(tid[i], NULL);
    }
    GET_TIME(finish);
    elapsed = finish - begin;
	
    #ifdef DEBUG
	printf("A\n");
	printMatrix(matrix1);
	printf("B\n");
	printMatrix(matrix2);
	printf("Produto\n");
    printMatrix(product);
	#endif
	writeMatrix(product, argv[3]);

    // escreva resultados para formato .csv
    // threads, dimensão, tempo
    printf("%d, %d, %f\n", nthreads, product->rows, elapsed);

	freeMatrix(matrix1);
	freeMatrix(matrix2);
	freeMatrix(product);

	return 0;
}
