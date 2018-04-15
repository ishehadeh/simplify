// Copyright Ian R. Shehadeh 2018

#include <string.h>
#include <math.h>

#include "simplify/scalar/scalar.h"

// TODO(IanS5): Cleanup this function
char* _approximate_number(char* str, int min_reps) {
    size_t len = strlen(str);

    int deci;
    for (deci = 0; str[deci] && str[deci] != '.'; ++deci) {}

    if (!str[deci])
        return str;

    ++deci;

    int chain = 0;
    char* chain_start = str + deci;

    int i;
    for (i = deci; str[i] >= '0' && str[i] <= '9'; ++i) {
        if (str[i] == '0') {
            ++chain;
        } else {
            chain = 0;
        }
    }


    if (str[i] == 0) {
        str[i - chain] = 0;
        len -= chain + (len - i);
        if (len == deci) {
            str[i - chain - 1] = 0;
        }
    }

    chain = 0;
    char last = 0;
    for (i = deci; (str[i] >= '0' && str[i] <= '9') || str[i] == 0; ++i) {
        if (str[i] == last) {
            ++chain;
        } else {
            if (chain > min_reps && last == '0') {
                str[i - chain + 4] = 0;
                len -= chain + (len - i);
                if (len == deci) {
                    str[i - chain - 1] = 0;
                }
            } else if (chain >= min_reps && last == '9') {
                char* r;
                char write_char = 0;
                for (r = str + i - 1; r != str && (*r == '9' || *r == '.'); --r) {
                    if (!write_char) --len;
                    if (*r == '.') {
                        *r = write_char;
                        write_char = '0';
                    } else {
                        *r = write_char;
                    }
                }

                if (r == str) {
                    memmove(str + 1, str, len-1);
                    str[0] = '1';
                    str[len - 2] = '0';
                } else {
                    ++(*r);
                }
                break;
            }
            last = str[i];
            chain_start = str + i;
        }
        if (str[i] == 0) {
            break;
        }
    }

    return str;
}


char* ltoa(long x, char* c) {
    int i = 0;
    if (x < 0) {
        *c = '-';
        ++i;
        x = -x;
    }

    while (x > 0) {
        c[i++] = (x % 10) + '0';
        x /= 10;
    }

    int ret = i;

    for (int j = 0, k = i; k > j; ++j, --k) {
        c[k] ^= c[j];
        c[j] ^= c[k];
        c[k] ^= c[j];
    }

    c[i] = 0;
    return c;
}

char * dtoa(double n, char *s) {
    if (isnan(n)) {
        strncpy(s, __g_nan_string, 5);
    } else if (isinf(n)) {
        strncpy(s, __g_inf_string, 5);
    } else if (n == 0.0) {
        strncpy(s, "0", 1);
    } else {
        int digit;
        char* c = s;

        int neg = n < 0;
        if (neg) n = -n;

        int magnitude = log10(n);
        int magnitude_exp;

        int scientific = (magnitude >= 14 || (neg&& magnitude >= 9) || magnitude <= -9);

        if (neg) *(c++) = '-';

        if (scientific) {
            if (magnitude < 0) magnitude -= 1.0;
            n = n / pow(10.0, magnitude);
            magnitude_exp = magnitude;
            magnitude = 0;
        }

        if (magnitude < 1.0)
            magnitude = 0;


        while (n > pow(10, -FLOAT_PRECISION) || magnitude >= 0) {
            double weight = pow(10.0, magnitude);
            if (weight > 0 && !isinf(weight)) {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (magnitude == 0 && n > 0)
                *(c++) = '.';
            magnitude--;
        }

        if (scientific) {
            *(c++) = 'e';
            if (magnitude_exp > 0) {
                *(c++) = '+';
            } else {
                *(c++) = '-';
                magnitude_exp = -magnitude_exp;
            }
            magnitude = 0;
            while (magnitude_exp > 0) {
                *(c++) = '0' + magnitude_exp % 10;
                magnitude_exp /= 10;
                magnitude++;
            }
            c -= magnitude;
            for (int i = 0, j = magnitude - 1; i < j; ++i, --j) {
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += magnitude;
        }
        *c = '\0';
    }

    _approximate_number(s, 5);
    return s;
}
