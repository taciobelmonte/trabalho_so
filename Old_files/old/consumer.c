#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <curl/curl.h>

#define READ 0
#define WRITE 1
#define TRUE 1

//Struct
struct string {
  char *ptr;
  size_t len;
};

CURL *curl;
CURLcode res = CURLE_OK;

//Functions Definitions
void createItem(int i);
void consumeItem(int i);
void producer(int fd[2], int qtde_itens);
void consumer(int fd[2]);


void initString(struct string *s) {
  s->len = 0;
  
  s->ptr = malloc(s->len+1);
  
  if (s->ptr == NULL) {
      
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
    
  }
  
  s->ptr[0] = '\0';
}

size_t writeFunc(void *ptr, size_t size, size_t nmemb, struct string *s)
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

void producer(int fd[2], int qtde_itens) {
    close(fd[READ]);

    int i, bytesEscritos;
    char url [ ] = "imaps://imap.gmail.com:993/INBOX/;UID=1";
    char number[5];
    
    for (i = 1 ; i <= qtde_itens; i++) {
        
        struct string s;
        initString(&s);
        
        sleep( rand() % 5 );
        sprintf(number, "%d", i);
        
        printf("Entrou\n");
        
       // strcat(url, number);
        
      //  printf("%s", url);
        
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    
        res = curl_easy_perform(curl);

        printf("%s\n", s.ptr);
        
        //Create Item here
        printf("Produtor criou o %d.o item.\n", i);

        //Check for errors    
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));

        /* Escreve no pipe */
        bytesEscritos = write(fd[WRITE], s.ptr, (strlen(s.ptr)+2));
        
        printf("ESCREVEU NO PIPE");

        if (bytesEscritos == -1) {
           perror("Erro de escrita no pipe!");
        }
        
        curl_easy_cleanup(curl);
    }
    close (fd[WRITE]);
}

void consumer(int fd[2]){
    close (fd[WRITE]);

    int i, bytesLidos;
    char readBuffer[3000];
    
    while (TRUE) {
    
        /* lê do pipe */
        bytesLidos = read (fd[READ], readBuffer, sizeof(readBuffer));
        
        sleep( rand() % 5 );
        
        //Cosume item here
      //  char filename [] = "email";
       // sprintf(filename, "%d", i);
       // strcat(filename, ".txt");

        FILE* file = fopen( "email.txt", "w");
        
        if(file != NULL){
            fputs(readBuffer, file);
            fclose(file);
        }

        printf("Consumidor consumiu o %d.o item.\n", i);
        
        if (bytesLidos == -1) {
           perror("Erro de leitura no pipe!");
        } else if (bytesLidos == 0) {
           break;
        }
    }
    close (fd[READ]);
}


int main (){
    
    int fd[2];
    char login[50];
    char senha[50];
    int qtd;
    
    /* Create Pipe */
    pipe (fd);
    
    //Initialize Curl
    curl = curl_easy_init();
    
    if(curl) {
     
        printf("CURL OK!");
        printf("Digite seu login\n");
        scanf("%s", login);
        printf("Digite sua senha\n");
        scanf("%s", senha);
        printf("Digite o numero de e-mails que deseja salvar\n");
        scanf("%d", &qtd);
        
        //Login - Gmail
        curl_easy_setopt(curl, CURLOPT_USERNAME, login);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, senha);
        
        /* Create new process */
        int pid = fork();

        if (pid == -1) {

            perror("Erro ao criar um novo processo!");

        } else if (pid == 0) {

            /* o novo processo funciona como produtor */
            producer(fd, qtd);

        } else {

            /* o processo pai funciona como consumidor */
            consumer(fd);

        }
    
    }else{
      printf("Curl nao foi inicializado!");
    }
   
    return 0;
}
