#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <dirent.h>
#include <signal.h>


#define PWD "PWD"

#define BLUE 1
#define HBLUE 2
#define CYAN 3
#define HCYAN 4
#define GREEN 5
#define HGREEN 6
#define MAGENTA 7
#define HMAGENTA 8
#define YELLOW 9
#define HYELLOW 10
#define RED 11
#define HRED 12
#define WHITE 13
#define HWHITE 14

void help(void);
void listitems(void);
void sighandler(int);

bool isdir(const char *);
bool isfifo(const char *);
bool isblock(const char *);
bool islink(const char *);
bool isunknown(const char *);
bool isreg(const char *);

void print_file(char *, bool);

void cdback();
void enter(char *);

// current working directory
char * cwd;

// array of files in directory
char **  dirContents;
// number of files in directory
size_t dirCount = 0;


int main ( int argc, char ** argv ) {
	// cwd is pwd if no args given
	if (argc < 2) {
		// this is a safer way of storing the home dir
		cwd = malloc(sizeof(char) * (strlen(getenv(PWD)) + 3));
		strcpy(cwd, getenv(PWD));
		strcat(cwd, "/"); // append the '\0' with strcat
	//help menu
	} else if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
		help();
		exit(0);
	} else {
		cwd = malloc(sizeof(char) * (strlen(argv[1]) + 3));
		strcpy(cwd, argv[1]);

		if (cwd[strlen(cwd) - 1] != '/') {
			strcat(cwd, "/");
		}

	}

	initscr();
	// this is here so that endwin() is called after init
	signal(SIGINT, sighandler);
	signal(SIGSEGV, sighandler);

	curs_set(0);

	// init color pairs
	start_color();
	// odd pairs are : color fg on black bg
	// even pairs are: black fg on color bg
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);   // blue fg on black bg
    init_pair(HBLUE, COLOR_BLACK, COLOR_BLUE);   // black fg on blue bg

    init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);   // cyan fg on black bg
    init_pair(HCYAN, COLOR_BLACK, COLOR_CYAN);   // black fg on cyan bg

    init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);  // green fg on black bg
    init_pair(HGREEN, COLOR_BLACK, COLOR_GREEN);  // black fg on green bg

    init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);// magenta fg on black bg
    init_pair(HMAGENTA, COLOR_BLACK, COLOR_MAGENTA);// black fg on magenta bg

    init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK); // yellow fg on black bg
    init_pair(HYELLOW, COLOR_BLACK, COLOR_YELLOW);// black fg on yellow bg

    init_pair(RED, COLOR_RED, COLOR_BLACK);   // red fg on black bg
    init_pair(HRED, COLOR_BLACK, COLOR_RED);   // black fg on red bg

    init_pair(WHITE, COLOR_WHITE, COLOR_BLACK); // white fg on black gb
    init_pair(HWHITE, COLOR_BLACK, COLOR_WHITE); // black fg on white bg

	dirContents = malloc(sizeof(char *) + 1); // allocate something to realloc
	dirContents[0] = malloc(1);   // allocate something to free

	size_t cursor = 0;

	listitems(); // preliminary list to save on disk io
	// driver loop -- ig bools are defined in nh
	while (true) {
		erase();

		printw("\n%s\n\n", cwd); // print path at top

		for (size_t i = 0; i < dirCount; i++) {
			if (cursor == i) 
				print_file(dirContents[i], true);
			else
				print_file(dirContents[i], false);
		}

		attron(COLOR_PAIR(0));
		printw("\n%lu/%lu\n", cursor + 1, dirCount);
		refresh();
		
		char c = getch();
		if (c == 'q') goto done; // 'q' command for quit
		if (c == 'j') cursor += 1 ? cursor < dirCount - 1 : 0; // down key
		else if (c == 'k') cursor -= 1 ? cursor > 0 : 0; // up key
		else if (c == 'h') {
			cdback();
			listitems();
			cursor = 0;
		}
		else if (c == 'l') {
			enter(dirContents[cursor]);
			listitems();
			cursor = 0;
		}

		refresh();
	}

	done:
	endwin();

	free(dirContents);
	free(cwd);

	return 0;
}


// no idea how this works but it does
static int cmp(const void * a, const void * b) {
	return strcmp(*(const char**)a, *(const char**)b);
}

void listitems(void) {
	struct dirent * dirItem;
	DIR * dir = opendir(cwd);

	// free every single index in the list
	// this will prevent future memory leaks
	for (long long i = dirCount; i >= 0; i--) {
		free(dirContents[dirCount]);
	}

	dirCount = 0;
	while ((dirItem = readdir(dir)) != NULL) {
		// do not append '.' and '..' directories
		if (!strcmp(dirItem->d_name, "..") || !strcmp(dirItem->d_name, ".")) {
			continue;
		}
		size_t namelen = strlen(dirItem->d_name);
		// allocate memory to store all the pointers
		dirContents = realloc(dirContents, sizeof(char*) * (dirCount + 2));
		// allocate memory for each index to store the file name
		dirContents[dirCount] = malloc(sizeof(char) * (namelen + 3));

		strcpy(dirContents[dirCount], dirItem->d_name);

	
		// may not work on all machines so this needs to be moved
		// to something more universal like stat
		switch (dirItem->d_type) {
			case DT_DIR:  // is a dir
				strcat(dirContents[dirCount], "/");
				break;
			case DT_FIFO: // is a fifo
				strcat(dirContents[dirCount], "|");
				break;
			case DT_LNK:  // is a symbolic link
				strcat(dirContents[dirCount], "@");
				break;
			case DT_BLK:  // is a block device
				strcat(dirContents[dirCount], "#");
				break;
			case DT_UNKNOWN:  // idk what this file is
				strcat(dirContents[dirCount], "?");
				break;
		}

		dirCount++;
	}
	
	closedir(dir);

	// sort the files
	qsort(dirContents, dirCount, sizeof(char*), cmp);
}


void enter(char * dir) {
	if (!isdir(dir)) return;
	// allocate enough memory to append name of dir
	cwd = realloc(cwd, sizeof(char) * (strlen(cwd) + strlen(dir) + 2));
	// since dir will already end with '/',
	// it is not necessary to append it to end of cwd
	strcat(cwd, dir);
}


void cdback() {
	if (!strcmp(cwd, "/") || !strcmp(cwd, "//")) return;
	size_t clen = strlen(cwd);
	cwd[--clen] = '\0';
	while (cwd[--clen] != '/') cwd[clen] = '\0';
	cwd = realloc(cwd, sizeof(char) * (clen + 2));
}


bool isdir(const char * fname) {
	return fname[strlen(fname) - 1] == '/';
}

bool isfifo(const char * fname) {
	return fname[strlen(fname) - 1] == '|';
}

bool isblock(const char * fname) {
	return fname[strlen(fname) - 1] == '#';
}

bool islink(const char * fname) {
	return fname[strlen(fname) - 1] == '@';
}

bool isunknown(const char * fname) {
	return fname[strlen(fname) - 1] == '?';
}

bool isreg(const char * fname) {
	if (isdir(fname) ||
		   	isfifo(fname) ||
			isblock(fname)||
			islink(fname) ||
			isunknown(fname)) return true;
	else return false;
}


void print_file(char * fname, bool highlight) {
	bool c = false; // c flag :)
	unsigned int cp;
	if (highlight) {
		if (isdir(fname)) cp = HBLUE;
		else if (isfifo(fname) || isblock(fname)) cp = HYELLOW;
		else if (islink(fname)) cp = HCYAN;
		else if (isunknown(fname)) cp = HRED;
		else cp = HWHITE;
	} else {
		if (isdir(fname)) cp = BLUE;
		else if (isfifo(fname) || isblock(fname)) cp = YELLOW;
		else if (islink(fname)) cp = CYAN;
		else if (isunknown(fname)) cp = RED;
		else cp = WHITE;
	}

	size_t flen = strlen(fname);

	attron(COLOR_PAIR(cp));
	if (cp == WHITE || cp == HWHITE) printw("%s\n", fname);
	else {
		c = true; // set c flag :)
		printw("%.*s", flen - 1, fname); // print up to the last character
	}
	attroff(COLOR_PAIR(cp));
	if (c) printw("%c\n", fname[flen - 1]);
}


void help(void) {
	printf("cscroll\n"
			"Options:\n"
			"  -h, --help\t\tShow this screen and exit\n");
}


void sighandler(int signo) {
	if (signo == SIGINT) {
		endwin();
		puts("Exited on SIGINT");
		exit(1);
	} else if (signo == SIGSEGV) {
		endwin();
		puts("Segmentation Fault");
		exit(2);
	}
}

