#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "opts.h"

#define CFG_FNAME "config"


bool show_icons = true;
bool show_dot_files = false;
bool color = true;
bool p_long = false;

static char * default_config_dir = NULL;
static char * csc_config_path = NULL;
static char * csc_config_file = NULL;


bool check_config(void) {
	char * csc_dir = "cscroll";
	int csc_len = 7;

	// find default config directory
	char * xdg_config = getenv("XDG_CONFIG_HOME");
	char * cfg_path = NULL;
	if (!xdg_config) {
		char * home = getenv("HOME");
		cfg_path = malloc(strlen(home) + 9 + csc_len);
		sprintf(cfg_path, "%s/.config", home);
	} else {
		cfg_path = malloc(strlen(xdg_config) + 1 + csc_len);
		strcpy(cfg_path, xdg_config);
	}

	// set default config path variable
	default_config_dir = malloc(strlen(cfg_path) + 1);
	strcpy(default_config_dir, cfg_path);

	// set cscroll config dir path
	strcat(cfg_path, "/"); strcat(cfg_path, csc_dir);
	csc_config_path = malloc(strlen(cfg_path) + 1);
	strcpy(csc_config_path, cfg_path);

	// set path to cscroll config file
	csc_config_file = malloc(strlen(csc_config_path) + 8);
	sprintf(csc_config_file, "%s/"CFG_FNAME, csc_config_path);

	free(cfg_path);

	struct stat st_buf;
	// either cscroll config file or dir does not exist
	if (stat(csc_config_path, &st_buf) < 0 ||
		stat(csc_config_file, &st_buf) < 0) return false;
	return true;
}


void create_config(void) {
	struct stat st_buf;
	// set with mode rwx------
	if (stat(default_config_dir, &st_buf) < 0)
		mkdir(default_config_dir, 0700);

	// set with mode rw-rw-r-
	if (stat(csc_config_path, &st_buf) < 0)
		mkdir(csc_config_path, 0664);

	// create config file if it doesnt exist
	if (stat(csc_config_file, &st_buf) < 0) {
		FILE * fp = fopen(csc_config_file, "w");
		if (fp) fclose(fp);
	}
}


void read_config(void) {
	FILE * fp = fopen(csc_config_file, "r");

	bool done = false;
	// read by line
	while (!done) {
		char line[256] = {0}; // 0 initialize
		char * lp = line;

		// read line from file into buffer
		int c;
		while ((c = fgetc(fp)) != '\n') {
			if (c == EOF) {
				done = true;
				break;
			}
			*lp++ = c;
		}

		if (!*line) break;


		char * var = line;
		// go past leading white space
		while (*var && isspace(*var)) var++;

		int n = 0;

		// add null terminator to var
		char * val = strchr(var, '=');
		while (isspace(val[n - 1])) n--;
		val[n] = 0;
		val++;

		// remove trailing white space after '='
		while (*val && *val != '=' && isspace(*val)) val++;

		bool bool_val = false;
		if (!strcmp(val, "true")) bool_val = true;

		if (!strcmp(var, ICONS_VAR)) show_icons = bool_val;
		else if (!strcmp(var, COLOR_VAR)) color = bool_val;
		else if (!strcmp(var, LONG_VAR)) p_long = bool_val;
	}

	fclose(fp);
}


void terminate_opts(void) {
	free(default_config_dir);
	free(csc_config_path);
	free(csc_config_file);
}
