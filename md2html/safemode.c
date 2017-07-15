/*
 * MD4C: Markdown parser for C
 * (http://github.com/mity/md4c)
 *
 * Copyright (c) 2016-2017 Martin Mitas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "safemode.h"

#include <string.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif



struct filter_record {
    const char* name;
    size_t size;
};

#define RECORD(str)     { str, sizeof(str)-1 }

static const struct filter_record tag_whitelist[] = {
    /* TODO: This is good enough only for testing during the development and
     *       needs to be expanded considerably. */
    RECORD("div"),
    RECORD("span")
};

static const int tag_whitelist_n = (sizeof(tag_whitelist) / sizeof(tag_whitelist[0]));

static const struct filter_record scheme_whitelist[] = {
    RECORD("http"),
    RECORD("https"),
    RECORD("ftp")
};

static const int scheme_whitelist_n = (sizeof(scheme_whitelist) / sizeof(scheme_whitelist[0]));


static int
whitelist_filter(const char* name, size_t size, const struct filter_record* whitelist, int whitelist_size)
{
    int i;

    for(i = 0; i < whitelist_size; i++) {
        if(size == whitelist[i].size) {
            if(strncasecmp(name, whitelist[i].name, size) == 0)
                return 0;
        }
    }

    /* Everything else is forbidden. */
    return -1;
}


int
filter_tag(const char* name, size_t size, void* userdata)
{
    return whitelist_filter(name, size, tag_whitelist, tag_whitelist_n);
}

int
filter_scheme(const char* name, size_t size, void* userdata)
{
    return whitelist_filter(name, size, scheme_whitelist, scheme_whitelist_n);
}
