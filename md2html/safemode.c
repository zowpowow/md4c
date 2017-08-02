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
#include <ctype.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif



struct str_list {
    const char* str;
    size_t size;
};

#define RECORD(str)     { str, sizeof(str)-1 }

/* List of allowed tags. Any other tag is prohibited. */
static const struct str_list tag_whitelist[] = {
    RECORD("a"),            RECORD("b"),            RECORD("blockquote"),
    RECORD("br"),           RECORD("code"),         RECORD("dd"),
    RECORD("del"),          RECORD("details"),      RECORD("div"),
    RECORD("dl"),           RECORD("dt"),           RECORD("em"),
    RECORD("h1 "),          RECORD("h2"),           RECORD("h3"),
    RECORD("h4"),           RECORD("h5"),           RECORD("h6"),
    RECORD("hr"),           RECORD("i"),            RECORD("img"),
    RECORD("ins"),          RECORD("kbd"),          RECORD("li"),
    RECORD("ol"),           RECORD("p"),            RECORD("pre"),
    RECORD("q"),            RECORD("rp"),           RECORD("rt"),
    RECORD("ruby"),         RECORD("s"),            RECORD("samp"),
    RECORD("strike"),       RECORD("strong"),       RECORD("sub"),
    RECORD("summary"),      RECORD("sup"),          RECORD("table"),
    RECORD("tbody"),        RECORD("td"),           RECORD("tfoot"),
    RECORD("th"),           RECORD("thead"),        RECORD("tr"),
    RECORD("tt"),           RECORD("ul"),           RECORD("var")
};
static const int tag_whitelist_n = (sizeof(tag_whitelist) / sizeof(tag_whitelist[0]));

/* List of attributes with URI values. We want to filter them depending on the
 * value. */
static const struct str_list uri_attr_list[] = {
    RECORD("action"),       RECORD("cite"),         RECORD("classid"),
    RECORD("codebase"),     RECORD("data"),         RECORD("href"),
    RECORD("longdesc"),     RECORD("profile"),      RECORD("src"),
    RECORD("usemap")
};
static const int uri_attr_list_n = (sizeof(uri_attr_list) / sizeof(uri_attr_list[0]));

/* List of allowed URI schemes. Any other scheme will be refused.
 * (Note attributes without any scheme, e.g. relative links, are allowed.) */
static const struct str_list scheme_whitelist[] = {
    RECORD("http:"),        RECORD("https:"),       RECORD("mailto:")
};

static const int scheme_whitelist_n = (sizeof(scheme_whitelist) / sizeof(scheme_whitelist[0]));


static inline int
is_listed(const char* str, size_t str_size, const struct str_list* list, int list_n)
{
    int i;

    for(i = 0; i < list_n; i++) {
        if(str_size == list[i].size) {
            if(strncasecmp(str, list[i].str, str_size) == 0)
                return 0;
        }
    }

    /* Everything else is forbidden. */
    return -1;
}

/*
 * See https://tools.ietf.org/html/rfc3986#section-3.1 how the scheme may look.
 * In brief it is regexp "[A-Za-z][A-Za-z0-9+-.]*" followed by delimiter ':'.
 *
 * The function checks if the URL starts with a scheme. If so, *scheme and
 * *scheme_size are set accordingly (the size includes the final ':'). If
 * no scheme is present in the URL, they are set to NULL and 0.
 */
static void
check_url_scheme(const char* url, size_t url_size, const char** scheme, size_t* scheme_size)
{
    size_t off = 0;

    if(off < url_size && isalpha(url[off])) {
        while(off <= url_size  &&  (isalnum(url[off]) || url[off] == '+' || url[off] == '-' || url[off] == '.'))
            off++;

        if(off < url_size && url[off] == ':') {
            /* Scheme is present. */
            *scheme = url;
            *scheme_size = off+1;
            return;
        }
    }

    /* Scheme not present. */
    *scheme = NULL;
    *scheme_size = 0;
}

int
filter_tag(const char* name, size_t size, void* userdata)
{
    return is_listed(name, size, tag_whitelist, tag_whitelist_n);
}

int
filter_attribute(const char* tag, size_t tag_size,
                 const char* attr, size_t attr_size,
                 const char* value, size_t value_size,
                 void* userdata)
{
    /* Refuse all those "onload", "onlick" and similar event and
     * scripting-related attributes. */
    if(attr_size > 2  &&  strncasecmp(attr, "on", 2) == 0)
        return -1;

    /* Refuse any attribute with URI using other then allowed scheme.
     * (URIs without any scheme, e.g. relative links, are allowed.) */
    if(is_listed(attr, attr_size, uri_attr_list, uri_attr_list_n) == 0) {
        const char* scheme;
        size_t scheme_size;

        check_url_scheme(value, value_size, &scheme, &scheme_size);
        if(scheme != NULL)
            return is_listed(scheme, scheme_size, scheme_whitelist, scheme_whitelist_n);
    }

    return 0;
}

