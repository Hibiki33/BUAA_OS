#include <lib.h>

/* Overview:
 *  Change the cur_path to path.
 */
int chdir(char *path) {
    return syscall_set_cur_path(path);
}

/* Overview:
 *  Get the cur_path.
 */
int getcwd(char *buf) {
    return syscall_get_cur_path(buf);
}

/* Overview:
 *  Concatenate the direct path with indirect path.
 */
void pathcat(char *path, const char *suffix) {
    int pre_len = strlen(path);

	if (suffix[0] == '.') {
		suffix += 2;
	}
	int suf_len = strlen(suffix);

	if (pre_len != 1) {
		path[pre_len++] = '/';
	}
	for (int i = 0; i < suf_len; i++) {
		path[pre_len + i] = suffix[i];
	}

	path[pre_len + suf_len] = 0;
}
