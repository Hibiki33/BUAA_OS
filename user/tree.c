#include <lib.h>
int flag[256];
int filecnt;
int dircnt;

void treedir(char *, int);
void tree1(char *, u_int, char *, int);

void tree(char *path) {
	int r;
    struct Stat st;
    
    if ((r = stat(path, &st)) < 0) {
		user_panic("stat %s: %d", path, r);
    }
    
    if (!st.st_isdir) {
		debugf("%s [error opening dir]\n", path);
        debugf("0 directories, 0 files\n");
        exit();
    }
	
    // dircnt += 1;
    debugf("%s\n", path);
    treedir(path, 0);
}

void treedir(char *path, int step) {
    int fd;
    int n;
    struct File f;

    if ((fd = open(path, O_RDONLY)) < 0) {
        user_panic("open %s: %d", path, fd);
    }

    while ((n = readn(fd, &f, sizeof(f))) == sizeof(f)) {
        if (f.f_name[0]) {
            tree1(path, f.f_type == FTYPE_DIR, f.f_name, step + 1);
        }
    }

    if (n > 0) {
		user_panic("short read in directory %s", path);
	}
	if (n < 0) {
		user_panic("error reading directory %s: %d", path, n);
	}
}

void tree1(char *path, u_int isdir, char *name, int step) {
    char *sep;

    if (flag['d'] && !isdir) {
        return;
    }

    for (int i = 0; i < step - 1; i++) {
        debugf("    ");
    }
    debugf("|-- ");

    if (path[0] && path[strlen(path) - 1] != '/') {
		sep = "/";
	} else {
		sep = "";
	}

    if (flag['f'] && path) {
		debugf("%s%s", path, sep);
    }
    debugf("%s\n", name);

    if (isdir) {
        dircnt += 1;

        char newpath[256];
        strcpy(newpath, path);
        int namelen = strlen(name);
        int pathlen = strlen(path);

        if (strlen(sep) != 0) {
            newpath[pathlen] = '/';
            for (int i = 0; i < namelen; i++) {
                newpath[pathlen + i + 1] = name[i];
            }
            newpath[pathlen + namelen + 1] = 0;
            treedir(newpath, step);
        } else {
            for (int i = 0; i < namelen; i++) {
                newpath[pathlen + i] = name[i];
            }
            newpath[pathlen + namelen] = 0;
            treedir(newpath, step);
        }
    } else {
        filecnt += 1;
    }
}

void usage(void) 
{
	debugf("usage: tree [-adf] [directory...]\n");
    exit();
}

int main(int argc, char **argv)
{
	int i;
    ARGBEGIN
    {
        default:
        	usage();
        case 'a':
        case 'd':
        case 'f':
        	flag[(u_char)ARGC()]++;
    		break;
    }
    ARGEND
        
    if (argc == 0) {
        tree("./");
        if (flag['d']) {
            debugf("\n%d directories\n", dircnt);
        } else {
            debugf("\n%d directories, %d files\n", dircnt, filecnt);
        }
	} else {
		for (i = 0; i < argc; i++)
        {
            tree(argv[i]);
            if (flag['d']) {
                debugf("\n%d directories\n", dircnt);
            } else {
                debugf("\n%d directories, %d files\n", dircnt, filecnt);
            }
		}
    }
	debugf("\n");
	return 0;
}