#include "get_args.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifndef _WIN32
#include <unistd.h>
#else 
#include <windows.h>
#endif

/**
 * @brief parser agruement from  command line
 *
 * @param agrc   [in]  agruement count
 * @param agrv[] [in]  pointer to agruement
 * @param opt    [out] options struct
 */
void get_args(int agrc, char *agrv[], struct options *opt)
{
    int i          = 0;
    int ret_int    = 1;
    int args_cnt   = 0;
    int arg_num    = 0;
    struct options *opts = opt;

    if (agrc < 1 || agrv == NULL) {
        fprintf(stderr, "No parameter to parser!\n");
        exit(1);
    }
    if (opt == NULL) {
        fprintf(stderr, "Please give struct options!\n");
        exit(1);
    }

    for (i = 1; i < agrc; i++) {
        if (agrv[i][0] != '-') continue;

        for (opts = opt; (opts->short_name != NULL || opts->long_name != NULL); opts++) {
            if (opts->been_get == 1) {
                continue;
            }
             
#ifndef _WIN32
            if ((opts->short_name != NULL && 
                    strlen(agrv[i]) == strlen(opts->short_name) && 
                    !strncasecmp(opts->short_name, agrv[i], strlen(opts->short_name))) || 
                (opts->long_name != NULL && 
                    strlen(agrv[i]) == strlen(opts->long_name) && 
                    !strncasecmp(opts->long_name, agrv[i], strlen(opts->long_name)))) {
#else
            if ((opts->short_name != NULL && 
                    strlen(agrv[i]) == strlen(opts->short_name) && 
                    !strncmp(opts->short_name, agrv[i], strlen(opts->short_name))) || 
                (opts->long_name != NULL && 
                    strlen(agrv[i]) == strlen(opts->long_name) && 
                    !strncmp(opts->long_name, agrv[i], strlen(opts->long_name)))) {
#endif
    
                if (opts->value == NULL) continue;
                switch (opts->has_args) {
                    case 0:
                        if (opts->value_type == RET_STR) {
                            fprintf(stderr, "option [%s] return value type wrong\n", agrv[i]);
                            exit(1);
                        }

                        if (opts->value_type == RET_INT) {
                            ret_int = 1;
                            memcpy(opts->value, &ret_int, sizeof(int));
                        } else {
                            memcpy(opts->value, "1", 1);
                        }
                        break;
                    case -1:
                        while (++i < agrc && agrv[i][0] != '-') {
                            switch (opts->value_type) {
                                case RET_CHR:
                                    memcpy(opts->value, agrv[i], 1);
                                    break;
                                case RET_INT:
                                    if (i < agrc) ret_int = atoi(agrv[i]);
                                    memcpy(opts->value, &ret_int, sizeof(int));
                                    break;
                                default:
                                    *(opts->value) = agrv[i];
                                    break;
                            }
                            opts->value++;

                        }
                        opts->value = NULL;
                        opts->been_get = 1;
                        if (i < agrc && agrv[i][0] == '-') i--;
                        break;
                    default:
                        if ((i + opts->has_args) >= agrc) {
                            fprintf(stderr, "agruement [%s] need a parameter\n", agrv[i]);
                            exit(1);
                        }

                        i++;
                        args_cnt = opts->has_args;
                        arg_num = i - 1;
                        while (args_cnt-- > 0) {
                            if (agrv[i][0] == '-') {
                                fprintf(stderr, "Option [%s] parameter error\n", agrv[arg_num]);
                                exit(1);
                            }
                            switch (opts->value_type) {
                                case RET_CHR:
                                    memcpy(opts->value, agrv[i], 1);
                                    break;
                                case RET_INT:
                                    if (i < agrc) ret_int = atoi(agrv[i]);
                                    memcpy(opts->value, &ret_int, sizeof(int));
                                    break;
                                default:
                                    *(opts->value) = agrv[i];
                                    break;
                            }

                            i++;
                            opts->value++;
                        }
                        i--;
                }

                opts->been_get = 1;
                break;
            } 
        }

        if (!opts->been_get) {
            fprintf(stderr, "Invalid arguement -- %s\n", agrv[i]);
            exit(1);
        }
    }

    return;

}

static char *get_proc_name()
{
    char proc_path[] = "/proc/self/exe";
    static char proc_name[256];
    char *proc = NULL;
    int name_len = 0;

#ifndef _WIN32
    if ((name_len = readlink(proc_path, proc_name, sizeof(proc_name))) < 0) return NULL;
    if ((proc = strrchr(proc_name, '/')) == NULL) return NULL;
#else
    GetModuleFileName(NULL, proc_name, sizeof(proc_name));
    if ((proc = strrchr(proc_name, '\\')) == NULL) return NULL;
#endif
    return proc + 1;
}

static int usage_line_len = 50;
void print_usage(struct usage *help_usage)
{
    struct usage *use    = help_usage;
    int opt_name_max_len = 0;
    int opt_name_len     = 0;
    int opt_usg_len      = 0;
    int usg_cnt          = 0;
    int cpy_cnt          = 0;
    char *buf            = NULL;
    //char buf[usage_line_len + 1] = {0};
    char c = ' ';

    if (!use) return;
    buf = (char *)malloc(usage_line_len + 1);
    if (!buf) return;
    
    printf("Usage: %s [Options] [Parameters]\n", get_proc_name());
    printf("The Options arg:\n");

    for (use = help_usage; use->opt_name != NULL; use++) {
        opt_name_len = strlen(use->opt_name);
        if (opt_name_len > opt_name_max_len) opt_name_max_len = opt_name_len;
    }

    for (use = help_usage; use->opt_name != NULL; use++) {
        usg_cnt = 0;
        opt_usg_len = strlen(use->opt_usage);
        opt_name_len = strlen(use->opt_name);
        if (opt_usg_len > usage_line_len) {
            memcpy(buf, use->opt_usage + usg_cnt, usage_line_len);
            usg_cnt += usage_line_len;
            if (isalpha((use->opt_usage + usg_cnt - 1)[0]))
                c = '-';
            printf("  %s%*c%s%c\n", use->opt_name, opt_name_max_len + 2 - opt_name_len, ' ', buf, c);

            while (usg_cnt < opt_usg_len) {
                cpy_cnt = usg_cnt + usage_line_len < opt_usg_len ? usage_line_len - 1 : (opt_usg_len - usg_cnt);
                memcpy(buf, use->opt_usage + usg_cnt, cpy_cnt);
                buf[cpy_cnt] = '\0';
                usg_cnt += cpy_cnt;
                if (usg_cnt < opt_usg_len) {
                    if (isalpha((use->opt_usage + usg_cnt - 1)[0]))
                        c = '-';
                } else {
                    c = ' ';
                }
                printf("  %*c%s%c\n", opt_name_max_len + 2, ' ', buf, c);

            }
        } else {
            printf("  %s%*c%s\n", use->opt_name, opt_name_max_len + 2 - opt_name_len, ' ', use->opt_usage);
        }
        memset(buf, 0, sizeof(usage_line_len + 1));
    }

    if (buf) free(buf);
}

/**
 * @brief set_print_usage_width 
 */
void set_print_usage_width(int width)
{
    usage_line_len = width;
}
