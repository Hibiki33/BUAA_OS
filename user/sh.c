#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

// .history has been created
static int hisinit;
// lines of .history
static int hislen;
// offset of lines in .history
static int hisoffsets[128];

int shellid;
// for env_value
//extern int shellid;

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *	   - ';' for ; (spilt cmds).
 *	   - '&' for & (coprocesses).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	if (*s == '\"') 
	{
		*p1 = ++s;
		while (*s && *s != '\"')
		{
			s++;
		}
		*p2 = s;
		*s++ = 0;
		return 'w';
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

void savecmd(const char *s) {
	int r;
	int fd;

	if (!hisinit) {
		if (create("/.history", FTYPE_REG) < 0) {
			// debugf("create .history failed\n");
			// exit();
			// debugf(".history already exists: %d\n", r);
		}
	}

	if ((r = open("/.history", O_WRONLY | O_APPEND)) < 0) {
		debugf("can't open file .history: %d\n", r);
		exit();
	}
	fd = r;

	if ((r = write(fd, s, strlen(s))) != strlen(s)) {
		user_panic("write error .history: %d\n", r);
	}

	// it is need to write a '\n'
	if ((r = write(fd, "\n", 1)) != 1) {
		user_panic("write error .history: %d\n", r);
	}

	if (!hisinit) {
		hisinit = 1;
		hisoffsets[hislen++] = strlen(s) + 1;
	} else {
		hisoffsets[hislen] = hisoffsets[hislen - 1] + strlen(s) + 1;
		hislen++;
	}

	close(fd);
}

void gethis(int index, char *cmd) {
	int r;
	int fd;
	char tempbuf[4096];
	int cmdlen = 0;

	if ((r = open("/.history", O_RDONLY)) < 0) {
		debugf("can't open file .history: %d\n", r);
		exit();
	}
	fd = r;

	if (index != 0) {
		if ((r = readn(fd, tempbuf, hisoffsets[index - 1])) != hisoffsets[index - 1]) {
			user_panic("can't read file .history: %d", r);
		}
		cmdlen = hisoffsets[index] - hisoffsets[index - 1];
	} else {
		cmdlen = hisoffsets[index];
	}

	if ((r = readn(fd, cmd, cmdlen)) != cmdlen) {
		user_panic("can't read file .history: %d", r);
	}

	close(fd);

	cmd[cmdlen - 1] = 0;

	// for debug
	// fd = open("/motd", O_WRONLY | O_APPEND);
	// fprintf(fd, "%s ", cmd);
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		//debugf("%d\n", c);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}

			// for env_value
			if (t[0] == '$') {
				t++;
				char buf[32];
				if ((r = syscall_get_env_value(t, shellid, buf)) < 0) {
					printf("environment value %s doesn't exist", t);
				}
				strcpy(t, buf);
			}

			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			if ((r = open(t, O_RDONLY)) < 0)
			{
				debugf("< open failed\n");
				exit();
			}
			fd = r;
			dup(fd, 0);
			close(fd);
			//user_panic("< redirection not implemented");
			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			if ((r = open(t, O_WRONLY)) < 0) {
				// debugf("> open failed\n");
				// exit();
				if (create(t, FTYPE_REG) < 0) {
					debugf("> open and create failed\n");
					exit();
				}
				r = open(t, O_WRONLY);
			}
			fd = r;
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");
			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			pipe(p);
			if ((*rightpipe = fork()) == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe);
			} else {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			//user_panic("| not implemented");
			break;
		case ';':
			if ((*rightpipe = fork()) == 0) {
				return argc;
			} else {
				wait(*rightpipe);
				return parsecmd(argv, rightpipe);
			}
			break;
		case '&':
			if ((r = fork()) == 0) {
				return argc;
			} else {
				return parsecmd(argv, rightpipe);
			}
			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int argc = parsecmd(argv, &rightpipe);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

	if (strcmp(argv[0], "declare") == 0) {
		char temp[4] = {
			(char)(shellid / 100) + '0',
			(char)(shellid % 100 / 10) + '0',
			(char)(shellid % 10) + '0', 0};
		argv[argc++] = temp;
		argv[argc] = 0;
	}

	if (strcmp(argv[0], "unset") == 0) {
		char temp[4] = {
			(char)(shellid / 100) + '0',
			(char)(shellid % 100 / 10) + '0',
			(char)(shellid % 10) + '0', 0};
		argv[argc++] = temp;
		argv[argc] = 0;
	}

	if (strcmp(argv[0], "cd") == 0) {
		int r;
		// printf("%d\n", argc);
		if (argc == 1) {
			if ((r = chdir("/")) < 0) {
				printf("cd failed: %d\n", r);
				exit();
			}
			printf("/\n");
		} else {
			if (argv[1][0] == '/') {
				if ((r = chdir(argv[1])) < 0) {
					printf("cd failed: %d\n", r);
					exit();
				}	
			} else {
				// Get curent direct path.
				char path[128];
				if ((r = getcwd(path)) < 0) {
					printf("cd failed: %d\n", r);
					exit();
				}

				// Parse the target indirect path into direct.
				pathcat(path, argv[1]);

				// Confirm the path exists and is a directory.
				if ((r = open(path, O_RDONLY)) < 0) {
					printf("path %s doesn't exist: %d\n", path, r);
					exit();
				}
				close(r);
				struct Stat st;
				if ((r = stat(path, &st)) < 0) {
					user_panic("stat %s: %d", path, r);
				}
				if (!st.st_isdir) {
					printf("path %s is not a directory\n", path);
					exit();
				}

				if ((r = chdir(path)) < 0) {
					printf("cd failed: %d\n", r);
					exit();
				}
			}
		}
		return;
	}

	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

// UP: 	  \033[A
// DOWN:  \033[B
// Left:  \033[D
// Right: \033[C
#define MOVEUP(y) printf("\033[%dA", (y))
#define MOVEDOWN(y) printf("\033[%dB", (y))
#define MOVELEFT(y) printf("\033[%dD", (y))
#define MOVERIGHT(y) printf("\033[%dC",(y))

int dirkey() {
	int r;
	char temp1, temp2;
	if ((r = read(0, &temp1, 1)) != 1) {
		if (r < 0) {
			debugf("read error: %d\n", r);
		}
		exit();
	}
	if ((r = read(0, &temp2, 1)) != 1) {
		if (r < 0) {
			debugf("read error: %d\n", r);
		}
		exit();
	}
	if (temp1 == '[') {
		switch (temp2) {
			case 'A':
				return 1;
			case 'B':
				return 2;
			case 'D':
				return 3;
			case 'C': 
				return 4;
		}
	}
	return 0;
}

void readline(char *buf, u_int n) {
	int r;
	int len = 0;
	int i = 0;
	int j = 0;
	char temp;

	char curcmd[1024];
	int index = hislen;

	//for (int i = 0; i < n; i++) {
	while (i < n) {
		if ((r = read(0, &temp, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}

		// if read '\b', refresh the <stdin>
		if (temp == '\b' || temp == 0x7f) {
			if (i > 0) {
				if (i == len) {
					buf[--i] = 0;
					printf("\033[D \033[D");
				} else {
					for (j = i - 1; j < len - 1; j++) {
						buf[j] = buf[j + 1];
					}
					buf[len - 1] = 0;
					MOVELEFT(i--);
					printf("%s ", buf);
					MOVELEFT(len - i);
				}
				len -= 1;
			}
		} else if (temp == '\033') {
			switch (dirkey())
			{
				case 1://;
					MOVEDOWN(1);
					if (index == hislen) {
						buf[len] = 0;
						strcpy(curcmd, buf);
						// for debug
						// int fff = open("/motd", O_WRONLY | O_APPEND);
						// fprintf(fff, "%s", curcmd);
					}
					if (index > 0) {
						index--;
					}
					if (i > 0) {
						MOVELEFT(i);
					}
					gethis(index, buf);
					printf("%s", buf);
					if (strlen(buf) < len) {
						for (j = 0; j < len - strlen(buf); j++) {
							printf(" ");
						}
						printf("\033[%dD", len - strlen(buf));
					}
					len = i = strlen(buf);
					break;
				case 2:
					// shouldn't move up
					// MOVEUP(1); 
					if (index == hislen - 1) {
						index++;
						strcpy(buf, curcmd);
						// for debug
						// printf(" %s %s", buf, curcmd);
					} else if (index == hislen) {
						break;
					} else {
						index++;
						gethis(index, buf);
					}
					if (i > 0) {
						MOVELEFT(i);
					}
					printf("%s", buf);
					if (strlen(buf) < len) {
						for (j = 0; j < len - strlen(buf); j++) {
							printf(" ");
						}
						printf("\033[%dD", len - strlen(buf));
					}
					len = i = strlen(buf);
					break;
				case 3:
					if (i > 0) {
						i -= 1;
					} else {
						MOVERIGHT(1);
					}
					break;
				case 4:
					if (i < len) {
						i += 1;
					} else {
						MOVELEFT(1);
					}
					break;
				default: 
					break;
			}
		} else if (temp == '\r' || temp == '\n') {
			buf[len] = 0;
			return;
		} else {
			if (i == len) {
				buf[i++] = temp;
			} else {		// i < len
				for (j = len; j > i; j--) {
					buf[j] = buf[j - 1];
				}
				buf[i] = temp;
				buf[len + 1] = 0;
				MOVELEFT(++i);
				printf("%s", buf);
				MOVELEFT(len - i + 1);
			}
			len += 1;
		}

		if (len >= n) {
			break;
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");

	if ((r = syscall_create_shell_id()) < 0) {
		printf("created shell id failed: %d\n", r);
	}
	shellid = r;
	printf("SHELL ID: %d\n", shellid);

	if ((r = chdir("/")) < 0) {
		printf("created root path failed: %d\n", r);
	}

	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[0], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[0], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		savecmd(buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
