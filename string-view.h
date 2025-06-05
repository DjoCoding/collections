#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define SV_FMT          "%.*s"
#define SV_ARG(s)       (int)s.count, s.data

typedef struct {
    const char *data;
    size_t count;
} StringView;

StringView sv(const char *data, size_t count);
StringView svnull();
StringView svfromcstr(const char *data);
StringView svtrimleft(StringView s);
StringView svtrimright(StringView s);
StringView svtrim(StringView s);
StringView svsplit(StringView *s, char c);
size_t     svfind(StringView a, StringView b);
bool       svisnum(StringView s);
bool       svisalnum(StringView s);
bool       svisalpha(StringView s);
bool       svisupper(StringView s);
bool       svislower(StringView s);
bool       svendswith(StringView a, StringView b);
void       svdump(FILE *f, StringView s);

#ifdef STRING_VIEW_IMPLEMENTATION

StringView sv(const char *data, size_t count) {
    return (StringView) {
        .data   = data,
        .count  = count
    };
}

StringView svnull() {
    return sv(NULL, 0);
}

StringView svfromcstr(const char *data) {
    return sv(data, strlen(data));
}

StringView svtrimleft(StringView s) {
    while(s.count != 0) {
        if(!isspace(s.data[0])) break;
        s.data  += 1;
        s.count -= 1;
    }
    return s;
}

StringView svtrimright(StringView s) {
    while(s.count != 0) {
        if(!isspace(s.data[s.count - 1])) break;
        s.count -= 1;
    }
    return s;
}

StringView svtrim(StringView s) {
    return svtrimleft(svtrimright(s));
}

StringView svsplit(StringView *s, char c) {
    size_t i = 0;
    while(i < s->count) {
        char current = s->data[i];
        if(current == c) break;
        i += 1;
    }

    StringView result = *s;

    if(s->data[i] != c) {
        s->count = 0;
        s->data += i;
        return result;
    }

    result.count = i;
    s->data     += i + 1;
    s->count    -= i + 1;

    return result;
}

size_t svfind(StringView a, StringView b) {
    if(b.count > a.count) return a.count;
    
    size_t base     = 0;
    
    while (base <= a.count - b.count) {
        if(a.data[base] == b.data[0]) {
            size_t offset   = 1;
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

bool svisnum(StringView s) {
    bool isfloat = false;
    for(size_t i = 0; i < s.count; ++i) {
        char current = s.data[i];
        
        if(current == '.') {
            if(isfloat) return false;
            isfloat = true;
            continue;
        }

        if(isalpha(current)) return false;
        if(!isalnum(current)) return false;
    }

    return true;
}

bool svisalnum(StringView s) {
    bool isfloat = false;
    for(size_t i = 0; i < s.count; ++i) {
        char current = s.data[i];
        
        if(current == '.') {
            if(isfloat) return false;
            isfloat = true;
            continue;
        }

        if(!isalnum(current)) return false;
    }

    return true;
}

bool svisalpha(StringView s) {
    for(size_t i = 0; i < s.count; ++i) {
        char current = s.data[i];
        if(!isalpha(current)) return false;
    }
    return true;
}

bool svisupper(StringView s) {
    for(size_t i = 0; i < s.count; ++i) {
        char current = s.data[i];
        if(isalpha(current) && !isupper(current)) return false;
    }
    return true;
}


bool svislower(StringView s) {
    for(size_t i = 0; i < s.count; ++i) {
        char current = s.data[i];
        if(isalpha(current) && !islower(current)) return false;
    }
    return true;
}

bool svendswith(StringView a, StringView b) {
    size_t index = svfind(a, b);
    return index == a.count - b.count;
}

void svdump(FILE *f, StringView s) {
    fprintf(f, SV_FMT"\n", SV_ARG(s));
}

#endif // STRING_VIEW_IMPLEMENTATION

#endif // STRING_VIEW_H