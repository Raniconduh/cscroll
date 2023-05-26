#ifndef _ICONS_H
#define _ICONS_H

#include "type.h"

#define ICON_DIR "\uf413"
#define ICON_GEAR "\uf013"

#define ICON_APK "\ue70e"
#define ICON_EXEC ICON_GEAR
#define ICON_SHELL "\uf489"
#define ICON_C "\ue61e"
#define ICON_CPP "\ue61d"
#define ICON_CS "\ue648"
#define ICON_CSS "\ue749"
#define ICON_FSHARP "\ue7a7"
#define ICON_XML "\ue60e"
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

#define ICON_AUDIO "\uf1c7"
#define ICON_VIDEO "\uf1c8"
#define ICON_IMAGE "\uf1c5"
#define ICON_GENERIC "\uf016"
#define ICON_ARCHIVE "\uf1c6"

struct icon_pair icons[] = {
	{"1",   ICON_ARCHIVE},
	{"3g2", ICON_VIDEO},
	{"3gp", ICON_VIDEO},
	{"7z",  ICON_ARCHIVE},
	{"7zip",ICON_ARCHIVE},
	/* A */
	{"a",   ICON_ARCHIVE},
	{"aac", ICON_AUDIO},
	{"ac3", ICON_AUDIO},
	{"ai",  ICON_IMAGE},
	{"aif", ICON_AUDIO},
	{"alz", ICON_ARCHIVE},
	{"amv", ICON_VIDEO},
	{"apk", ICON_APK},
	{"asec",ICON_APK},
	{"asf", ICON_VIDEO},
	{"asm", ICON_GEAR},
	{"avi", ICON_VIDEO},
	/* B */
	{"bash",ICON_SHELL},
	{"bin", ICON_EXEC},
	{"bmp", ICON_IMAGE},
	{"bz2", ICON_ARCHIVE},
	/* C */
	{"c",   ICON_C},
	{"c++", ICON_CPP},
	{"cc",  ICON_CPP},
	{"cbr", ICON_ARCHIVE},
	{"cda", ICON_AUDIO},
	{"class",ICON_JAVA},
	{"cpgz",ICON_ARCHIVE},
	{"cs",  ICON_CS},
	{"cso", ICON_ARCHIVE},
	{"css", ICON_CSS},
	/* D */
	{"dar", ICON_ARCHIVE},
	{"dbz", ICON_ARCHIVE},
	{"deb", ICON_ARCHIVE},
	{"drc", ICON_VIDEO},
	{"dz",  ICON_ARCHIVE},
	/* E */
	{"ear", ICON_ARCHIVE},
	{"eps", ICON_IMAGE},
	/* F */
	{"f#", ICON_FSHARP},
	{"f4a", ICON_AUDIO},
	{"f4b", ICON_AUDIO},
	{"f4p", ICON_VIDEO},
	{"f4v", ICON_VIDEO},
	{"flv", ICON_VIDEO},
	{"fish", ICON_SHELL},
	/* G */
	{"gif", ICON_IMAGE},
	{"gifv",ICON_VIDEO},
	{"gip", ICON_ARCHIVE},
	{"git", ICON_GIT},
	{"go",  ICON_GO},
	{"gz",  ICON_ARCHIVE},
	/* H */
	{"h",   ICON_C},
	{"h264",ICON_VIDEO},
	{"heif",ICON_IMAGE},
	{"hh",  ICON_CPP},
	{"hpp", ICON_CPP},
	{"htm", ICON_HTML},
	{"html",ICON_HTML},
	{"htmlz",ICON_ARCHIVE},
	/* I */
	{"ico", ICON_IMAGE},
	{"igz", ICON_ARCHIVE},
	{"ipa", ICON_ARCHIVE},
	/* J */
	{"jar", ICON_JAVA},
	{"java",ICON_JAVA},
	{"jpeg",ICON_IMAGE},
	{"jpg", ICON_IMAGE},
	{"js",  ICON_JS},
	{"json",ICON_JSON},
	/* K */
	/* L */
	{"lua", ICON_LUA},
	{"lz",  ICON_ARCHIVE},
	{"lz4", ICON_ARCHIVE},
	/* M */
	{"m2ts",ICON_VIDEO},
	{"m2v", ICON_VIDEO},
	{"m4a", ICON_AUDIO},
	{"m4p", ICON_AUDIO},
	{"m4v", ICON_VIDEO},
	{"maff",ICON_ARCHIVE},
	{"md",  ICON_MD},
	{"mid", ICON_AUDIO},
	{"midi",ICON_AUDIO},
	{"mkv", ICON_VIDEO},
	{"mng", ICON_IMAGE},
	{"mov", ICON_VIDEO},
	{"mp3", ICON_AUDIO},
	{"mp4", ICON_VIDEO},
	{"mpeg",ICON_VIDEO},
	{"mpg", ICON_VIDEO},
	{"mpq", ICON_ARCHIVE},
	{"mts", ICON_VIDEO},
	{"mxf", ICON_VIDEO},
	/* N */
	{"npk", ICON_ARCHIVE},
	{"nsv", ICON_VIDEO},
	{"nxz", ICON_ARCHIVE},
	/* O */
	{"o",   ICON_EXEC},
	{"ogg", ICON_AUDIO},
	{"ogv", ICON_VIDEO},
	{"out", ICON_EXEC},
	/* P */
	{"pbm", ICON_IMAGE},
	{"pgm", ICON_IMAGE},
	{"pkg", ICON_ARCHIVE},
	{"png", ICON_IMAGE},
	{"pnm", ICON_IMAGE},
	{"ppm", ICON_IMAGE},
	{"ps",  ICON_IMAGE},
	{"psd", ICON_IMAGE},
	{"pup", ICON_ARCHIVE},
	{"py",  ICON_PY},
	{"pyc", ICON_PY},
	{"pyd", ICON_PY},
	{"pyo", ICON_PY},
	{"pz",  ICON_ARCHIVE},
	{"pzip",ICON_ARCHIVE},
	/* Q */
	{"qt",  ICON_VIDEO},
	/* R */
	{"rar", ICON_ARCHIVE},
	{"rb",  ICON_RB},
	{"rm",  ICON_VIDEO},
	{"rmvb",ICON_VIDEO},
	{"roq", ICON_VIDEO},
	{"rpa", ICON_ARCHIVE},
	{"rpm", ICON_ARCHIVE},
	/* S */
	{"sar", ICON_ARCHIVE},
	{"scala",ICON_SCALA},
	{"sh",  ICON_SHELL},
	{"shar",ICON_ARCHIVE},
	{"sis", ICON_ARCHIVE},
	{"sisx",ICON_ARCHIVE},
	{"so",  ICON_EXEC},
	{"svg", ICON_IMAGE},
	{"svi", ICON_VIDEO},
	/* T */
	{"tar", ICON_ARCHIVE},
	{"tgz", ICON_ARCHIVE},
	{"tif", ICON_IMAGE},
	{"tiff",ICON_IMAGE},
	{"ts",  ICON_VIDEO},
	{"txz", ICON_ARCHIVE},
	/* U */
	{"uha", ICON_ARCHIVE},
	/* V */
	{"vim", ICON_VIM},
	{"vimrc",ICON_VIM},
	{"viv", ICON_VIDEO},
	{"vob", ICON_VIDEO},
	{"vsix",ICON_ARCHIVE},
	/* W */
	{"wav", ICON_AUDIO},
	{"webm",ICON_VIDEO},
	{"webp",ICON_IMAGE},
	{"wma", ICON_AUDIO},
	{"wmv", ICON_VIDEO},
	/* X */
	{"xap", ICON_ARCHIVE},
	{"xml", ICON_XML},
	{"xz",  ICON_ARCHIVE},
	{"xzm", ICON_ARCHIVE},
	{"xzn", ICON_ARCHIVE},
	/* Y */
	{"yuv", ICON_VIDEO},
	/* Z */
	{"z",   ICON_ARCHIVE},
	{"zab", ICON_ARCHIVE},
	{"zi",  ICON_ARCHIVE},
	{"zip", ICON_ARCHIVE},
	{"zlib",ICON_ARCHIVE},
	{"zsh", ICON_SHELL},
	{"zst", ICON_ARCHIVE},
	{"zstd",ICON_ARCHIVE},
	{"zxp", ICON_ARCHIVE},
	{"zz",  ICON_ARCHIVE},
};

#define n_icons (sizeof(icons)/sizeof(icons[0]))

int icmp(const void * a, const void * b) {
	return strcmp(*(const char **)a, ((const struct icon_pair*)b)->ext);
}

#endif /* _ICONS_H */
