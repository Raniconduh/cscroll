#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "info.h"
#include "opts.h"
#include "var.h"
#include "io.h"

#define CFG_FNAME "config"


bool show_icons = true;
bool show_dot_files = false;
bool color = true;
bool p_long = false;
bool oneshot = false;
bool show_dot_dirs = false;
struct opener_t opener = {NULL, 0};

uint32_t custom_colors[11];

uint32_t dir_color     = COLOR_DEFAULT;
uint32_t link_color    = COLOR_DEFAULT;
uint32_t exec_color    = COLOR_DEFAULT;
uint32_t sock_color    = COLOR_DEFAULT;
uint32_t fifo_color    = COLOR_DEFAULT;
uint32_t unknown_color = COLOR_DEFAULT;
uint32_t reg_color     = COLOR_DEFAULT;
uint32_t blk_color     = COLOR_DEFAULT;
uint32_t media_color   = COLOR_DEFAULT;
uint32_t archive_color = COLOR_DEFAULT;

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
		cfg_path = malloc(strlen(homedir) + 9 + csc_len + 1);
		sprintf(cfg_path, "%s/.config", homedir);
	} else {
		cfg_path = malloc(strlen(xdg_config) + 2 + csc_len);
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
	// set with mode rwxr-xr-x
	if (stat(default_config_dir, &st_buf) < 0)
		mkdir(default_config_dir, 0755);

	// set with mode rwxr-xr-x
	if (stat(csc_config_path, &st_buf) < 0)
		mkdir(csc_config_path, 0755);

	// create config file if it doesnt exist
	if (stat(csc_config_file, &st_buf) < 0) {
		FILE * fp = fopen(csc_config_file, "w");
		if (fp) fclose(fp);
	}
}


enum var_stat parse_var(char * var) {
	char * line = var;
	// go past leading white space
	while (*var && isspace(*var)) var++;

	int n = 0;

	// add null terminator to var
	char * val = strchr(var, '=');
	if (!val) return VAR_STAT_NOEQ;

	while (val + n > line && isspace(val[--n]));
	val[n + 1] = 0;
	val++;

	// remove trailing white space after '='
	while (*val && *val != '=' && isspace(*val)) val++;

	void * ptr_val = NULL;
	size_t vlen = strlen(val);

	if (!strcmp(val, "true")) {
		ptr_val = VAR_TRUE;
	} else if (!strcmp(val, "false")) {
		ptr_val = VAR_FALSE;
	} else if (vlen > 2 && val[0] == '"' && val[vlen - 1] == '"') {
		// empty strings not supported
		val++;

		// -2 to compensate for inc on prev line & for strlen
		val[vlen - 2] = 0;
		ptr_val = val;
	} else return VAR_STAT_NOTYPE;

	var_set(var, ptr_val);
	return VAR_STAT_OK;
}


void read_config(void) {
	FILE * fp = fopen(csc_config_file, "r");

	size_t l = 1;
	bool done = false;
	// read by line
	char line[256];

	while (!done) {
		short len = 0; // only needs to go up to 255

		// read line from file into buffer
		int c;
		while ((c = fgetc(fp)) != '\n') {
			if (len >= 255 || c == EOF) {
				done = c == EOF; // only done at EOF
				break;
			}
			line[len++] = c;
		}

		line[len] = '\0';
		if (*line) switch (parse_var(line)) {
			case VAR_STAT_NOEQ:
				display_info(INFO_WARN,
				             "Config line %lu: Missing '=' in variable definition", l);
				break;
			case VAR_STAT_NOTYPE:
				display_info(INFO_WARN,
				             "Config line %lu: Missing or unknown type", l);
				break;
			default:
			case VAR_STAT_OK:
				break;
		}

		l++;
	}

	fclose(fp);
}


void terminate_opts(void) {
	free(default_config_dir);
	free(csc_config_path);
	free(csc_config_file);
}


void generate_colors(void) {
	custom_colors[COLOR_DIR]     = dir_color;
	custom_colors[COLOR_LINK]    = link_color;
	custom_colors[COLOR_EXEC]    = exec_color;
	custom_colors[COLOR_SOCK]    = sock_color;
	custom_colors[COLOR_FIFO]    = fifo_color;
	custom_colors[COLOR_UNKNOWN] = unknown_color;
	custom_colors[COLOR_FILE]    = reg_color;
	custom_colors[COLOR_BLOCK]   = blk_color;
	custom_colors[COLOR_MEDIA]   = media_color;
	custom_colors[COLOR_ARCHIVE] = archive_color;
}
