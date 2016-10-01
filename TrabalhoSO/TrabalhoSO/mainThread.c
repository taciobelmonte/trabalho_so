#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <curl/curl.h>

#define buffer_size 10  //////

#define READ 0
#define WRITE 1
#define TRUE 1
#define FILENAME 200

typedef struct DataModel {
    int index;
    char *ptr;
    size_t len;
}DTModel;

//Global Variables
int indeX = 0;
CURL *curl;
CURLcode res = CURLE_OK;

//definicao de funcoes
void produtor(void *);
void consumidor(void *);
void initData(DTModel *s);

//definicao dos semaforos 
sem_t vazio;
sem_t cheio;
sem_t mutex;

int main(){
	pthread_t t_produtor, t_consumidor;

	sem_init(&mutex, 0, 1);
	sem_init(&vazio, 0, buffer_size);
	sem_init(&cheio, 0, 0);

	pthread_create(&t_produtor, NULL, (void *) produtor, NULL);
	pthread_create(&t_consumidor, NULL, (void *) consumidor, NULL);

	pthread_join(t_produtor, NULL);
	pthread_join(t_consumidor, NULL);

	return EXIT_SUCCESS;
}

void produtor (void *n){
	int item, i;
	while(1){
		//pruduz item
		//item = produz_item();
		sem_wait(&vazio);  //down em vazio
		sem_wait(&mutex);    //down em mutex para bloquear regiao critica
		for(i = 0;i<100000000;i++);
		printf("Produtor produziu item\n");
		//insere item no buffer
		sem_post(&mutex);
		sem_post(&cheio);	
		sleep(1);
	}
}

void consumidor(void *n){
	while(1){
		sem_wait(&cheio);  //down em vazio
		sem_wait(&mutex);    //down em mutex para bloquear regiao critica
		//remove item
		printf("Consumiu item\n");
		sem_post(&mutex);
		sem_post(&vazio);
		//faz algo com o item
		sleep(5);
	}
}

void initData(DTModel *s) {
    
    indeX++;
    
    s->len = 0;
    s->index = indeX;

    s->ptr = malloc(s->len+1);
    
    if (s->ptr == NULL) {
        
        fprintf(stderr, "malloc() failed\n");
        
        exit(EXIT_FAILURE);
    }
    
    s->ptr[0] = '\0';
}
