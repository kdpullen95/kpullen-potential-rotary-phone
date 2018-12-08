#include <stdio.h>
#include <string.h>
#include "csapp.h"

struct message {
  char username[32];
  char content[2048];
}

int VERBOSE = 0, HEADLESS = 0, SAVE = 0, LOAD = 0;
//I know all caps for variables defies
                  // convention, but it makes my code easier to visually parse
Sem_t mutex;
struct message[] recentMessages;

int main(int argc, char **argv)
{

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
    if (VERBOSE) mlog("|||| starting as host");
  } else {
    if (VERBOSE) mlog ("|||| starting as client");
  }
}

void hostCycle() {
  mprint("Enter Desired Port Number: ");
  mprint("Enter Username: ");

  while (1) {
      //accept incoming connections
      //spawn thread: parse, print parsed string, thread: optional save
      //pass on to others in list of other name
  }

}

void clientCycle() {
  mprint("Enter IP + Port of Host (format ##.###.###.###:####): ");
  mprint("Enter Username: ");

  while(1) {
        // read response, thread: optional save
        // start another go if disconnect? sync request
  }

}

void listenKeyboard() {
  //parse, print parsed string, thread: optional save
  //pass on to host or client pass
}

int saveToChatlog() {

}
