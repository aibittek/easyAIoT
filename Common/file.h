#ifndef __FILE_H
#define __FILE_H

#include <cstring.h>

cstring_t * readFile(const char *pathname);
size_t writeFile(const char *pathname, void *data, int len);

#endif