#include <lib.h>

void usage(void) 
{
	printf("usage: history\n");
    exit();
}

int main(int argc, char **argv) {
    int r;
    int f;
    int linecnt = 0;
    int newline = 1;
    char buf;

    if (argc != 1) {
        usage();
    } else {
        if ((r = open("/.history", O_RDONLY)) < 0) {
            user_panic("can't open file .history: %d\n", r);
        }
        f = r;

        while ((r = read(f, &buf, 1)) == 1) {
            if (newline) {
                printf("%5d  ", linecnt++);
                newline = 0;
            } 
            printf("%c", buf);

            if (buf == '\n') {
                newline = 1;
            }
        }

    }

    return 0;
}