/* Copyright Ian Shehadeh 2018 */

#ifndef FLAGS_FLAGS_H_
#define FLAGS_FLAGS_H_

#define FLAG(SHORT, LONG, RESULT)                                             \
    if ((*flag_argv)[1] == (SHORT) || strcmp(LONG, (*flag_argv) + 2) == 0 ) { \
        _GET_FLAG_VALUE()                                                     \
        RESULT;                                                               \
    } else

#define PARSE_FLAGS(FLAGS) _PARSE_FLAGS(argv, argc, FLAGS)

#define _PARSE_FLAGS(ARGV, ARGC, BLOCK)                                                 \
    char** flag_argv = (ARGV) + 1;                                                      \
    int    flag_argc = (ARGC) - 1;                                                      \
{                                                                                       \
    for (; flag_argc != 0;  ++flag_argv, --flag_argc) {                                 \
        if ((*flag_argv)[0] == '-') {                                                   \
            char* flag_value = NULL;                                                    \
            BLOCK                                                                       \
            {                                                                           \
                printf("unkown argument '%s'\n", (*flag_argv));                         \
                exit(1);                                                                \
            }                                                                           \
        } else {                                                                        \
            break;                                                                      \
        }                                                                               \
    }                                                                                   \
}                                                                                       \

#define _GET_FLAG_VALUE()                                                       \
    if ((*flag_argv)[1] != '-' && (*flag_argv)[2] != 0) {                       \
        flag_value = (*flag_argv) + 2;                                          \
    } else {                                                                    \
        flag_value = *(++flag_argv);                                            \
        --flag_argc;                                                            \
    }                                                                           \



#endif  // FLAGS_FLAGS_H_
