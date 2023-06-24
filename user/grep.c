#include <lib.h>

int fd;
char buf[512], lbuf[512];

void grep(char *pattern) {
	int n;
	char *plb = lbuf;
	int slen = strlen(pattern);

	while ((n = read(fd, buf, sizeof(buf))) > 0) {
		for (int i = 0; i < n; i++) {
			if (buf[i] == '\n'){
				*plb++ = '\n';
				*plb = '\0';
				char *pp = pattern, *tpl = lbuf;
				int len = 0;
				for (; tpl < plb; tpl++) {
					if (*pp == *tpl) {
						pp++;
						len++;
						if (len == slen) {
							printf(lbuf);
							break;
						}
					}
					else {
						pp = pattern;
						tpl = tpl - len;
						len = 0;
					}
				}
				plb = lbuf;
			}
			else {
				*plb++ = buf[i];
			}
		}
	}
	if (n < 0) {
		user_panic("error reading file: %d", n);
	}
}

int main(int argc, char **argv) {

	if(argc == 2) {
		fd = 0;		
	} else {
		fd = open(argv[2], O_RDONLY);
	}

	grep(argv[1]);

	return 0;
}