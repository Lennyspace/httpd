#include "config/config.h"
#include "daemon/daemon.h"
#include "server/server.h"

int main(int argc, char **argv) {
  struct config *conf = parse_configuration(argc, argv);
  if (!conf) {
    return 2;
  }
  int ret = 0;
  if (conf->daemon == NO_OPTION) {
    start_serv(conf->servers->ip, conf->servers->port, conf->servers->root_dir,
               conf->servers->default_file);

  } else if (conf->daemon == START) {
    ret = daemon_start(conf);
  } else if (conf->daemon == STOP) {
    ret = daemon_stop(conf);
  } else if (conf->daemon == RESTART) {
    ret = daemon_restart(conf);
  }
  config_destroy(conf);
  return ret;
}
