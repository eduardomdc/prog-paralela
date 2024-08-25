#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

float* vec1;
float* vec2;
double dotp = 0;

typedef struct {
    int start;
    int end;
} boundary;

void* thread_dot(void* arg){
    boundary* bound = (boundary*) arg;
    double* dot = malloc(sizeof(double));
    //printf("dot product from [%d] to [%d]\n", bound->start, bound->end);
    for (int i = bound->start; i < bound->end; i++){
        *dot += vec1[i]*vec2[i]; 
    }
    free(arg);
    pthread_exit((void*) dot);
}

int main(int argc, char* argv[]) {
    long int nthreads;
    FILE* file;
    if(argc < 3) {
        fprintf(stderr, "Digite: %s <num. de threads> <arquivo de entrada>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);
    file = fopen(argv[2], "rb");
    if (!file){
        fprintf(stderr, "Erro na abertura de arquivo\n");
        return 3;
    }
    // lendo arquivo gerado
    long int vec_size;
    double correct_dotp;
    fread(&vec_size, sizeof(long int), 1, file);
    vec1 = malloc(sizeof(float)*vec_size);
    vec2 = malloc(sizeof(float)*vec_size);
    fread(vec1, sizeof(float), vec_size, file);
    fread(vec2, sizeof(float), vec_size, file);
    fread(&correct_dotp, sizeof(double), 1, file);

    //repartir tarefas e chamar threads
    pthread_t tid[nthreads];
    int start = 0;
    int end = 0;
    int remainder = vec_size%nthreads;
    int partition_size = vec_size/nthreads;
    for (int i = 0; i < nthreads; i++){
        boundary* args = malloc(sizeof(boundary));
        args->start = end;
        args->end = args->start + partition_size;
        if (remainder>0){
            args->end++;
            remainder--;
        }
        start = args->start;
        end = args->end;
        if (pthread_create(&tid[i], NULL, thread_dot, args)){
            fprintf(stderr, "Erro: pthread_create()\n");
            return 2;
        }
    }

    // espera por todas threads
    for (int i = 0; i < nthreads; i++){
        double* dot_thread;
        pthread_join(tid[i], (void*) &dot_thread);
        dotp += *dot_thread;
        free(dot_thread);
    }
    
    double var = fabs((dotp-correct_dotp)/correct_dotp);

    printf("Valor pré-computado: %f\n", dotp);
    printf("Valor computado concorrentemente: %f\n", correct_dotp);
    printf("Variação relativa: %f\n", var);
    return 0;
}
