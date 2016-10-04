/*
 UNIVERSIDADE FEDERAL DA BAHIA
 Trabalho de Sistemas Operacionais
 Professor: Ricardo Rios
 Alunos: Tácio Belmonte e Marivaldo Júnior
 */

#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include <string.h>
#include<pthread.h>
#include <sys/types.h>
#include<sys/ipc.h>
#include<semaphore.h>
#include <curl/curl.h>

#define buffer_size 10
#define FILENAME 200

typedef struct DataModel {
    char *ptr;
    size_t len;
}DTModel;


//Global Variables
int indeX = 0;
CURL *curl;
CURLcode res = CURLE_OK;

int qtdProduzido = 0;
int totalEmails = 0;
int totalConsumidos = 0;
int buffer[buffer_size]; // regiao critica
int bufferCounter = 0;
int flag = 1;

//Definição de Funções
void produtor(void *n);
void consumidor(void *n);
void initData(DTModel *s);

//Definição dos Semáforos
sem_t vazio;
sem_t cheio;
sem_t mutex;

void imprimeBuffer(){

    for(int i = 0; i < buffer_size; i++){
        
        printf("%d ", buffer[i]);
        
        if(i < 9){
            printf("- ");
        }
    }
    printf("\n");
}

size_t getData(void *ptr, size_t size, size_t nmemb, DTModel *s)
{
    size_t new_len = s->len + size*nmemb;
    
    s->ptr = realloc(s->ptr, new_len+1);
    
    if (s->ptr == NULL) {
        
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
        
    }
    
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    
    s->ptr[new_len] = '\0';
    s->len = new_len;
    
    return size*nmemb;
}

void produtor (void *n){
    
	while( flag ){
        
        //produz item
        qtdProduzido++;
        totalEmails--;
        
        sem_wait(&vazio);  //down em vazio
		sem_wait(&mutex);    //down em mutex para bloquear regiao critica
        
        /*REGIAO CRITICA*/
        
            printf("\n------PRODUTOR------\n");
            printf("Produziu item %d\n", qtdProduzido);
        
            //for(i = 0;i<100000000;i++);
            //printf("Produtor produziu item\n");
        
            //insere item no buffer
            buffer[bufferCounter] = qtdProduzido;
            bufferCounter++;
        
            printf("------PRODUTOR------\n");

        /*REGIAO CRITICA*/
        
		sem_post(&mutex);
		sem_post(&cheio);
        
		sleep(rand() % 5);
        
        if(totalEmails <= 0){
            flag = 0;
        }
	}
}

void consumidor(void *n){
    int item;
    char emailID[5];
    
	while( 1 ){
        
		sem_wait(&cheio);       //down em vazio
		sem_wait(&mutex);       //down em mutex para bloquear regiao critica
        
        /*REGIAO CRITICA*/
		
            //remove item
            printf("\n------CONSUMIDOR------\n");
            printf("Consumiu item %d\n", bufferCounter-1);
        
            bufferCounter--;
            item = buffer[bufferCounter];
            buffer[bufferCounter] = 0;
            totalConsumidos--;
        
            printf("\n");
            imprimeBuffer();
        
            printf("------CONSUMIDOR------\n");
        
        /*REGIAO CRITICA*/
        
		sem_post(&mutex);
		sem_post(&vazio);
        
        //Recupera o e-mail via IMAP com o id que foi recuperado do buffer
        char url[100]= "imaps://imap.gmail.com:993/INBOX/;UID=";
        sprintf(emailID, "%d", item); //Transforma o int em char
        strcat(url, emailID); //Concatena para forma a URL
        
        DTModel data;
        initData(&data); // Inicia a struct que vai receber os dados
        
        //Passa url montada para o curl
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        //Define a funcao que vai receber os dados
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getData);
        
        //Escreve os dados na struct
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        
        //Realiza a operação
        res = curl_easy_perform(curl);
        
        //Checa se o retorno da operacao foi 1
        if((int)res == CURLE_OK){
            
            char filename[200];
            
            //Define o nome do arquivo
            snprintf(filename, FILENAME ,"email%d.txt", item);
            
            // Certifica-se que o nome do arquivo ira escapar o simbolo de finalizacao
            filename[FILENAME-1]=0;
            
            //Abre arquivo
            FILE* file = fopen( filename, "w");
            
            if(file != NULL){
                //Insere conteudo no arquivo
                printf("     - Consumidor criou  arquivo!\n");
                fputs("E-mail:", file);
                fputs(emailID, file);
                fputs("\n\n", file);
                fputs(data.ptr, file);
                fclose(file);
            }
        }
        
        //Interrompe execucao caso o total de consumidos seja menor ou igual a zero (Previnir que o programa rode infinitamente)
        if(totalConsumidos <= 0){
            exit(0);
        }
	}
}


void initData(DTModel *s) {
    
    s->len = 0;
    s->ptr = malloc(s->len+1);
    
    if (s->ptr == NULL) {
        
        fprintf(stderr, "malloc() failed\n");
        
        exit(EXIT_FAILURE);
    }
    
    s->ptr[0] = '\0';
}


int main(){
    char login[100];
    char senha[100];
    pthread_t t_produtor, t_consumidor;
    
    sem_init(&mutex, 0, 1);
    sem_init(&vazio, 0, buffer_size);
    sem_init(&cheio, 0, 0);
    
    //Init curl
    curl = curl_easy_init();

    if(curl) {
        
        printf("\n");
        
        printf("Digite seu login\n");
        scanf("%s", login);
        
        printf("Digite sua senha\n");
        scanf("%s", senha);
        
        printf("Digite o numero de e-mails que deseja salvar\n");
        scanf("%d", &totalEmails);
        
        totalConsumidos = totalEmails;
        
        //Inicia Buffer
        for(int i = 0; i < buffer_size; i++){
            buffer[i] = 0;
        }
        
        printf("INIT BUFFER\n");
        imprimeBuffer();
        
        curl_easy_setopt(curl, CURLOPT_USERNAME, login);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, senha);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        
        //Inicia as threads
        pthread_create(&t_produtor, NULL, (void *) produtor, NULL);
        pthread_create(&t_consumidor, NULL, (void *) consumidor, NULL);
        
        pthread_join(t_produtor, NULL);
        pthread_join(t_consumidor, NULL);
        
    }
    
    return EXIT_SUCCESS;
}

