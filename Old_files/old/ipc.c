#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>

#define N 10                        /* tamanho do buffer */

sem_t mutex;          
sem_t empty;          
sem_t full;  

int buffer[N];

int it = 0;  

int produce_item(){
	return it++;

}

//cinicializa todos os valores do vetor em zero
void init_buffer(int *vet){
	int i;
	for (i = 0;i<N;i++){
		vet[i] = 0;
	}	
}

void insert_item(int item){
	int i;
	for (i = 0;i<N;i++){
		if(buffer[i] == 0){
			buffer[i] = item;
		}
	}
	return;
	
}


void removeItem(){
	
}

void producer () {
    int item;
        while (1) {
            item = produce_item();      	 /* gera um item */
            sem_wait(&empty);                /* decrementa o contador de slots vazios */
            sem_wait(&mutex);                /* entrada na região crítica */
            insert_item(item);               /* insere item no buffer compartilhado */
            sem_post(&mutex);                /* saída da região crítica */
            sem_post(&full);                 /* incrementa contagem de slots ocupados */
        }
}

void consumer() {
    int item;

    while (1) {
        sem_wait(&full);                 /* decrementa o contador de slots ocupados */
        sem_wait(&mutex);                /* entrada na região crítica */
        //item = remove_item();          /* retira item do buffer */
        sem_post(&mutex);                /* saída da região crítica */
        sem_post(&empty);                /* incrementa contagem de slots vazios */
        //consume_item(item)             /* faz algo com o item retirado */
    }          

}

int main(){
	
	int status;
	
	 init_buffer(buffer);
	
	 int i = 0;
	 	 
	 for(i=0;i<N;i++){
		 printf("%d ", buffer[i]);
	 }
	 printf("\n");
	 
	 long pid = fork();
	 if(pid == 0){
		 printf("processo Filho\n");
		 producer();
	 }
	 else if(pid <0){
		 printf("Erro ao cricar processo\n");
	 }
	 else{
		 //wait(&status);
		 printf("Processo pai\n");
		 consumer();
	 } 
	 return 1;
	 
	 
}



