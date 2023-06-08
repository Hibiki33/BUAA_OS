#include <lib.h>

int flag[256];

void usage(void) {
    printf("usage: declare [-xr] [NAME [=VALUE]]\n");
    exit();
}

int main(int argc, char **argv) {
    int r;
    char *val;
    int id;

    ARGBEGIN {
        default:
            usage();
        case 'x':
        case 'r':
            flag[(u_char)ARGC()]++;
            break;
    } ARGEND

    char *s = argv[argc - 1];
    int shellid = (s[0] - '0') * 100 + (s[1] - '0') * 10 + (s[2] - '0');

    if (flag['x']) {
        id = 0;
    } else {
        id = shellid;
    }

    if (argc == 1) {
        char buf[4096];
        if ((r = syscall_get_env_value(NULL, shellid, buf)) < 0) {
            printf("declare wrong: %d\n", r);
        }
        printf("%s\n", buf);
    } else if (argc == 2) {
        if ((r = syscall_declare_env_value(argv[0], "", id, flag['r'])) < 0) {
            printf("environment value %s is read only: %d\n", argv[0], r);
        }
    } else if (argc == 3) {
        val = argv[1];
        if (val[0] == '=') {
            val++;
            if ((r = syscall_declare_env_value(argv[0], val, id, flag['r'])) < 0) {
                printf("environment value %s is read only: %d\n", argv[0], r);
            }
        } else {
            usage();
        }
    } else {
        usage();
    }

    return 0;
}
