#include <stdio.h>
#include <file.h>

cstring_t * readFile(const char *pathname)
{
    cstring_t * cs = NULL;
    FILE *fp = fopen(pathname, "rb");
    if (!fp) return NULL;

    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    cstring_new_len(str, len);
    size_t total = 0;
    while (total != len) {
        size_t size = fread(str->str, len, 1, fp);
        if (size <= 0) break;
        str->len += size*len;
    }
    
    fclose(fp);

    cs = str;

    return cs;
}

size_t writeFile(const char *pathname, void *data, int len)
{
    cstring_t * cs = NULL;
    FILE *fp = fopen(pathname, "wb+");
    if (!fp) return 0;

    fwrite(data, len, 1, fp);
    fclose(fp);

    return len;
}

size_t appendFile(const char *pathname, void *data, int len)
{
    cstring_t * cs = NULL;
    FILE *fp = fopen(pathname, "ab");
    if (!fp) return 0;

    fwrite(data, len, 1, fp);
    fclose(fp);

    return len;
}