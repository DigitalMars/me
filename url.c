
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/*********************************
 * Determine if c is a valid URL character.
 */
int isURLchar(char c)
{
    if (isalnum(c))
	return 1;
    switch (c)
    {
        case '-':
	case '_':
	case '?':
        case '=':
	case '%':
	case '&':
        case '/':
	case '+':
	case '#':
        case '~':
        case '.':
        case ':':
	    return 1;

	default:
	    return 0;
    }
}

/******************************
 * Determine if string s of length is the start of a URL.
 * Returns:
 *	0 means not a URL, >0 gives the length of the URL
 */
size_t isURL(const char *s, size_t length)
{
    /* Must start with one of:
     *  http://
     *  https://
     */

    if (length < 9 || s[6] != '/')
        return 0;

    //writefln("isURL(%s)", s);

    if (!((s[0] == 'h' || s[0] == 'H') &&
          (s[1] == 't' || s[1] == 'T') &&
          (s[2] == 't' || s[2] == 'T') &&
          (s[3] == 'p' || s[3] == 'P')))
	return 0;

    size_t i;
    if (s[4] == ':' && s[5] == '/')
	i = 7;
    else if ((s[4] == 's' || s[4] == 'S') && s[5] == ':' && s[7] == '/')
	i = 8;
    else
        return 0;

    size_t lastdot;
    for (; i < length; i++)
    {
        auto c = s[i];
        if (isalnum(c))
            continue;
        if (c == '-' || c == '_' || c == '?' ||
            c == '=' || c == '%' || c == '&' ||
            c == '/' || c == '+' || c == '#' ||
            c == '~')
            continue;
        if (c == '.')
        {
            lastdot = i;
            continue;
        }
        break;
    }
    if (!lastdot)
        return 0;

    return i;
}


/****************************************************
 * Determine if index is in a URL or not.
 */
int inURL(const char *s, size_t length, size_t index)
{
    if (length < 9 || !isURLchar(s[index]))
	return 0;

    size_t i;
    size_t end = length - 9;
    if (index < end)
	end = index + 1;
    for (i = 0; i < end; ++i)
    {
	size_t j = isURL(s + i, length - i);
	if (j)
	{
	    if (i <= index && index < i + j)
		return 1;
	    i = i + j - 1;
	}
    }
    return 0;
}


/************************************************
 * Determine URL that index is in.
 * Return malloc'd string, NULL if not in a URL.
 */

char *getURL(const char *s, size_t length, size_t index)
{
    size_t i;
    for (i = 0; i <= index; ++i)
    {
	size_t j = isURL(s + i, length - i);
	if (j)
	{
	    if (i <= index && index < i + j)
	    {
		char *p = (char *)malloc(j + 1);
		memcpy(p, s + i, j);
		p[j] = 0;
		return p;
	    }
	    i = i + j - 1;
	}
    }
    return NULL;
}
