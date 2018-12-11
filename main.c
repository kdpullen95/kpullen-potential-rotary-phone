#include <stdio.h>
#include <string.h>
#include "csapp.h"

#define USERNAMELEN 32
#define CONTENTLEN 32
#define TIMELEN 32

struct messageT {
  char username[USERNAMELEN];
  char content[CONTENTLEN];
  char time[TIMELEN];
};

struct connT {
  char username[USERNAMELEN];
  char ip[14];
  char port[5];
};

void clientCycle();
void hostCycle();
void parseAdd(char message);
void changeUsername(char buf);
void setupConnection(char buf);
void* listenKeyboard();
void handleSync();
void connectionTimer();
void mlog(char* str);
void mprint(char* str);
void printRecentMessages();
void* handleSconn(void* tempc);
void* saveToChatlog(void* message);
int syncRequest();
int loadHistory(char* fileName);
int startsWith(char *buf, char *str);
char* grabUsername(struct connT user);


int VERBOSE = 0, HEADLESS = 0, SAVE = 0, LOAD = 0;
//I know all caps for variables defies
                  // convention, but it makes my code easier to visually parse
sem_t fileMutex;
sem_t arrayMutex;
struct messageT recentMessages[40]; //TODO: replace with linked list if time
int newestMessage = 0;
struct connT connections[100]; //TODO: linked list if time
struct connT host;
struct connT self;
pthread_t keyThread;

int main(int argc, char **argv)
{
  Sem_init(&fileMutex, 0, 1);
  Sem_init(&arrayMutex, 0, 1);

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      HEADLESS = 1;
    } else
    if (strcmp(argv[i], "-v") == 0) {
      VERBOSE = 1;
    } else
    if (strcmp(argv[i], "-s") == 0) {
      SAVE = 1;
    } else
    if (strcmp(argv[i], "-l") == 0) {
      LOAD = 1;
      i++;
    }
  }

  if (argc > 1 && strcmp(argv[1], "-host") == 0) {
    if (VERBOSE) mlog("starting as host");
    hostCycle();
  } else {
    if (VERBOSE) mlog ("starting as client");
    clientCycle();
  }
}

void hostCycle() {
  int listenfd;
  socklen_t clientlen;
  pthread_t sconnThread;
  struct sockaddr_storage clientaddr;

  mprint("Enter Desired Port Number: \n");
  char portNum[5];
  fgets(portNum, 5, stdin);
  getchar();
  mprint("Enter Username: \n");
  fgets(self.username, 32, stdin);

  //no it might be C
  if (VERBOSE) mlog("read in port, username");

  Pthread_create(&keyThread, NULL, listenKeyboard, NULL);
  listenfd = Open_listenfd(portNum);

  if (VERBOSE) mlog("starting server");
  while (1) {
    clientlen = sizeof(clientaddr);
    int *connfd = Malloc(sizeof(*connfd));
    *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (VERBOSE) mlog("connection attempted");
    Pthread_create(&sconnThread, NULL, handleSconn, connfd);
      /*accept incoming connections*/
  }

}

void* handleSconn(void* tempc) {
  int connfd = *((int *) tempc);
  char buf[MAXLINE];
  if (VERBOSE) mlog(">>> creating thread: sconn");
  Pthread_detach(pthread_self());
  Free(tempc);
  struct connT connection;
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while(Rio_readlineb(&rio, buf, MAXLINE) != 0) {
    if (VERBOSE) mlog(buf);
    if (startsWith(buf, "INTCON{")) {
      setupConnection(buf);
    } else
    if (startsWith(buf, "MSG{")) {
      parseAdd(buf);
    } else
    if (startsWith(buf, "CHNUSERN{")) {
      changeUsername(buf);
    }
  }
  ///handle scones
  //parse, addtomessages

  //pass on to others in list of other name
  if (VERBOSE) mlog("<<< exiting thread: sconn");
  return NULL; //auto reap
}

void changeUsername(char buf) {

}

void setupConnection(char buf) {

}

int startsWith(char *buf, char *str) { //TODO: actual starts with
  return strstr(buf, str);
}

void clientCycle() {
  mprint("Enter IP of Host (format ##.###.###.###): ");
  mprint("Enter Port of Host: ");
  mprint("Enter Username: ");

        // read response, thread: optional save
        // start another go if disconnect? sync request

}

void* listenKeyboard() {
  //parse, add parsed string to messages, thread: optional save
  //pass on to host or client pass
  return NULL;
}

void parseAdd(char message) {

}

void addToMessages(char username, char content, char time) {
  struct messageT *message = malloc(sizeof *message);
  strncpy(message->username, &username, USERNAMELEN);
  strncpy(message->content, &content, CONTENTLEN);
  strncpy(message->time, &time, TIMELEN);
  P(&arrayMutex);
  newestMessage++;
  //free...?
  recentMessages[newestMessage] = *message;
  printRecentMessages();
  V(&arrayMutex);
  if (SAVE) {
    pthread_t saveThread;
    Pthread_create(&saveThread, NULL, saveToChatlog, &message);
  }
}

void* saveToChatlog(void *message) {
  if (VERBOSE) mlog(">>> creating thread: save");
  Pthread_detach(pthread_self());
  P(&fileMutex);
  //save
  V(&fileMutex);
  if (VERBOSE) mlog("<<< exiting thread: save");
  return NULL; //auto reap
}

void handleSync() {

}

void connectionTimer() {

}

char* grabUsername(struct connT user) {
  return "";
}

void printRecentMessages() {

}

void mlog(char* str) {
  printf("|||||||||||||||||| %s\n", str);
}

void mprint(char* str) {
  printf("%s", str);
}
