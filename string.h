#ifndef STRING_H
#define STRING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

typedef struct { 
    char * const data;          // a constant pointer to a char pointer
    size_t count;
} String;

void strclean(String s);

String str(char *data, size_t count) {
    return (String) {
        .data   = (char *const) data,
        .count  = count 
    };
}

String strnull() {
    return str(NULL, 0);
}

String strfromcstr(const char *cstr) {
    size_t count        = strlen(cstr);

    char *const data    = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, cstr, count);

    return str(data, count);
}

String strclone(String s) {
    size_t count       = s.count;

    char *const data   = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, s.data, count);

    return str(data, count);
}

String stradd(String a, String b) {
    size_t count        = a.count + b.count;
    
    char *const data    = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, a.data, a.count);
    memcpy(&data[a.count], b.data, b.count);
    
    return str(data, count);
}

String strupper(String s) {
    String sc = strclone(s);

    for(size_t i = 0; i < sc.count; ++i) {
        sc.data[i] = toupper(sc.data[i]);
    }
    
    return sc;
}

String strlower(String s) {
    String sc = strclone(s);

    for(size_t i = 0; i < sc.count; ++i) {
        sc.data[i] = tolower(sc.data[i]);
    }
    
    return sc;
}

// if the returned value == a.count then the string `b` is not found in `a`
size_t strfind(String a, String b) {
    if(b.count > a.count) return a.count;
    
    size_t base     = 0;
    
    while (base < a.count - b.count) {
        if(a.data[base] == b.data[0]) {
            size_t offset   = 0;
            while(offset < b.count) {
                if(b.data[offset] != a.data[base + offset]) break;
                offset += 1;
            }
            if(offset == b.count) return base;
        }
        base += 1;
    } 

    return a.count;
}

bool strexists(String a, String b) {
    return strfind(a, b) != a.count;
}

String strsub(String a, size_t from, size_t to) {
    if(from > to) return strnull();
    if(from >= a.count) return strnull();
    if(to >= a.count) to = a.count;

    size_t count = to - from;
    
    char *const data = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, &a.data[from], count);

    return str(data, count);
}

String strtrimleft(String a) {
    size_t i = 0;
    while(i < a.count && isspace(a.data[i])) i += 1;
    
    size_t count = a.count - i;
    if(count == 0) return strnull();
    
    char *const data = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, &a.data[i], count);

    return str(data, count);
}

String strtrimright(String a) {
    size_t count = a.count;
    
    while(count > 0 && isspace(a.data[count - 1])) count -= 1;
    if(count == 0) return strnull();

    char *const data = (char *const) malloc(sizeof(char) * count);
    assert(data != NULL && "ERROR: malloc failed.");
    memcpy(data, a.data, count);

    return str(data, count);
}

String strtrim(String s) {
    String left     = strtrimleft(s);
    String result   = strtrimright(left);
    strclean(left);
    return result;
}

void strclean(String s) {
    if(s.data == NULL) return;
    return free(s.data);
}

char *strtocstr(String s) {
    char *data = (char *)malloc(sizeof(char) * (s.count + 1));
    assert(data != NULL && "ERROR: malloc failed.");

    memcpy(data, s.data, s.count);
    memset(&data[s.count], 0, sizeof(char));
    
    return data;
}

void strdump(String s) {
    fprintf(stdout, "%.*s\n", (int)s.count, s.data);
}



#endif