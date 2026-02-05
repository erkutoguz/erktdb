#include "../../include/common/utils.h"

i64 hash(const char *str) {
	i64 hash = 5381;
	int c;

	while ((c = *str++)) {
		// hash * 33 + c
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}