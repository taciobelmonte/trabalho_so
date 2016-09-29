/*
 UNIVERSIDADE FEDERAL DA BAHIA
 Trabalho de Sistemas Operacionais
 Professor: Ricardo Rios
 Alunos: Tácio Belmonte e Marivaldo
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#include <curl/curl.h>

#define READ 0
#define WRITE 1
#define TRUE 1
#define FILENAME 200

//Global Variables
int indeX = 0;
CURL *curl;
CURLcode res = CURLE_OK;

typedef struct DataModel {
    int index;
    char *ptr;
    size_t len;
}DTModel;


//Functions Definitions
void createItem(int i);
void consumeItem(int i);
void producer(int fd[2], int qtde_itens);
void consumer(int fd[2]);
void initString(DTModel *s);


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

void producer(int fd[2], int qtd) {
    
    close(fd[READ]);
    
    int i, bytesEscritos;
    char emailnumber[5];
    
    
    for (i = 1 ; i <= qtd; i++) {
        
        //Define the url to search via Curl
        char url[100]= "imaps://imap.gmail.com:993/INBOX/;UID=";
        sprintf(emailnumber, "%d", i);
        strcat(url, emailnumber);
        
        sleep( rand() % 5 );
        
        //Initialize the struct that will retrieve the data
        DTModel dataWritten;
        initData(&dataWritten);

        
        //Logic to create the e-mail file
        printf("Produtor inseriu o e-mail %d no pipe.\n", dataWritten.index);
        
        //Pass the url to the CURL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        //Define the function that will read the data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, getData);
        
        
        //Write the data in the struct created
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dataWritten);

        //Perform the query
        res = curl_easy_perform(curl);
        
        //printf("%d\n", data.index);
        //printf("%s\n", data.ptr);
        
        /* escreve no pipe */
        //bytesEscritos = write(fd[WRITE], data.ptr, (strlen(data.ptr)+1));
        
        bytesEscritos = write(fd[WRITE], &dataWritten, sizeof(struct DataModel));
        
        if (bytesEscritos == -1) {
            perror("Erro de escrita no pipe!");
        }
        
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        
       // free(data.ptr);
        
    }
    close (fd[WRITE]);
}

void consumer(int fd[2]){
    close (fd[WRITE]);
    
    int counter = 1;
    char filename[200];
    char email[100000];
    
    int bytesLidos;
    
    while (TRUE) {
        
        DTModel dataReturned;
        
        /* lê do pipe */
        //bytesLidos = read (fd[READ], email, sizeof(email));
        bytesLidos = read (fd[READ], &dataReturned, sizeof(struct DataModel));
        
        printf("LEU DO PIPE:\n %s\n", dataReturned.ptr);

        sleep( rand() % 5 );
        
        //Logica para consumir o valor do buffer
        printf("Consumidor pegou o e-mail %d do pipe.\n", dataReturned.index);
        
        //Set the filename
        snprintf(filename, FILENAME ,"email%d.txt", counter);
        counter++;
        
        //To be ensure that the filename will skip the terminator char.
        filename[FILENAME-1]=0;
        
        //Open file
        FILE* file = fopen( filename, "w");
        
        if(file != NULL){
            printf("Entrou no teste da abertura do arquivo\n");
            
            fputs("E-mail", file);
            fprintf(file, "%d", dataReturned.index);
            fputs("\n", file);
            //fputs(email, file);
            fputs(dataReturned.ptr, file);
            fclose(file);
        }
        
        printf("Criou arquivo\n");
        
        //printf("Conteudo: %s", data.ptr);
        
        if (bytesLidos == -1) {
            perror("Erro de leitura no pipe!");
        } else if (bytesLidos == 0) {
            break;
        }
    }
    close (fd[READ]);
}


int main (int argc, char** argv){
    int fd[2];
    char login[100];
    char senha[100];
    int qtd;
    
 
    
    //Init curl
    curl = curl_easy_init();
    
    if(curl) {
        
        /* Create Pipe  */
        pipe (fd);
        
        printf("CURL OK!");
        
        printf("Digite seu login\n");
        scanf("%s", login);
        
        printf("Digite sua senha\n");
        scanf("%s", senha);
        
        printf("Digite o numero de e-mails que deseja salvar\n");
        scanf("%d", &qtd);
        
        
        curl_easy_setopt(curl, CURLOPT_USERNAME, login);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, senha);
        
        
        /* Create new Process */
        int pid = fork();
        
        
        if (pid == -1) {
            
            perror("Erro ao criar um novo processo!");
            
        } else if (pid == 0) {
            
            /* the new process works as a producer*/
            producer(fd, qtd);
            
        } else {
            
            /* father process works as a consumer */
            consumer(fd);
        }
        
    
    }else{
        
        printf("CURL NOT OK!\n");
        
    }
    
    return 0;
}
