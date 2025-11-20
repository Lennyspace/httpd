#include "config.h"
#include "../utils/string/string.h"
#include <bits/getopt_core.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct config *parse_configuration(int argc, char *argv[]) {
  struct config *conf = malloc(sizeof(struct config));
  conf->pid_file = NULL;
  conf->log = true;
  conf->log_file = NULL;
  conf->daemon = NO_OPTION;

  conf->servers = malloc(sizeof(struct server_config));
  conf->servers->server_name = NULL;
  conf->servers->port = NULL;
  conf->servers->ip = NULL;
  conf->servers->root_dir = NULL;
  conf->servers->default_file = strdup("index.html");

  struct option options[] = {{"pid_file", required_argument, NULL, 'a'},
                             {"log", required_argument, NULL, 'b'},
                             {"log_file", required_argument, NULL, 'c'},
                             {"server_name", required_argument, NULL, 'd'},
                             {"port", required_argument, NULL, 'e'},
                             {"ip", required_argument, NULL, 'f'},
                             {"root_dir", required_argument, NULL, 'g'},
                             {"default_file", required_argument, NULL, 'h'},
                             {"daemon", required_argument, NULL, 'i'},
                             {NULL, 0, NULL, 0}};

  bool pid_file = false;
  bool server_name = false;
  bool has_port = false;
  bool has_ip = false;
  bool has_root_dir = false;

  char ch;
  while ((ch = getopt_long(argc, argv, "", options, NULL)) != -1) {
    switch (ch) {
    case 'a':
      conf->pid_file = strdup(optarg);
      pid_file = true;
      break;
    case 'b':
      if (strcmp(optarg, "true") == 0) {
        conf->log = true;
      } else {
        conf->log = false;
      }
      break;
    case 'c':
      conf->log_file = strdup(optarg);
      break;
    case 'd':
      conf->servers->server_name = string_create(optarg, strlen(optarg));
      server_name = true;
      break;
    case 'e':
      conf->servers->port = strdup(optarg);
      has_port = true;
      break;
    case 'f':
      conf->servers->ip = strdup(optarg);
      has_ip = true;
      break;
    case 'g':
      has_root_dir = true;
      conf->servers->root_dir = strdup(optarg);
      break;
    case 'h':
      free(conf->servers->default_file);
      conf->servers->default_file = strdup(optarg);
      break;
    case 'i':
      if (strcmp(optarg, "start") == 0) {
        conf->daemon = START;
      } else if (strcmp(optarg, "stop")) {
        conf->daemon = STOP;
      } else if (strcmp(optarg, "restart")) {
        conf->daemon = RESTART;

      } else {
        config_destroy(conf);
        return NULL;
      }
      break;
    default:
      config_destroy(conf);
      return NULL;
      break;
    }
  }
  if (!pid_file || !server_name || !has_port || !has_ip || !has_root_dir) {
    config_destroy(conf);
    return NULL;
  }
  return conf;
}

void config_destroy(struct config *config) {
  if (config->pid_file)
    free(config->pid_file);
  if (config->log_file)
    free(config->log_file);

  if (config->servers->port)
    free(config->servers->port);

  if (config->servers->ip)
    free(config->servers->ip);
  if (config->servers->root_dir)
    free(config->servers->root_dir);
  free(config->servers->default_file);

  if (config->servers->server_name)
    string_destroy(config->servers->server_name);
  free(config->servers);
  free(config);
}
