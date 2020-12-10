#ifndef __FILE_H
#define __FILE_H

#include <cstring.h>

/**
 * @brief 读取文件内容
 * @param  pathname         需要读取的文件路径名称
 * @return cstring_t*       读取数据存放的字符串结构体指针，读取失败返回NULL
 */
cstring_t * readFile(const char *pathname);

/**
 * @brief 写入文件内容
 * @param  pathname         要写入的文件
 * @param  data             写入的数据
 * @param  len              写入数据的长度
 * @return size_t           真实写入的大小
 */
size_t writeFile(const char *pathname, void *data, int len);

#endif