#include "../../include/common/utils.h"

#include <stdlib.h>
#include <string.h>

i64 hash(const char *str) {
	i64 hash = 5381;
	int c;

	while ((c = *str++)) {
		// hash * 33 + c
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

char *safestrgen(char *str, i32 len) {
	char *g = malloc(len + 1);
	if (!g) return NULL;
	memcpy(g, str, len);
	g[len] = '\0';
	return g;
}