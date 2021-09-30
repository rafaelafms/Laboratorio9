//implementa produtores/consumidores usando semaforos

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<semaphore.h>

#define N 5 //tamanho do buffer
#define P 1 //quantidade de threads produtoras
#define C 10 //quantidade de threads consumidoras

//variaveis do problema
int Buffer[N]; //espaco de dados compartilhados
int count=0, in=0, out=0;

//semaforos
sem_t mutex, cond, consumidora;

//inicializa o buffer
void IniciaBuffer(int n){
    int i;
    for(i=0; i<n; i++)
        Buffer[i] = 0;
}

//imprime buffer
void ImprimeBuffer(int n){
    int i;
    for(i=0; i<n; i++)
        printf("%d ", Buffer[i]);
    printf("\n");
}

//insere um elemento no Buffer ou bloqueia a thread caso o Buffer esteja cheio
void Insere (int item, int id){
    sem_wait(&mutex); //exclusao mutua
    printf("P[%d] quer inserir\n", id); //produtora chega para inserir
    while(count == N){ //buffer cheio
        printf("Produtora %d bloqueou\n", id); 
        sem_post(&consumidora); //libera a consumidora
        sem_wait(&cond); //bloqueia a produtora
        printf("Produtora %d desbloqueou\n", id);
    }
    Buffer[in] = item;
    in = (in + 1) % N;
    count++;
    printf("Produtora %d inseriu\n", id);
    ImprimeBuffer(N);
    sem_post(&mutex); //libera a entrada da proxima thread
}

//retira um elemento do Buffer ou bloqueia a thread caso o Buffer esteja vazio
int Retira (int id){
    int item;
    sem_wait(&consumidora); //verifica se a consumidora esta liberada
    printf("Consumidora %d quer consumir\n", id);
    while(count == 0){ //buffer vazio
        printf("Consumidora %d bloqueou\n", id);
        sem_wait(&consumidora); //bloqueia a produtora
        printf("Consumidora %d desbloqueou\n", id);
    }
    for(int i=0; i<N; i++){ //consome todos os elementos do buffer 
        Buffer[out] = 0;
        out = (out + 1) % N;
        item = Buffer[out];
    }
    count = count-N;
    printf("Consumidora %d consumiu o buffer\n", id);
    sem_post(&cond); //libera a produtora
    ImprimeBuffer(N);
    return item;
}

//thread produtora
void * produtor(void * arg){
    int *id = (int *) arg;
    while(1){
        Insere(*id, *id);
        sleep(1);
    }
    free(arg);
    pthread_exit(NULL);
}

//thread consumidora
void * consumidor(void * arg){
    int *id = (int *) arg;
    while(1){
        Retira(*id); //pode bloquear a thread
        sleep(1);
    }
    free(arg);
    pthread_exit(NULL);
}

//funcao principal
int main(void){
    //variaveis auxiliares
    int i;

    //identificadores das threads
    pthread_t tid[P+C];
    int *id[P+C];

    //aloca espaco para os IDs das threads
    for(i=0; i<P+C; i++){
        id[i] = malloc(sizeof(int));
        if(id[i] == NULL) exit(-1);
        *id[i] = i+1;
    }

    //inicializa o Buffer
    IniciaBuffer(N);

    //inicializa os semaforos
    sem_init(&mutex, 0, 1); //exclusao mutua, inicia com 1
    sem_init(&cond, 0, 0); //utilizo quando quero bloquear a produtora (quando nao ha mais espaco no buffer para inserir elementos)
    sem_init(&consumidora, 0, 0); //so permito a entrada nas consumidoras quando a produtora ja inseriu todos os elementos do buffer, libero pelas produtoras 

    //cria as threads produtoras
    for(i=0; i<P; i++){
        if(pthread_create(&tid[i], NULL, produtor, (void *) id[i])) exit(-1);
    }

    //cria as threads consumidoras    
    for(i=0; i<C; i++){
        if(pthread_create(&tid[i+P], NULL, consumidor, (void *) id[i+P])) exit(-1);
    }

    pthread_exit(NULL);
    return 1;
}
