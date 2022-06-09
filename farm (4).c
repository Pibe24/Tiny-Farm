#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "xerrori.h"

#define HOST "127.0.0.1"
#define PORT 65432
#define QUI __LINE__,__FILE__

//funzione writen per andare a scrivere nel socket tutti i byte

ssize_t  writen(int fd, void *ptr, size_t n) {  
   size_t   nleft;
   ssize_t  nwritten;
 
   nleft = n;
   while (nleft > 0) {
     if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount written so far */
     } else if (nwritten == 0) break; 
     nleft -= nwritten;
     ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}

//variabile globale per il segnale del tipo volatile per evitare ottimizazioni
volatile bool continua=true;


//andiamo a scrivere l'handler per gestire sigint

void handler (int s){

  printf("\narriavto il segnale %d al master worker\n",s);

  if(s==SIGINT){
    continua=false;
  }
}

//finito l'handler


//struct per la sincronizazione dei thread e condivisione dei dati

typedef struct {
//buffer
char**buffer;
//indice che va a cambiare
int *c;
//dimensione del buffer
int *q;
//nome del file da aprire
//char*nomefile;
//risultato finale
long risultato;
//semafori
sem_t *sem_free_slot;
sem_t *sem_items;
//mutex
pthread_mutex_t *cmutex;

}dati;



//corpo degli n threads

void *tbody(void* data) {

  dati*d=(dati*) data;
  
  //vado ad aprire la socket
  int sock_fd=0;
  struct sockaddr_in serv_addr;
  size_t e,e1;
  //int tmp=0;

  //vado a creare la socket con il controllo

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    xtermina("Errore creazione socket",QUI);

   serv_addr.sin_family = AF_INET;
  // converto numero della porta 
   serv_addr.sin_port = htons(PORT);
   serv_addr.sin_addr.s_addr = inet_addr(HOST);
  // apro la connessione
  if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    xtermina("Errore apertura connessione",QUI);
  

  while(true){

    char nome[256];
    
    //vado a leggere dal buffer
    
    xsem_wait(d->sem_items,QUI);
    
    xpthread_mutex_lock(d->cmutex,QUI);

    
    strcpy(nome,d->buffer[*(d->c) % *(d->q)]);
    *(d->c)+=1;

    xpthread_mutex_unlock(d->cmutex,QUI);
 
    xsem_post(d->sem_free_slot,QUI);


    //finito la lettura dal buffer

    //confronto il nome del file con quello di terminazione

    if(strcmp(nome,"No file")==0){


       e=writen(sock_fd,&nome,sizeof(nome));

       if(e!=sizeof(nome)){
         xtermina("Errore nell invio del nome",QUI);
         }
  
      break;
    }

    //vado ad aprire il file
    FILE*f=fopen(nome,"r");

    //controllo sulla apertura del file
    if(f==NULL){
      puts("file non valido vado avanti a valutare gli altri");
     // tmp=0;
      continue;
    }

    //vado a leggere dentro il file
    long k=0,prod=0,somma=0;
    while (true){

      long num;
      //vado a leggere dal file
      int e= fread(&num,1,sizeof(long),f);
      
      if(e<=0){
        break;
      }
      prod=k*num;
      somma+=prod;
      
      k++;
    }

    d->risultato=somma;
    printf("la somma nel thread vale esattamente :%ld\n",d->risultato);
    
    long numero=d->risultato;
  
    char str[256];
    sprintf(str, "%ld", numero);
    printf("la somma come stringa %s\n", str);
     
    e=writen(sock_fd,&nome,sizeof(nome));
      if(e!=sizeof(nome)){
        xtermina("errore passaggio dati",QUI);
      } 

    e1=writen(sock_fd,&str,sizeof(str));
    if(e1!=sizeof(str)){
      xtermina("errore passaggio del nome del file",QUI);
    }

    
     
    if(fclose(f)<0){
      xtermina("errore chiusura file",QUI);
    }
    
    
    }

  
  if(close(sock_fd)<0){
    xtermina("errore chiusura connessione",QUI);
  }

  pthread_exit(NULL);
  
}
//fine corpo dei threads



int main(int argc, char *argv[])
{

  
  // controlla numero argomenti
  if(argc<2) {
      printf("Uso: %s file [file ...] \n",argv[0]);
      return 1;
  }

  //per salvare valore di ritorno della funzione getopt
  int e=0;
  //indice per lo scorrimento della linea di comando
  int quanti=0;//indica il numero di parametri opzionali
  int n,q,t;
  //valori di default dei parametri opzionali
  n=4;//numero dei thread
  q=8;//lunghezza del buffer
  t=0;//tempo di ritardo millisecondi

  // vado a controllare la presenza degli eventuali parametri opzionali
  while((e=getopt(argc,argv,"n:q:t:"))!=-1) {
    //studiare caso per caso
    switch(e){
      
      case 'n':
        quanti++;
        n=atoi(optarg);
        break;

      case 'q':
        quanti++;
        q=atoi(optarg);
        break;

      case 't':
        quanti++;
        t=atoi(optarg);
        break;

      default:
        puts("errore nel match della optzione");
        break;
    }
  }//fine studio parametri opzionali

//gestione del segnale SIGINT
  
  struct sigaction sa;
  sa.sa_handler=&handler;//quando arriva il segnale parte l'handler

  //andiamo a mettere la maschera per bloccare gli altri segnali
  sigemptyset(&sa.sa_mask);
  //andiamo a fare il restart in caso le system call vengano bloccati
  sa.sa_flags=SA_RESTART;
  //adesso andiamo a lanciare l'handler con sigaction
  sigaction(SIGINT,&sa,NULL);
  

  //creaimo i semafori per sincronizazione letture e scritture nel buffer
  sem_t sem_free_slot, sem_items;
  xsem_init(&sem_free_slot,0,q,QUI);
  xsem_init(&sem_items,0,0,QUI);

  //vado a creare i mutex

  pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;

  //vado a creare il buffer e gli elementi per operarci sopra
  

  char**buffer=malloc(q*sizeof(char*));
  int p=0,c=0;

  //creo array di dati e dei thread pid

  dati *d=malloc(n*sizeof(dati));
  pthread_t th[n];

  //vado a creare i tread

  for(int i=0;i<n;i++){
    d[i].buffer=buffer;
    d[i].c=&c;
    d[i].q=&q;
    d[i].risultato=0;
    d[i].cmutex=&cmutex; 
    d[i].sem_free_slot=&sem_free_slot;
    d[i].sem_items=&sem_items;

    xpthread_create(&th[i],NULL,&tbody,&d[i],QUI);
    }

  //vado a leggere i file 

  for(int i=1+quanti;i<argc;i++){

    //sleep(2);
    if(continua==false){
      break;
    }

    else{
      
    xsem_wait(&sem_free_slot,QUI);
    buffer[p%q]=strdup(argv[i]);
    printf("%s da lettura linea di comando\n",buffer[p%q]);
    p++;
    xsem_post(&sem_items,QUI);
      }
  }

  //vado a scrivere i valori di terminazione
  for(int i=0;i<q;i++) {
    //sleep(2);
    xsem_wait(&sem_free_slot,QUI);
    buffer[p++ % q]=strdup("No file");
    xsem_post(&sem_items,QUI);
  }

   for(int i=0;i<n;i++) {
    xpthread_join(th[i],NULL,QUI);
    printf(" somma : %ld \n",d[i].risultato);
  }
  //vado a mandare messaggio di terminazione per il server

  int sock_fd=0;
  struct sockaddr_in serv_addr;
  size_t errore;
  //int tmp=0;

  //vado a creare la socket con il controllo

  if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    xtermina("Errore creazione socket",QUI);

   serv_addr.sin_family = AF_INET;
  // converto numero della porta 
   serv_addr.sin_port = htons(PORT);
   serv_addr.sin_addr.s_addr = inet_addr(HOST);
  // apro la connessione
  if (connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    xtermina("Errore apertura connessione",QUI);

  //una volta aperta la socket mando il messaggio di terminazione
  char messaggio[256];
  strcpy(messaggio,"fine");
  errore=writen(sock_fd,&messaggio,sizeof(messaggio));
  if(errore!=sizeof(messaggio)){
    xtermina("errore nel mandare il messaggio di terminazione",QUI);
  }

  //una volta inviato chiudo il socket
  if(close(sock_fd)<0){
    xtermina("errore nella chiusra del socket",QUI);
  }
  

  free(d);
  free(buffer);
  
	return 0;
}