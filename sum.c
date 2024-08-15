#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int start;
    int end;
    int* array;
} boundary;

void* thread_sum(void* arg){
    boundary* bound = (boundary*) arg;
    for (int i = bound->start; i < bound->end; i++){
        bound->array[i]++;
    }
    free(arg);
    pthread_exit(NULL);
}

int* createArray(int n){
    int* arr = malloc(sizeof(int)*n);
    for (int i=0; i<n; i++){
        arr[i] = 100;
    }
    return arr;
}

void printArray(int* arr, int size){
    printf("[");
    for (int i=0; i<size; i++){
        printf("%d, ", arr[i]);
    }
    printf("]\n");
}

int main(int argc, char* argv[]){
    if(argc<3){
        printf("--ERRO: informe a qtde de threads e o tamanho do array como argumentos\n");
        return 1;
    }
    int nthreads = atoi(argv[1]);
    int arraysize = atoi(argv[2]);

    if(nthreads>arraysize){
        printf("--ERRO: a qtde de threads é maior do que o tamanho do array.\n");
        return 1;
    }

    pthread_t tid_sistema[nthreads];

    int start = 0;
    int end = 0;
    int* array = createArray(arraysize);

    int remainder = arraysize%nthreads;
    int partition_size = arraysize/nthreads; // divisão arredonda para baixo
    printf("remainder: %d\n", remainder);
    printf("partition_size: %d\n", partition_size);
    
    for (int i=0; i<nthreads; i++){
        boundary* args = malloc(sizeof(boundary));
        if (args == NULL){
            printf("--ERRO: malloc()\n");
            return 2;
        }
        args->array = array;
        args->start = end;
        args->end = args->start + partition_size;
        if (remainder > 0){
            args->end++;
            remainder--;
        }
        start = args->start;
        end = args->end;
        printf("args->start: %d\n", args->start);
        printf("args->end: %d\n", args->end);
        if (pthread_create(&tid_sistema[i], NULL, thread_sum, args)){ 
            printf("--ERRO: pthread_create() falhou\n");
            return 2;
        }
    }

    for (int i=0; i<nthreads; i++){
        pthread_join(tid_sistema[i], NULL);
    }

    //printArray(array, arraysize);
    pthread_exit(NULL);
    return 0;
}
