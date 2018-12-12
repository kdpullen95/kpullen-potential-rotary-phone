#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "csapp.h"
//
#define USERNAMELEN 32
#define CONTENTLEN 2048
#define TIMELEN 32
#define IPLEN 20
#define PORTLEN 5
#define MAXCONN 15
#define MAXHISTORY 40

struct connT {
  char username[USERNAMELEN];
  char ip[IPLEN];
  char port[PORTLEN];
  int connfd;
  int EXISTS;
};

//CLIENT ONLY ------------------------------------------------------------------
void* clientCycle();
void printRecentMessages();
void sendMessage(char* buf);

//SHARED -----------------------------------------------------------------------
void mlog(char* str);
void mprint(char* str);
int startsWith(char *buf, char *str);
void addToMessages(char* buf);

//HOST ONLY --------------------------------------------------------------------
void slog(char* str);
void hostCycle();
void* ping();
int loadHistory(char* fileName);
void sendMessageOn(char* buf, int connfd);
void* handleSconn(void* tempc);
void* saveToChatlog(void* message);


int VERBOSE = 0, HEADLESS = 1, SAVE = 0, LOAD = 0, HOST = 0;
//I know all caps for variables defies
                  // convention, but it makes my code easier to visually parse

//MUTEXES
sem_t fileMutex; //file sync
sem_t arrayMutex; //message array sync
sem_t connMutex; //connection array sync

char recentMessages[MAXHISTORY][MAXLINE]; //TODO: replace with linked list
int newestMessage = 0;
struct connT connections[MAXCONN]; //TODO: linked list
int lastConn = 0;

//important individuals
struct connT host;
struct connT self;

char chatlogName[CONTENTLEN];
char *pingString = "PING\n";

int main(int argc, char **argv)
{
  Sem_init(&fileMutex, 0, 1);
  Sem_init(&arrayMutex, 0, 1);
  Sem_init(&connMutex, 0, 1);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      HEADLESS = 0;
      mlog("not headless (not implemented)");
    } else
    if (strcmp(argv[i], "-v") == 0) {
      VERBOSE = 1;
      mlog("verbose");
    } else
    if (strcmp(argv[i], "-s") == 0) {
      SAVE = 1;
      sprintf(chatlogName, "log-%d", (int)time(NULL));
      mlog("saving chatlogs");
    } else
    if (strcmp(argv[i], "-l") == 0) {
      LOAD = 1;
      i++;
      mlog("will attempt to load chatlogs from file");
      mlog(argv[i+1]);
    }
  }

  if (argc > 1 && strcmp(argv[1], "-host") == 0) {
    HOST = 1;
    if (VERBOSE) mlog("starting as host");
    mprint("Enter Desired Port Number: \n");
    fgets(host.port, PORTLEN, stdin); getchar();
    strcpy(host.ip, "127.0.0.1");
    if (Fork() == 0) {
      hostCycle();
    }
  } else {
    if (VERBOSE) mlog ("starting as client");
    mprint("Enter IP of Host (format ###.###.###.###): ");
    fgets(host.ip, IPLEN, stdin); getchar();
    mprint("Enter Port of Host: ");
    fgets(host.port, PORTLEN, stdin); getchar();
  }

  SAVE = 0; //we only need save for the server

  pthread_t cycleThread;
  mprint("Enter Username: ");
  fgets(self.username, USERNAMELEN, stdin);
  self.username[strcspn(self.username, "\n")] = 0;

  host.connfd = Open_clientfd(host.ip, host.port);
  host.EXISTS = 1;
  connections[0] = host;
  Pthread_create(&cycleThread, NULL, clientCycle, NULL);

  char buf[CONTENTLEN];
  while(1) {
    fgets(buf, CONTENTLEN, stdin);
    if (VERBOSE) { mlog("sending on message"); mlog(buf); }
    char m[MAXLINE];
    sprintf(m, "MSG{[%d] %s: %s", (int)time(NULL), self.username, buf);
    sendMessage(m);
    addToMessages(m);
    printRecentMessages();
  }
}

void* clientCycle() {
  Pthread_detach(pthread_self());
  if (VERBOSE) mlog("<<< entering client thread");
  rio_t rio; char buf[MAXLINE];
  Rio_readinitb(&rio, host.connfd);
  while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    if (VERBOSE) mlog(buf);
    if (startsWith(buf, "MSG{")) {
      addToMessages(buf);
      printRecentMessages();
    }
  }
  return NULL;
}

void sendMessage(char* buf) {
  if (host.EXISTS) {
    Rio_writen(host.connfd, buf, strlen(buf));
  } else {
    //add to backlog, sync request
  }
}

void printRecentMessages() {
  P(&arrayMutex);
  char t[MAXLINE];
  for (int i = newestMessage + 1; i < MAXHISTORY; i++) {
    sprintf(t, "%s", recentMessages[i] + 4);
    mprint(t);
  }
  for (int i = 0; i < newestMessage + 1; i++) {
    sprintf(t, "%s", recentMessages[i] + 4);
    mprint(t);
  }
  V(&arrayMutex);
}

//SHARED -----------------------------------------------------------------------

void addToMessages(char* buf) {
  P(&arrayMutex);
  newestMessage++;
  strcpy(recentMessages[newestMessage], buf);
  V(&arrayMutex);
  if (SAVE) {
    if (VERBOSE) mlog("saving to chatlog");
    P(&fileMutex);
    FILE * fp;
    fp = fopen(chatlogName, "ab+");
    fprintf(fp, "%s", buf);
    fclose(fp);
    V(&fileMutex);
  }
}

int startsWith(char *buf, char *str) { //TODO: actual starts with
  return strstr(buf, str);
}

void mlog(char* str) {
  //str[strcspn(str, "\n")] = 0; seg fault whyyyyyyyyyyyyy
  printf("|||||||||||||||||| %s\n", str);
}

void mprint(char* str) {
  printf("%s", str);
}


//HOST ONLY --------------------------------------------------------------------

void slog(char* str) {
  //str[strcspn(str, "\n")] = 0;
  printf("|||||||||||||||||| S: %s\n", str);
}

void sendMessageOn(char* buf, int connfd) {
  for (int i = 0; i < 100; i++) {
    if (connections[i].EXISTS == 1 && connections[i].connfd != connfd) {
      Rio_writen(connections[i].connfd, buf, strlen(buf));
    }
  }
}

void* ping() {
  while(1) {
    Sleep(10);
    for (int i = 0; i < MAXCONN; i++) {
      if (connections[i].EXISTS == 1) {
        Rio_writen(connections[i].connfd, pingString, strlen(pingString));
      }
    }
  }
  return NULL;
}

void hostCycle() {
  pthread_t pingThread;
  Pthread_create(&pingThread, NULL, ping, NULL);
  int listenfd;
  socklen_t clientlen;
  pthread_t sconnThread;
  struct sockaddr_storage clientaddr;

  listenfd = Open_listenfd(host.port);
  if (VERBOSE) slog("starting server");

  while (1) {
    clientlen = sizeof(clientaddr);
    struct connT connect;
    connect.connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (VERBOSE) slog("connection attempted");
    connect.EXISTS = 1;
    P(&connMutex);
    if (lastConn == MAXCONN - 1) { lastConn = 0; } else { lastConn++; }
    int *i = Malloc(sizeof(*i));
    *i = lastConn;
    connections[lastConn] = connect;
    V(&connMutex);
    Pthread_create(&sconnThread, NULL, handleSconn, i);
      /*accept incoming connections*/
  }
}

void* handleSconn(void* tempc) {
  if (VERBOSE) slog(">>> creating thread: sconn");
  Pthread_detach(pthread_self());
  int i = *((int *) tempc);
  char buf[MAXLINE];
  Free(tempc);
  rio_t rio;

  Rio_readinitb(&rio, connections[i].connfd);
  while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    if (VERBOSE) slog(buf);
    if (startsWith(buf, "MSG{")) {
      sendMessageOn(buf, connections[i].connfd);
      addToMessages(buf);
    } else
    if (startsWith(buf, "SYNCREQ{")) {

    } else
    if (startsWith(buf, "ENDSESS{")) {
      break;
    }
  }

  P(&connMutex);
  connections[i].EXISTS = 0;
  V(&connMutex);

  if (VERBOSE) slog("<<< exiting thread: sconn");
  return NULL; //auto reap
}
