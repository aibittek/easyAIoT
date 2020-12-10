#include <stdio.h>
#include <string.h>

int hex2dec(char c)
{
    if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if ('A' <= c && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else
    {
        return -1;
    }
}

char dec2hex(short int c)
{
    if (0 <= c && c <= 9)
    {
        return c + '0';
    }
    else if (10 <= c && c <= 15)
    {
        return c + 'A' - 10;
    }
    else
    {
        return -1;
    }
}

void urlencode(char in[], char out[])
{
    int i = 0;
    int len = strlen(in);
    int res_len = 0;

    for (i = 0; i < len; ++i)
    {
        char c = in[i];
        if (('0' <= c && c <= '9') ||
            ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            c == '.' || c == '/' ||
            c == '?' || c == '&' || c == '=' ||
            c == '-' || c == '_')
        {
            out[res_len++] = c;
        }
        else if (c == ' ') {
            out[res_len++] = '+';
        }
        else
        {
            int j = (short int)c;
            if (j < 0)
                j += 256;
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            out[res_len++] = '%';
            out[res_len++] = dec2hex(i1);
            out[res_len++] = dec2hex(i0);
        }
    }
    out[res_len] = '\0';
}

void urldecode(char in[], char out[])
{
    int i = 0;
    int len = strlen(in);
    int res_len = 0;

    for (i = 0; i < len; ++i)
    {
        char c = in[i];
        if (c != '%')
        {
            if (c == '+') out[res_len++] = ' ';
            else out[res_len++] = c;
        }
        else
        {
            char c1 = in[++i];
            char c0 = in[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            out[res_len++] = num;
        }
    }
    out[res_len] = '\0';
}

void testUrlEncode()
{
    char in[100] = "http://www.example.com/vad?date=Thu, 31 Thu 1999 15:04:39 GMT";
    char out[100];

    urlencode(in, out);
    printf("%s\n", out);

    urldecode(out, in);
    printf("%s\n", in);
}