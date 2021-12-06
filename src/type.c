#include <string.h>

#include "type.h"

char * media_exts[] = {
	"3g2", "3gp", "aac", "ac3", "ai", 
	"aif", "amv", "asf", "avi", "bmp", 
	"cda", "drc", "eps", "f4a", "f4b", 
	"f4p", "f4v", "flv", "gif", "gifv", 
	"h264", "heif", "ico", "jpeg", "jpg", 
	"m2ts", "m2v", "m4a", "m4p", "m4v", 
	"mid", "midi", "mkv", "mng", "mov", 
	"mp3", "mp4", "mpeg", "mpg", "mts", 
	"mxf", "nsv", "ogg", "ogv", "pbm", 
	"pgm", "png", "pnm", "ppm", "ps", 
	"psd", "qt", "rm", "rmvb", "roq", 
	"svg", "svi", "tif", "tiff", "ts", 
	"viv", "vob", "wav", "webm", "webp", 
	"wma", "wmv", "yuv" 
};

char * archive_exts[] = {
	"7zip", "a", "alz", "apk", "asec",
	"bz2", "cbr", "cmp", "cpgz", "cso",
	"dar", "dbz", "deb", "dz", "ear",
	"exe", "gip", "gz", "htmlz", "igz",
	"ipa", "jar", "lz", "lz4", "maff",
	"mpq", "npk", "nxz", "pkg", "pup",
	"pz", "pzip", "rar", "rpa", "sar",
	"shar", "sis", "sisx", "tar", "tgz",
	"txz", "uha", "vsix", "xap", "xz",
	"xzm", "xzn", "z", "zab", "zi",
	"zip", "zlib", "zst", "zxp", "zz"
};


char * get_ext(char * s) {
	char * ns = strrchr(s, '.');
	if (!ns) return NULL;
	return ns + 1;
}

int scmp(const void * a, const void * b) {
	return strcmp(*(const char**)a, *(const char**)b);
}

void lowers(char * s) {
	for (char * p = s; *p; p++)
		// A to Z
		if (*p >= 65 && *p <= 90)
			*p += 32; // convert upper to lower
}
