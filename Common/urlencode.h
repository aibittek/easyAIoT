#ifndef __URL_ENCODE_H
#define __URL_ENCODE_H

/**
 * @brief url网址编码，便于网络传输
 * @param  in               编码前网址
 * @param  out              编码后网址
 */
void urlencode(char in[], char out[]);

/**
 * @brief url网址解码，便于查看
 * @param  in               解码前网址
 * @param  out              解码后网址
 */
void urldecode(char in[], char out[]);

#endif