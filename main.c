#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inih/ini.h"

#include "qwk2wwiv.h"



static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
	struct config *pconfig = (struct config*)user;
	 
	if (strcasecmp("general", section) == 0) {
		if (strcasecmp("archive", name) == 0) {
			pconfig->archivecmd = strdup(value);
		} else if (strcasecmp("unarchive", name) == 0) {
			pconfig->unarchivecmd = strdup(value);
		} else if (strcasecmp("qwkpath", name) == 0) {
			pconfig->qwkpath = strdup(value);
		} else if (strcasecmp("reppath", name) == 0) {
			pconfig->reppath = strdup(value);
		} else if (strcasecmp("temppath", name) == 0) {
			pconfig->temppath = strdup(value);
		} else if (strcasecmp("wwivin", name) == 0) {
			pconfig->wwivin = strdup(value);
		} else if (strcasecmp("wwivout", name) == 0) {
			pconfig->wwivout = strdup(value);
		} else if (strcasecmp("yourwwivnode", name) == 0) {
			pconfig->yourwwivnode = atoi(value);
		} else if (strcasecmp("theirwwivnode", name) == 0) {
			pconfig->theirwwivnode = atoi(value);
		}
	} else if (strcasecmp("type2sub", section) == 0) {
		if (pconfig->subcount == 0) {
			pconfig->subs = (struct sub **)malloc(sizeof(struct sub *));
		} else {
			pconfig->subs = (struct sub **)realloc(pconfig->subs, sizeof(struct sub *) * (pconfig->subcount + 1));
		}
		
		pconfig->subs[pconfig->subcount] = (struct sub *)malloc(sizeof(struct sub));
		
		pconfig->subs[pconfig->subcount]->type = strdup(name);
		pconfig->subs[pconfig->subcount]->num = atoi(value);
		pconfig->subcount++;
	}
	
	return 1;
}

void usage() {
	printf("Usage: qwk2wwiv [command] [system.ini]\n");
	printf("where: command is one of: q2w or w2r\n");
}

int main(int argc, char **argv) {
	struct config configuration;
	
	configuration.subcount = 0;
	
	if (argc < 3) {
		usage();
		exit(-1);
	}

	if (ini_parse(argv[2], handler, &configuration) <0) {
		printf("Unable to load configuration ini (%s)!\n", argv[2]);
		exit(-1);
	}

	
	if (strcasecmp(argv[1], "q2w") == 0) {
		q2w(&configuration);
	} else if (strcasecmp(argv[1], "w2r") == 0) {\
		
	} else {
		usage();
		exit(-1);
	}
	
	return 0;
}
