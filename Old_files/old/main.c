
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
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

int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  char login[50];
  char senha[50];
  FILE* file = fopen( "e-mail.txt", "w");
  
  curl = curl_easy_init();
  if(curl) {
   
      struct string s;
       init_string(&s);

       
       printf("CURL OK!");
       printf("Digite seu login\n");
       scanf("%s", login);
       printf("Digite sua senha\n");
       scanf("%s", senha);
       
      
    /* Set username and password */
    curl_easy_setopt(curl, CURLOPT_USERNAME, login);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, senha);

    /* This will list every message of the given mailbox */
    curl_easy_setopt(curl, CURLOPT_URL, "imaps://imap.gmail.com:993/INBOX");
    
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "SEARCH FROM 'comprovantepormail@santander.com.br'");
    
    /* Perform the list */
    res = curl_easy_perform(curl);
    
     //printf("%s\n", s.ptr);
     //free(s.ptr);
     
     fclose(file);

    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* Always cleanup */
    curl_easy_cleanup(curl);
  }else{
      printf("CURL NOT OK!");
  }

  return (int)res;
}


/*

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
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

int main(void)
{
  CURL *curl;
  CURLcode res = CURLE_OK;
  
  char login[50];
  char senha[50];
  int i;
  char url[200];

  curl = curl_easy_init();
  if(curl) {
   
      struct string s;
       init_string(&s);
       
      // fp = fopen(outfilename,"wb");

       printf("CURL OK!");
       printf("Digite seu login\n");
       scanf("%s", login);
       printf("Digite sua senha\n");
       scanf("%s", senha);
       
        curl_easy_setopt(curl, CURLOPT_USERNAME, login);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, senha);
       
        
       for(i = 0; i < 10; i++){
            strcpy(url, "imaps://imap.gmail.com:993/INBOX/;UID=1"); 
            //sprintf(url, "%d", i);            
            
            printf("%s", url);
            
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

            res = curl_easy_perform(curl);
    
            printf("%s\n", s.ptr);
            free(s.ptr);
            
       }
   
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    curl_easy_cleanup(curl);
  }else{
      printf("CURL NOT OK!");
  }

  return (int)res;
}*/