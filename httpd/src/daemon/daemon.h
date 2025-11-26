#ifndef DAEMON_H
#define DAEMON_H

#include "../config/config.h"
int daemon_start(struct config *conf);

int daemon_stop(struct config *conf);
int daemon_restart(struct config *conf);
#endif /* DAEMON_H */
