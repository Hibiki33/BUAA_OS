#include <lib.h>

int flag_n;
void find(char *path, char *name);
void dirfind(char *path, char *name);

void usage(void) {
    debugf("find [path] -name [fname]\n");
    exit();
}

char *strcat(char *dst, const char *src) {
	char *res = dst;
	while (*dst) {
		dst++;
	}
	while ((*dst++ = *src++) != 0) {
	}
	return res;
}

void find(char *path, char *name) {
    int r;
    struct Stat st;

    if ((r = stat(path, &st)) < 0) {
		user_panic("stat %s: %d", path, r);
    }

    if (!st.st_isdir) {
		user_panic("path: %s is not a directory\n", path);
    }

    dirfind(path, name);
}

void dirfind(char *path, char *name) {
    int fd;
    int n;
    struct File f;

    if ((fd = open(path, O_RDONLY)) < 0) {
        user_panic("open %s: %d", path, fd);
    }
    // printf("%s\n", path);
    while ((n = readn(fd, &f, sizeof(f))) == sizeof(f)) {
        if (f.f_name[0]) {
            if (f.f_type == FTYPE_DIR) {
                char newpath[128];
                strcpy(newpath, path);
                int len = strlen(newpath);
                if (newpath[len - 1] != '/') {
                    newpath[len] = '/';
                    newpath[len + 1] = 0;
                }
                dirfind(strcat(newpath, f.f_name), name);
            } else if (f.f_type == FTYPE_REG) {
                if (strcmp(f.f_name, name) == 0) {
                    char newpath[128];
                    strcpy(newpath, path);
                    int len = strlen(newpath);
                    if (newpath[len - 1] != '/') {
                        newpath[len] = '/';
                        newpath[len + 1] = 0;
                    }
                    printf("%s\033[31m%s\033[0m\n", newpath, name);
                }
            }
        }
    }
}

int main(int argc, char **argv) {

    if (argv[1][0] == '-') {
        if (strcmp(argv[1], "-name") == 0) {
            flag_n = 1;
        //} else if (strcmp(argv[1], "-type") == 0) {
        //    flag_t = 1;
        } else {
            usage();
        }
        for (int i = 2; i < argc; i++) {
            find("./", argv[i]);
        }
    } else if (argv[2][0] == '-') {
        if (strcmp(argv[2], "-name") == 0) {
            flag_n = 1;
        //} else if (strcmp(argv[2], "-type") == 0) {
        //    flag_t = 1;
        } else {
            usage();
        }
        for (int i = 2; i < argc; i++) {
            if (argv[1][0] != '.' && argv[1][0] != '/') {
                char temp[128] = {'.', '/'};
                find(strcat(temp, argv[1]), argv[i]);
            } else {
                find(argv[1], argv[i]);
            }
        }
    } else {
        usage();
    }
    return 0;
}
