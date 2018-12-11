#include <stdio.h>
#include <string.h>
#include "csapp.h"

struct messageT {
  char username[32];
  char content[2048];
};

struct connT {
  char username[32];
  char ip[14];
  char port[5];
};

void clientCycle();
void hostCycle();
void* listenKeyboard();
void handleSync();
void connectionTimer();
void mlog(char str);
void* createSconnThread(void* tempc);
void* saveToChatlog(void* message);
int syncRequest();
int loadHistory(char* fileName);
char* grabUsername(struct connT user);


int VERBOSE = 0, HEADLESS = 0, SAVE = 0, LOAD = 0;
//I know all caps for variables defies
                  // convention, but it makes my code easier to visually parse
Sem_t fileMutex;
Sem_t arrayMutex;
struct message[40] recentMessages;
int newestMessage = 0;
struct connT[] connections;
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
      if (loadHistory(argv[i+1])) {
        LOAD = 1;
        i++;
      }
    }
  }

  if (argc > 1 && strcmp(argv[1], "-host") == 0) {
    if (VERBOSE) mlog("|||||| starting as host\n");
    hostCycle();
  } else {
    if (VERBOSE) mlog ("|||||| starting as client\n");
    clientCycle();
  }
}

void hostCycle() {
  int listenfd;
  socklen_t clientlen;
  pthread_t sconnThread;
  struct sockaddr_storage clientaddr;

  mprint("Enter Desired Port Number: ");
  char portNum[5];
  fgets(portNum, 5, stdin);
  mprint("Enter Username: ");
  fgets(self.username, 32, stdin);

  Pthread_create(&keyThread, NULL, listenKeyboard, NULL);
  listenfd = Open_listenfd(portNum);

  while (1) {
    clientlen = sizeof(clientaddr);
    int *connfd = Malloc(sizeof(*connfd));
    *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    if (VERBOSE) mlog("|||||| connection attempted\n");
    Pthread_create(&sconnThread, NULL, handleSconn, connfd);
      /*accept incoming connections*/
  }

}

void* handleSconn(void* tempc) {
  int connfd = *((int *) tempc);
  if (VERBOSE) mlog(">>>>>> creating thread: sconn\n");
  Pthread_detach(pthread_self());
  Free(tempc);
  ///handle scones
  //parse, addtomessages

  //pass on to others in list of other name
  if (VERBOSE) mlog("<<<<<< exiting thread: sconn\n");
  return NULL; //auto reap
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
}

void addToMessages(char username, char content) {
  struct messageT *message = malloc(sizeof *message);
  strncpy(message.username, username, NUMMMM);
  strncpy(message.content, content, NUMMMMMMM);
  P(&arrayMutex);
  newestMessage++;
  if (recentMessages[newestMessage] != NULL) {
    Free(recentMessages[newestMessage]);
  }
  recentMessages[newestMessage] = message;
  printRecentMessages();
  V(&arrayMutex);
  if (SAVE) {
    pthread_t saveThread;
    Pthread_create(&saveThread, NULL, saveToChatlog, &message)
  }
}

void* saveToChatlog(void *message) {
  if (VERBOSE) mlog(">>>>>> creating thread: save");
  Pthread_detach(pthread_self());
  P(&fileMutex);
  //save
  V(&fileMutex);
  if (VERBOSE) mlog("<<<<<< exiting thread: save");
  return NULL; //auto reap
}

int syncRequest() {

}

void handleSync() {

}

void connectionTimer() {

}

char* grabUsername(struct connT user) {

}
