#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define NTHREADS 2
#define N 100

char buffer1[N];
char buffer2[N*2]; 
sem_t sem_buffer1_empty;
sem_t sem_buffer1;
sem_t sem_buffer2_empty;
sem_t sem_buffer2;
char* filename;
int file_done=0;
int buffer2_end=0;
pthread_mutex_t mutex;

void* reader(void* arg){
	size_t current=0;
	size_t total = 0;
	FILE* f = fopen(filename, "r");
	while(1){
		sem_wait(&sem_buffer1_empty);
		current = fread(&buffer1, sizeof(char), N-1 ,f);
		if(current < N-1){
			buffer1[current+1] = '\0';
			pthread_mutex_lock(&mutex);
			file_done=1;
			pthread_mutex_unlock(&mutex);
			sem_post(&sem_buffer1);
			break;	
		}
		total += current;
		sem_post(&sem_buffer1);
	}
	fclose(f);	
    pthread_exit(NULL);
}

void* process(void* arg){	
	int count=0;
	int offset = 0;
	int char_count=0;
	int j=0; 
	while(1){
		sem_wait(&sem_buffer1);
		sem_wait(&sem_buffer2_empty);
		offset = 0;
		for(int i=0; i<N-1;i++){
			if(buffer1[i] == '\0'){
				buffer2[i+offset] = '\0';
				break;
			}
			if(count == 2*j+1 && j <= 10){
				buffer2[i+offset] = '\n';
				offset++;
				count=0;
                char_count++;
				j++;
			} else if(j > 10 && count % 10 == 0){
				buffer2[i+offset] = '\n';
				offset++;
				count=0;
                char_count++;
			}
			buffer2[i+offset] = buffer1[i];
            char_count++;
			count++;
		}
		buffer2[N+offset] = '\0';
		sem_post(&sem_buffer1_empty);
		sem_post(&sem_buffer2);
		pthread_mutex_lock(&mutex);
		if(file_done){
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_lock(&mutex);
	buffer2_end = 1;
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}


void* printer(void* arg){
	int current=0;
	while(1){
		sem_wait(&sem_buffer2);
		current += printf("%s", buffer2);
		sem_post(&sem_buffer2_empty);
		pthread_mutex_lock(&mutex);
		if(buffer2_end){
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[]){
	if(argc < 2){
		fprintf(stderr, "%s < filename > ", argv[0]);
		exit(1);
	}
	filename = argv[1];
 	pthread_t tid[NTHREADS]; 
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_buffer1_empty, 0, 1);
	sem_init(&sem_buffer2_empty, 0, 1);
	sem_init(&sem_buffer1, 0, 0);
	sem_init(&sem_buffer2, 0, 0);
	pthread_create(&tid[0], NULL, reader, NULL);
	pthread_create(&tid[1], NULL, process, NULL);
	pthread_create(&tid[2], NULL, printer, NULL);
	for(int i=0; i< NTHREADS; i++){
		if(pthread_join(tid[i], NULL)){
			fprintf(stderr, "Erro pthread join %d\n", i);
			exit(50);
		}
	}
	sem_destroy(&sem_buffer1);
	sem_destroy(&sem_buffer1_empty);
	sem_destroy(&sem_buffer2);
	sem_destroy(&sem_buffer2_empty);
	pthread_mutex_destroy(&mutex);
	return 0;
}
