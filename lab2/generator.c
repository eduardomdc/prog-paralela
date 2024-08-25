#include <stdlib.h>
#include <stdio.h>
#include <time.h>

float rand_float(){
    // retorna float aleatorio em [-1,1]
    return 2*((float)random()/RAND_MAX)-1;
}

float* create_vector(long int n){
    float* vec = malloc(n * sizeof(float));
    for (int i=0; i < n; i++){
        vec[i] = rand_float();
    }
    return vec;
}

double dot_prod(float* vec1, float* vec2, long int n){
    double dotp = 0;
    for (int i=0; i < n; i++){
        dotp += vec1[i]*vec2[i];
    }
    return dotp;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    long int n;
    FILE* file;
    if(argc < 3) {
        fprintf(stderr, "Digite: %s <dimensao> <arquivo saida>\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);
    float* vec1 = create_vector(n);
    float* vec2 = create_vector(n);
    double dotp = dot_prod(vec1, vec2, n);
    file = fopen(argv[2], "wb");
    if (!file){
        fprintf(stderr, "Erro na abertura de arquivo\n");
        return 3;
    }
    fwrite(&n, sizeof(long int), 1, file);
    fwrite(vec1, sizeof(float), n, file);
    fwrite(vec2, sizeof(float), n, file);
    fwrite(&dotp, sizeof(double), 1, file);
    fclose(file);

    free(vec1);
    free(vec2);

    return 0;
}
