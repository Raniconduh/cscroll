#ifndef ICONS_H
#define ICONS_H

#define ICON_DIR "\uf74a"
#define ICON_GEAR "\uf013"

#define ICON_APK "\ue70e"
#define ICON_EXEC ICON_GEAR
#define ICON_SHELL "\uf489"
#define ICON_C "\ue61e"
#define ICON_CPP "\ue61d"
#define ICON_CS "\uf81a"
#define ICON_CSS "\ue749"
#define ICON_FSHARP "\ue7a7"
#define ICON_XML "\uf81a"
#define ICON_RB "\ue739"
#define ICON_LUA "\ue620"
#define ICON_GIT "\ue702"
#define ICON_GO "\ue626"
#define ICON_HTML "\ue736"
#define ICON_JAVA "\ue738"
#define ICON_JS "\ue781"
#define ICON_JSON "\ue60b"
#define ICON_PY "\ue73c"
#define ICON_SCALA "\ue737"
#define ICON_MD "\ue609"
#define ICON_VIM "\ue62b"

#define ICON_AUDIO "\uf722"
#define ICON_VIDEO "\uf72a"
#define ICON_GENERIC "\uf15b"
#define ICON_ARCHIVE "\uf1c6"

struct icon_pair {
	char * ext;
	char * icon;
};

struct icon_pair icons[] = {
	/* A */
	{"apk", ICON_APK},
	{"asm", ICON_GEAR},
	/* B */
	{"bash", ICON_SHELL},
	{"bin", ICON_EXEC},
	/* C */
	{"c", ICON_C},
	{"c++", ICON_CPP},
	{"cc", ICON_CPP},
	{"class", ICON_JAVA},
	{"cs", ICON_CS},
	{"css", ICON_CSS},
	/* D */
	/* E */
	/* F */
	{"f#", ICON_FSHARP},
	{"fish", ICON_SHELL},
	/* G */
	{"git", ICON_GIT},
	{"go", ICON_GO},
	/* H */
	{"h", ICON_C},
	{"hh", ICON_CPP},
	{"hpp", ICON_CPP},
	{"htm", ICON_HTML},
	{"html", ICON_HTML},
	/* I */
	/* J */
	{"java", ICON_JAVA},
	{"js", ICON_JS},
	{"json", ICON_JSON},
	/* K */
	/* L */
	{"lua", ICON_LUA},
	/* M */
	{"md", ICON_MD},
	/* N */
	/* O */
	{"o", ICON_EXEC},
	{"out", ICON_EXEC},
	/* P */
	{"py", ICON_PY},
	{"pyc", ICON_PY},
	{"pyd", ICON_PY},
	{"pyo", ICON_PY},
	/* Q */
	/* R */
	{"rb", ICON_RB},
	/* S */
	{"scala", ICON_SCALA},
	{"sh", ICON_SHELL},
	{"so", ICON_EXEC},
	/* T */
	/* U */
	/* V */
	{"vim", ICON_VIM},
	{"vimrc", ICON_VIM},
	/* W */
	/* X */
	{"xml", ICON_XML},
	/* Y */
	/* Z */
	{"zsh", ICON_SHELL},
};

#define n_icons (sizeof(icons)/sizeof(icons[0]))

int icmp(const void * a, const void * b) {
	return strcmp(*(const char **)a, ((const struct icon_pair*)b)->ext);
}

#endif /* ICONS_H */
