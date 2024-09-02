#include <stdlib.h>
#include <stdio.h>

#define DEBUG

typedef struct {
	float* v;
	int rows;
	int cols;
	long long int size;
} mat;

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

mat* matProduct(mat* m1, mat* m2){
	mat* p = malloc(sizeof(mat));
	p->rows = m1->rows;
	p->cols = m2->cols;
	p->size = p->rows*p->cols;
	p->v = malloc(sizeof(float)*p->size);
	for(int col2 = 0; col2 < m2->cols; col2++){
		for(int row1 = 0; row1 < m1->rows; row1++){
			int m1_row_start = row1*m1->cols;
			float dotp = 0;
			for (int i=0; i < m2->rows; i++){
				dotp += m2->v[col2+(i*m2->cols)] * m1->v[m1_row_start+i];
			}
			p->v[col2+p->cols*row1] = dotp;
		}
	}
	return p;
}

void freeMatrix(mat* m){
	free(m->v);
	free(m);
}

int main(int argc, char* argv[]){
	mat* matrix1;
	mat* matrix2;

	if (argc < 4){
		fprintf(stderr, "Falta: %s <matriz 1> <matriz 2> <arquivo saida>\n", argv[0]);
		return 1;
	}

	matrix1 = malloc(sizeof(mat));
	matrix2 = malloc(sizeof(mat));
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

	mat* product = matProduct(matrix1, matrix2);
	#ifdef DEBUG
	printf("A\n");
	printMatrix(matrix1);
	printf("B\n");
	printMatrix(matrix2);
	printf("Produto\n");
	printMatrix(product);
	#endif
	writeMatrix(product, argv[3]);

	freeMatrix(matrix1);
	freeMatrix(matrix2);
	freeMatrix(product);

	return 0;
}
