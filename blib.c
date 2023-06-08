#include <blib.h>

size_t strlen(const char *s) {
	size_t i = 0;
	while (s[i++]) {}
	return i - 1;
}

char *strcpy(char *dst, const char *src) {
	char *res = dst;
	while ((*dst++ = *src++) != 0) {}
	return res;	
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	char *res = dst;
	while (*dst != 0) {
		dst++;
	}
	while((*dst++ = *src++) != 0) {}
	return res;
}

int strcmp(const char *s1, const char *s2) {
	while (*s1 == *s2 && *s1 && *s2) {
		s1++;
		s2++;
	}
	return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	char *src = (char *)s;
	while (n--) {
		*src++ = c;
	}
	return s;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	char *str1 = (char *)s1;
	char *str2 = (char *)s2;
	while (n--) {
		if (*str1 != *str2) {
			return *str1 - *str2;
		}
		if (!(*str1)) {
			break;
		}
		str1++;
		str2++;
	}
	return 0;
}
