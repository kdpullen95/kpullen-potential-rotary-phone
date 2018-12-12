#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "csapp.h"
//
#define USERNAMELEN 32
#define CONTENTLEN 2048
#define TIMELEN 32
#define IPLEN 14
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

void* clientCycle();
void* hostCycle();
void* ping();
void setupConnection(char* buf, int connfd);
void mlog(char* str);
void mprint(char* str);
void printRecentMessages();
void* handleSconn(void* tempc);
void* saveToChatlog(void* message);
void sendMessage(char* buf);
void addToMessages(char* buf);
int loadHistory(char* fileName);
int startsWith(char *buf, char *str);


int VERBOSE = 0, HEADLESS = 1, SAVE = 0, LOAD = 0, HOST = 0;
//I know all caps for variables defies
                  // convention, but it makes my code easier to visually parse
sem_t fileMutex;
sem_t arrayMutex;
sem_t connMutex;

char recentMessages[MAXHISTORY][MAXLINE]; //TODO: replace with linked list
int newestMessage = 0;
struct connT connections[MAXCONN]; //TODO: linked list
int lastConn = 0;

struct connT host;
struct connT self;
pthread_t keyThread;
char chatlogName[CONTENTLEN];
char *pingString = "PING";

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
    host.ip = "127.0.0.1";
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

  listenfd = Open_listenfd(self.port);
  if (VERBOSE) mlog("starting server");

  while (1) {
    clientlen = sizeof(clientaddr);
    struct connT connect;
    connect.connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (VERBOSE) mlog("connection attempted");
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
  if (VERBOSE) mlog(">>> creating thread: sconn");
  Pthread_detach(pthread_self());
  int i = *((int *) tempc);
  char buf[MAXLINE];
  Free(tempc);
  rio_t rio;

  Rio_readinitb(&rio, connections[i].connfd);
  while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    if (VERBOSE) mlog(buf);
    if (startsWith(buf, "MSG{")) {
      char t[MAXLINE];
      sendMessage(t);
      sprintf(t, "%s", buf + 4);
      addToMessages(t);
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

  if (VERBOSE) mlog("<<< exiting thread: sconn");
  return NULL; //auto reap
}

int startsWith(char *buf, char *str) { //TODO: actual starts with
  return strstr(buf, str);
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

void sendMessagesOn(char* buf) {
  for (int i = 0; i < 100; i++) {
    if (connections[i].EXISTS == 1) {
      Rio_writen(connections[i].connfd, buf, strlen(buf));
    }
  }
}

void sendMessage(char* buf) {
  if (host.EXISTS) {
    Rio_writen(host.connfd, buf, strlen(buf));
  } else {
    //add to backlog, sync request
  }
}

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

void printRecentMessages() {
  P(&arrayMutex);
  for (int i = newestMessage + 1; i < MAXHISTORY; i++) {
    mprint(recentMessages[i]);
  }
  for (int i = 0; i < newestMessage + 1; i++) {
    mprint(recentMessages[i]);
  }
  V(&arrayMutex);
}

void mlog(char* str) {
  printf("|||||||||||||||||| %s\n", str);
}

void mprint(char* str) {
  printf("%s", str);
}
