#ifndef _COMMON_H_
#define _COMMON_H_

char *strStr(const char *str1, const char *str2) {
	if (*str2) {
		while (*str1) {
			for (int i = 0; *(str1 + i) == *(str2 + i); i++) {
				if (!*(str2 + i + 1)) {
					return (char *)str1;
				}
			}
			str1++;
		}
		return NULL;
	}
	else {
		return (char *)str1;
	}
}

#endif // !_COMMON_H_
