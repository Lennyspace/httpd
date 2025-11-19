#include "config.h"
#include <getopt.h>
#include "../utils/string/string.h"
#include <string.h>
#include <stdlib.h>






struct config *parse_configuration(int argc, char *argv[]){
	 struct config* conf=malloc(sizeof(struct config));
	 conf->servers=malloc(sizeof(struct server_config));

	 struct option options[] = { { "pid_file", required_argument, NULL, 'a' },
                                { "log", required_argument, NULL, 'b' },
                                { "log_file", required_argument, NULL, 'c' },
                                { "server_name", required_argument, NULL, 'd' },
                                { "port", required_argument, NULL, 'e' },
                                { "ip", required_argument, NULL, 'f' },
                                { "root_dir", required_argument, NULL, 'g' },
                                { "default_file", required_argument, NULL, 'h' },
                                { NULL, 0, NULL, 0 } };
	 conf->servers->default_file=malloc(11);
	 conf->servers->default_file=strdup("index.htlm");
			 		
	 conf->log=true;
	 conf->log_file=NULL;
	 char ch;

	 while ((ch = getopt_long(argc, argv, "", options, NULL)) != -1){
		 switch(ch)
		 {
			 case 'a':
				 conf->pid_file=strdup(optarg);
				break;
			 case 'b':
				 if(strcmp(optarg,"true")){
					 conf->log=true;
				 }
				 conf->log=false;
				break;
			 case 'c':
				 conf->log_file=strdup(optarg);
				break;
			 case 'd':
				 conf->servers->server_name=string_create(optarg,strlen(optarg));
				break;
			 case 'e':
			 	conf->servers->port=strdup(optarg);
				break;
			 case 'f':
			 	conf->servers->ip=strdup(optarg);
				break;
			 case 'g':
			 	conf->servers->root_dir=strdup(optarg);
				break;
			 case 'h':
			 	conf->servers->default_file=strdup(optarg);
				break;
			 default :
				config_destroy(conf);
				break;
		 }
	 }
	 if(conf->log_file==NULL){
		conf->log=true;
		return NULL;
	 }
	 return conf;
}



void config_destroy(struct config *config){
	free(config->pid_file);
	free(config->log_file);

	free(config->servers->port);
	free(config->servers->ip);
	free(config->servers->root_dir);
	free(config->servers->default_file);

	string_destroy(config->servers->server_name);
	free(config->servers);
	free(config);
}
