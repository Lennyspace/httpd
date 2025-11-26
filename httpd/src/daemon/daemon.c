#include "daemon.h"
#include "../server/server.h"
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
int daemon_start(struct config *conf) {
  FILE *f = fopen(conf->pid_file, "r");
  if (f != NULL) {
    int val = 0;
    if (fscanf(f, "%d", &val) == 1 && val > 0) {
      if (kill(val, 0) == 0) {
        fclose(f);
        return 1;
      }
    }
    fclose(f);
  }

  pid_t pid = fork();
  if (pid > 0) {
    f = fopen(conf->pid_file, "w+");
    fprintf(f, "%d\n", pid);
    fclose(f);
    return 0;

  } else {
    close(0);
    close(1);
    close(2);
    start_serv(conf->servers->ip, conf->servers->port, conf->servers->root_dir,
               conf->servers->default_file);
    exit(0);
  }
}

int daemon_stop(struct config *conf) {
  FILE *f = fopen(conf->pid_file, "r");
  if (!f) {
    return 0;
  }
  int pid = 0;
  if (fscanf(f, "%d", &pid) == 1 && pid > 0) {
    kill(pid, SIGINT);
  }
  fclose(f);
  unlink(conf->pid_file);
  return 0;
}

int daemon_restart(struct config *conf) {
  daemon_stop(conf);
  return daemon_start(conf);
}
