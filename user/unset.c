#include <lib.h>

void usage(void) {
    printf("usage: unset NAME\n");
    exit();
}

int main(int argc, char **argv) {
    int r;
    int i;

    char *s = argv[argc - 1];
    int shellid = (s[0] - '0') * 100 + (s[1] - '0') * 10 + (s[2] - '0');
    
    if (argc == 2) {
        usage();
    } else {
        for (i = 1; i < argc - 1; i++) {
            if ((r = syscall_unset_env_value(argv[i], shellid)) < 0) {
                printf("environment value %s isn't declared or is readonly\n", argv[i]);
            }
        }
    }

    return 0;
}