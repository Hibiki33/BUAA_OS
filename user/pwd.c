#include <lib.h>

void usage(void) {
    debugf("usage: pwd\n");
    exit();
}

int main(int argc, char **argv) {
    int r;
    char buf[128];

    if (argc != 1) {
		usage();
	} else {
		if ((r = getcwd(buf)) < 0) {
            debugf("get path failed: %d\n", r);
            exit();
        }
        debugf("%s\n", buf);
	}

    return 0;
}