#include <lib.h>

int rm_d, rm_r;
int flag[256];

void rmdir(char *);
void rm1(char *);

void rm(char *path) {
	int r;
	struct Stat st;

	if ((r = stat(path, &st)) < 0) {
		user_panic("Cannot remove %s: no such file or directory\n", path);
		// return;
	}
	if (st.st_isdir) {
		rmdir(path);
	} else {
		rm1(path);
	}
}

char *strcat(char *dst, const char *src) {
	char *address = dst;
	while (*dst) {
		dst++;
	}
	while ((*dst++ = *src++) != 0) {
	}
	return address;
}

void rmdir(char *path) {
	int fd, n;
	struct File f;
	char buf[4096];
	buf[0] = '\0';
	if (!rm_d && !rm_r) {
		user_panic("Cannot remove %s: is a directory\n", path);
		// return;
	}

	if ((fd = open(path, O_RDONLY)) < 0) {
		user_panic("Cannot remove '%s': no such file or directory\n", path);
		// return;
	}
	while ((n = readn(fd, &f, sizeof f)) == sizeof f) {
		if (f.f_name[0]) {
			if (!rm_r) {
				user_panic("Cannot remove %s: directory is not empty\n", path);
				// return;
			}
			buf[0] = 0;
			strcat(buf, path);
			strcat(buf, "/");			
			strcat(buf, f.f_name);
			rm(buf);
		}
	}
	if (n > 0) {
		user_panic("Short read in directory %s", path);
	}
	if (n < 0) {
		user_panic("Error reading directory %s: %d", path, n);
	}
	rm1(path);
}

void rm1(char *path) {
	int r = remove(path);
	if (r != 0) {
		user_panic("Cannot remove %s: no such file or directory\n", path);
		// return;
	}
	// debugf("Successfully removed %s\n", path);
	// return;
}


void usage(void) {
	debugf("Usage: rm [-dhr]... file(s)...\n");
	exit();
}

int main(int argc, char **argv) {
	int i;

	rm_d = 0;
	rm_r = 0;
	ARGBEGIN {
		case 'h':
			usage();
			break;
		case 'd':
			rm_d = 1;
			break;
		case 'r':
			rm_r = 1;
			break;
	}
	ARGEND

	if (argc == 0) {
		usage();
	} else {
		for (i = 0; i < argc; i++) {
            if (argv[i][0] == '/' || argv[i][0] == '.') {
                rm(argv[i]);
            } else {
                char path[128] = {'.', '/'};
                strcat(path, argv[i]);
                rm(path);
            }
		}
	}
	// debugf("\n");
	return 0;
}
