#include "9cc.h"
#include <stdlib.h>
#include <string.h>

char *create_string_copy(const char *src, int len) {
  if (!src || len < 0) {
    return NULL;
  }
  char *name = calloc(len + 1, sizeof(char));
  if (!name) {
    error(ERR_MSG_FAILED_TO_ATTACH_MEMORIES);
  }
  strncpy(name, src, len);
  name[len] = '\0';
  return name;
}
