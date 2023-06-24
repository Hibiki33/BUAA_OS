#include <lib.h>

void usage(void) 
{
	debugf("usage: mkdir [dirname]\n");
    exit();
}

int main(int argc, char **argv) {
    int i;
    int f;

    if (argc == 1) {
        usage();
    } else {
        for (i = 1; i < argc; i++) {
            // if the dir already exists
            if ((f = open(argv[i], O_RDONLY)) >= 0) {
                user_panic("path: %s already exists\n", argv[i]);
            } 
            if ((f = create(argv[1], FTYPE_DIR)) < 0) {
                user_panic("error create directory %s: %d\n", argv[i], f);
            }
        }
    }

    return 0;
}