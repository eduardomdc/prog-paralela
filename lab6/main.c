#include <stdio.h>
#include <stdlib.h>
#include "list_int.h"
#include <pthread.h>
#include "timer.h"

#define QTDE_OPS 10000000 //quantidade de operacoes sobre a lista (insercao, remocao, consulta)
#define QTDE_INI 100 //quantidade de insercoes iniciais na lista
#define MAX_VALUE 100 //valor maximo a ser inserido

//lista compartilhada iniciada 
struct list_node_s* head_p = NULL; 
//qtde de threads no programa
int nthreads;
pthread_mutex_t lock;
pthread_cond_t readers_ok;
pthread_cond_t writers_ok;
int writers_waiting = 0;
int writers_active = 0;
int readers = 0;

void read_lock() {
    pthread_mutex_lock(&lock);
    while (writers_active || writers_waiting) {
        pthread_cond_wait(&readers_ok, &lock);
    }
    readers++;
    pthread_mutex_unlock(&lock);
}

void read_unlock() {
    pthread_mutex_lock(&lock);
    readers--;
    if (readers == 0) {
        pthread_cond_signal(&writers_ok);
    }
    pthread_mutex_unlock(&lock);
}

void write_lock() {
    pthread_mutex_lock(&lock);
    writers_waiting++;
    while (readers || writers_active) {
        pthread_cond_wait(&writers_ok, &lock);
    }
    writers_waiting--;
    writers_active = 1;
    pthread_mutex_unlock(&lock);
}

void write_unlock() {
    pthread_mutex_lock(&lock);
    writers_active = 0;
    pthread_cond_broadcast(&readers_ok); // Sinaliza leitores
    pthread_cond_signal(&writers_ok); // Sinaliza próximo escritor
    pthread_mutex_unlock(&lock);
}

//tarefa das threads
void* tarefa(void* arg) {
    long int id = (long int) arg;
    int op;
    int in, out, read;
    in=out=read = 0;

    //realiza operacoes de consulta (98%), insercao (1%) e remocao (1%)
    for(long int i=id; i<QTDE_OPS; i+=nthreads) {
        op = rand() % 100;
        if(op<98) {
            printf("id %ld: quero ler\n", id);
            read_lock(&lock);
            printf("id %ld: to lendo\n", id);
            Member(i%MAX_VALUE, head_p);   /* Ignore return value */
            printf("id %ld: li!\n", id);
            read_unlock(&lock);
            read++;
        } else if(98<=op && op<99) {
            printf("id %ld: quero escrever\n", id);
            write_lock(&lock);
            printf("id %ld: to escrevendo\n", id);
            Insert(i%MAX_VALUE, &head_p);  /* Ignore return value */
            printf("id %ld: escrevi!\n", id);
            write_unlock(&lock);
            in++;
        } else if(op>=99) {
            printf("id %ld: quero deletar\n", id);
            write_lock(&lock);
            printf("id %ld: to deletando\n", id);
            Delete(i%MAX_VALUE, &head_p);  /* Ignore return value */
            printf("id %ld: deletei!\n", id);
            write_unlock(&lock);
            out++;
        }
    }
    //registra a qtde de operacoes realizadas por tipo
    printf("Thread %ld: in=%d out=%d read=%d\n", id, in, out, read);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    pthread_t *tid;
    double ini, fim, delta;

    //verifica se o numero de threads foi passado na linha de comando
    if(argc<2) {
        printf("Digite: %s <numero de threads>\n", argv[0]); return 1;
    }
    nthreads = atoi(argv[1]);

    //insere os primeiros elementos na lista
    for(int i=0; i<QTDE_INI; i++)
        Insert(i%MAX_VALUE, &head_p);  /* Ignore return value */


    //aloca espaco de memoria para o vetor de identificadores de threads no sistema
    tid = malloc(sizeof(pthread_t)* nthreads);
    
    if(tid==NULL) {  
        printf("--ERRO: malloc()\n"); return 2;
    }

    //tomada de tempo inicial
    GET_TIME(ini);
    //inicializa a variavel mutex
    //pthread_rwlock_init(&rwlock, NULL);

    //cria as threads
    for(long int i=0; i<nthreads; i++) {
        if(pthread_create(tid+i, NULL, tarefa, (void*) i)) {
            printf("--ERRO: pthread_create()\n"); return 3;
        }
    }

    //aguarda as threads terminarem
    for(int i=0; i<nthreads; i++) {
        if(pthread_join(*(tid+i), NULL)) {
            printf("--ERRO: pthread_join()\n"); return 4;
        }
    }

    //tomada de tempo final
    GET_TIME(fim);
    delta = fim-ini;
    printf("Tempo: %lf\n", delta);

    //libera o mutex
    // pthread_rwlock_destroy(&rwlock);
    //libera o espaco de memoria do vetor de threads
    free(tid);
    //libera o espaco de memoria da lista
    Free_list(&head_p);

    return 0;
}  /* main */