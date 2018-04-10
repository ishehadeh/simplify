#include "scalar.h"
#include "string.h"
#include "stdio.h"

#if !defined(HAVE_GMP)
char* __ltoa(long x, char* buf) {
    int i = 0;
    if (x < 0) {
        *buf = '-';
        ++i;
        x = -x;
    }

    while(x > 0) {
        buf[i++] = (x % 10) + '0';
        x /= 10;
    }

    int ret = i;

    char t = buf[i];
    for(int j = 0; i > j; ++j, --i) {
        buf[i] = buf[j];
        buf[j] = t;
    }

    buf[ret] = 0;
    return buf;
}

char * __dtoa(double n, char *s) {
    if (isnan(n)) {
        strcpy(s, "@NAN@");
    } else if (isinf(n)) {
        strcpy(s, "@INF@");
    } else if (n == 0.0) {
        strcpy(s, "0");
    } else {
        int digit;
        char* c = s;

        int neg = n < 0;
        if(neg) n = -n;

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


        while (n > 0.00000000001 || magnitude >= 0) {
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

    return s;
}

#endif  // HAVE_GMP

char* _approximate_number(char* str, int min_reps) {
    size_t len = strlen(str);

    int deci;
    for(deci = 0; str[deci] && str[deci] != '.'; ++deci);

    if (!str[deci])
        return str;

    ++deci;

    int chain = 0;
    char* chain_start = str + deci;

    int i = deci;
    for(; str[i] >= '0' && str[i] <= '9'; ++i) {
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
    for(i = deci; (str[i] >= '0' && str[i] <= '9') || str[i] == 0; ++i) {
        if ( str[i] == last) {
            ++chain;
        } else {
            if (chain > min_reps && last == '0') {
                str[i - chain + 1] = 0;
                len -= chain + (len - i);
                if (len == deci) {
                    str[i - chain - 1] = 0;
                }
            }else if( chain >= min_reps && last == '9') {
                char* r = str + i - 1;
                char write_char = 0;
                for(; r != str && (*r == '9' || *r == '.'); --r ) {
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
        if(str[i] == 0) {
            break;
        }
    }

    return str;
}