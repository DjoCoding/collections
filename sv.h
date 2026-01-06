#ifndef COLLECTIONS_SV_H
#define COLLECTIONS_SV_H

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief Represents a non-owning view of a string.
 *
 * A StringView provides a pointer to a portion of a string and its size.
 * It does not manage memory — it only references existing string data.
 */
typedef struct {
    char   *content; /**< Pointer to the string data (not null-terminated). */
    size_t  size;    /**< Length of the string segment (excluding '\0'). */
} StringView;

/**
 * @brief Creates a StringView from a char array and its size.
 *
 * @param content Pointer to the string data.
 * @param size Length of the string.
 * @return A new StringView.
 */
StringView sv(char *content, size_t size);

/**
 * @brief Creates a StringView from a null-terminated C string.
 *
 * @param content Pointer to a null-terminated string.
 * @return A new StringView referencing the given string.
 */
StringView sv_from_cstr(char *content);

/**
 * @brief Converts all characters to lowercase.
 *
 * @param sv Input StringView.
 * @return A new lowercase StringView.
 */
StringView sv_lower(StringView sv);

/**
 * @brief Converts all characters to uppercase.
 *
 * @param sv Input StringView.
 * @return A new uppercase StringView.
 */
StringView sv_upper(StringView sv);

/**
 * @brief Removes leading whitespace characters.
 *
 * @param sv Input StringView.
 * @return Trimmed StringView.
 */
StringView sv_ltrim(StringView sv);

/**
 * @brief Removes trailing whitespace characters.
 *
 * @param sv Input StringView.
 * @return Trimmed StringView.
 */
StringView sv_rtrim(StringView sv);

/**
 * @brief Removes both leading and trailing whitespace.
 *
 * @param sv Input StringView.
 * @return Fully trimmed StringView.
 */
StringView sv_trim(StringView sv);

/**
 * @brief Returns a substring of the StringView.
 *
 * @param sv Source StringView.
 * @param lower Start index (inclusive).
 * @param upper End index (exclusive).
 * @return Sliced StringView.
 */
StringView sv_slice(StringView sv, size_t lower, size_t upper);

/**
 * @brief Capitalizes the first character (if alphabetic).
 *
 * @param sv Input StringView.
 * @return Capitalized StringView.
 */
StringView sv_capitalize(StringView sv);

/**
 * @brief Checks if a StringView ends with another StringView.
 *
 * @param sv Main string.
 * @param a Suffix to check.
 * @return true if sv ends with a, false otherwise.
 */
bool sv_endswith(StringView sv, StringView a);

/**
 * @brief Checks if a StringView begins with another StringView.
 *
 * @param sv Main string.
 * @param a Prefix to check.
 * @return true if sv begins with a, false otherwise.
 */
bool sv_beginswith(StringView sv, StringView a);

/**
 * @brief Compares two StringViews for equality.
 *
 * @param a First StringView.
 * @param b Second StringView.
 * @return true if both views have equal content and size.
 */
bool sv_eq(StringView a, StringView b);

/**
 * @brief Checks if the StringView contains only digits (integer).
 *
 * @param sv Input StringView.
 * @return true if valid integer representation.
 */
bool sv_is_int(StringView sv);

/**
 * @brief Checks if the StringView represents a valid floating-point number.
 *
 * @param sv Input StringView.
 * @return true if valid float representation.
 */
bool sv_is_float(StringView sv);

/**
 * @brief Returns the size (length) of the StringView.
 *
 * @param sv Input StringView.
 * @return Length of the string segment.
 */
size_t sv_size(StringView sv);

/**
 * @brief Returns the raw content pointer of the StringView.
 *
 * @param sv Input StringView.
 * @return Pointer to the underlying string data.
 */
char *sv_content(StringView sv);

/**
 * @brief Converts the StringView content to an integer.
 *
 * @param sv Input StringView containing digits.
 * @return Integer value represented by the StringView.
 */
int sv_conv_int(StringView sv);

/**
 * @brief Converts the StringView content to a float.
 *
 * @param sv Input StringView containing a numeric value.
 * @return Float value represented by the StringView.
 */
float sv_conv_float(StringView sv);

/**
 * @brief Splits a StringView at the first occurrence of a character.
 * @param sv Input StringView to be split.
 * @param c The delimiter character.
 * @return A StringView containing the part before the delimiter, or the original view if not found. 
 */
StringView sv_split(StringView sv, char c);

#define SV(c)        ((StringView){ .content = (char *)(c), .size = (c) == NULL ? 0 : (sizeof(c) - 1) })
#define SV_NULL         (SV(NULL))
#define SV_FMT          "%.*s"
#define SV_ARG(s)       (int)(s).size, (s).content


#ifdef COLLECTIONS_SV_IMPLEMENTATION

#include <string.h>
#include <ctype.h>


StringView sv(char *content, size_t size) {
    return (StringView) {
        .content = content,
        .size = size
    };
}

StringView sv_from_cstr(char *content) {
    return sv(content, strlen(content));
}

StringView sv_lower(StringView sv) {
    for (size_t i = 0; i < sv.size; ++i) {
        if(!isupper(sv.content[i])) continue;
        sv.content[i] = sv.content[i] - 'A' + 'a';
    }
    return sv;
}

StringView sv_upper(StringView sv) {
    for (size_t i = 0; i < sv.size; ++i) {
        if(!islower(sv.content[i])) continue;
        sv.content[i] = sv.content[i] - 'a' + 'A';
    }
    return sv;
}

StringView sv_ltrim(StringView sv) {
    while (sv.size > 0 && isspace((unsigned char)*sv.content)) {
        sv.content++;
        sv.size--;
    }
    return sv;
}

StringView sv_rtrim(StringView sv) {
    while (sv.size > 0 && isspace((unsigned char)sv.content[sv.size - 1]))
        sv.size--;
    return sv;
}

StringView sv_trim(StringView sv) {
    return sv_rtrim(sv_ltrim(sv));
}

StringView sv_slice(StringView s, size_t lower, size_t upper) {
    if (lower >= s.size || upper <= lower) return SV_NULL;
    if (upper > s.size) upper = s.size;
    return sv(s.content + lower, upper - lower);
}

StringView sv_capitalize(StringView sv) {
    if (sv.size == 0) return sv;
    sv_lower(sv);
    if (isalpha((unsigned char)sv.content[0]))
        sv.content[0] = (char)toupper((unsigned char)sv.content[0]);
    return sv;
}

bool sv_endswith(StringView s, StringView a) {
    if (a.size > s.size) return false;
    return sv_eq(sv(s.content + s.size - a.size, a.size), a);
}

bool sv_beginswith(StringView s, StringView a) {
    if (a.size > s.size) return false;
    return sv_eq(sv(s.content, a.size), a);
}

bool sv_eq(StringView a, StringView b) {
    return a.size == b.size && memcmp(a.content, b.content, a.size) == 0;
}

bool sv_is_int(StringView sv) {
    if (sv.size == 0) return false;
    size_t i = 0;
    if (sv.content[0] == '-') {
        if (sv.size == 1) return false;
        i++;
    }
    for (; i < sv.size; i++)
        if (!isdigit((unsigned char)sv.content[i])) return false;
    return true;
}

bool sv_is_float(StringView sv) {
    if (sv.size == 0) return false;
    size_t i = 0;
    if (sv.content[0] == '-') {
        if (sv.size == 1) return false;
        i++;
    }
    bool dot = false;
    for (; i < sv.size; i++) {
        if (sv.content[i] == '.') {
            if (dot || i == 0 || i == sv.size - 1) return false;
            dot = true;
        } else if (!isdigit((unsigned char)sv.content[i])) {
            return false;
        }
    }
    return true;
}

size_t sv_size(StringView sv) { return sv.size; }
char *sv_content(StringView sv) { return sv.content; }

int sv_conv_int(StringView sv) {
    int sign = 1;
    size_t i = 0;
    if (sv.content[0] == '-') {
        sign = -1;
        i++;
    }
    int result = 0;
    for (; i < sv.size; i++)
        result = result * 10 + (sv.content[i] - '0');
    return result * sign;
}

float sv_conv_float(StringView sv) {
    int sign = 1;
    size_t i = 0;
    if (sv.content[0] == '-') {
        sign = -1;
        i++;
    }
    float result = 0.0f;
    float frac = 0.0f, base = 0.1f;
    bool after_dot = false;

    for (; i < sv.size; i++) {
        char c = sv.content[i];
        if (c == '.') {
            if (after_dot) break;
            after_dot = true;
            continue;
        }
        if (!isdigit((unsigned char)c)) break;
        if (after_dot) {
            frac += (c - '0') * base;
            base *= 0.1f;
        } else {
            result = result * 10.0f + (c - '0');
        }
    }

    return sign * (result + frac);
}

StringView sv_split(StringView s, char c) {
    for(size_t i = 0; i < s.size; ++i) {
        if(s.content[i] == c) return sv(s.content, i);
    }
    return s;
}

#endif // COLLECTIONS_SV_IMPLEMENTATION
#endif // COLLECTIONS_SV_H
