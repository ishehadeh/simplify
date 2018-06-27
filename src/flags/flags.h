/* Copyright Ian Shehadeh 2018 */

#ifndef FLAGS_FLAGS_H_
#define FLAGS_FLAGS_H_

#define _FLAG_NEXT() ++flag_argv, --flag_argc

#define FLAG(SHORT, LONG, RESULT)                                          \
    if ((__flag_argi[0] == (SHORT) && (__flag_should_advance = 0) == 0) || \
        (__flag_argi[1] == '-' && strcmp(LONG, __flag_argi + 1) == 0)) {   \
        RESULT;                                                            \
    } else

#define PARSE_FLAGS(FLAGS) _PARSE_FLAGS(argv, argc, FLAGS)

#define _PARSE_FLAGS(ARGV, ARGC, BLOCK)                                 \
    char** flag_argv = (ARGV) + 1;                                      \
    int flag_argc = (ARGC)-1;                                           \
    {                                                                   \
        while (flag_argc > 0) {                                         \
            if (!strcmp(*flag_argv, "--")) {                            \
                _FLAG_NEXT();                                           \
                break;                                                  \
            }                                                           \
            if ((*flag_argv)[0] == '-') {                               \
                char* __flag_argi = *flag_argv + 1;                     \
                int __flag_should_advance = 0;                          \
                while (!__flag_should_advance || __flag_argi[0] == 0) { \
                    __flag_should_advance = 1;                          \
                    BLOCK {                                             \
                        printf("unkown flag '%s'\n", (*flag_argv));     \
                        exit(1);                                        \
                    }                                                   \
                    ++__flag_argi;                                      \
                }                                                       \
                _FLAG_NEXT();                                           \
            } else {                                                    \
                break;                                                  \
            }                                                           \
        }                                                               \
    }

#define FLAG_VALUE                                                                        \
    (((*flag_argv)[1] != '-' && (*flag_argv)[2] != 0) && (__flag_should_advance = 1) == 1 \
         ? __flag_argi + 1                                                                \
         : ((flag_argc >= 0 && --flag_argc >= 0) ? *(++flag_argv) : ""))

#endif  // FLAGS_FLAGS_H_
