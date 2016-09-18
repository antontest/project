#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "print.h"
#ifndef _WIN32 
#include <unistd.h>
#include <utils/utils.h>
#else
#include <io.h>
#include <windows.h>
#include "utils.h"
#endif

#ifndef _WIN32
/**
 *
 *  左上 = ┌
 *  上   =  ┬
 *  右上 =  ┐
 *  左   =  ├
 *  中心 =  ┼
 *  右   =  ┤
 *  左下 =  └
 *  下   =  ┴
 *  右下 =  ┘
 *  垂直 =  │
 *  水平 =   ─
 *
 */

typedef enum direct_id_t direct_id_t;
enum direct_id_t {
    DIRECT_UPLEFT = 0,
    DIRECT_UP,
    DIRECT_UPRIGHT,
    DIRECT_LEFT,
    DIRECT_CENTER,
    DIRECT_RIGHT,
    DIRECT_DOWNLEFT,
    DIRECT_DOWN,
    DIRECT_DOWNRIGHT,
    DIRECT_VERTICAL,
    DIRECT_HORIZONTAL
};

typedef struct direct_info_t direct_info_t;
struct direct_info_t {
    direct_id_t id;
    char *key;
};

direct_info_t direct_info[] = {
    {DIRECT_UPLEFT,     "┌"},
    {DIRECT_UP,         "┬"},
    {DIRECT_UPRIGHT,    "┐"},
    {DIRECT_LEFT,       "├"},
    {DIRECT_CENTER,     "┼"},
    {DIRECT_RIGHT,      "┤"},
    {DIRECT_DOWNLEFT,   "└"},
    {DIRECT_DOWN,       "┴"},
    {DIRECT_DOWNRIGHT,  "┘"},
    {DIRECT_VERTICAL,   "│"},
    {DIRECT_HORIZONTAL, "─"},
};

#endif

typedef enum input_type_t input_type_t;
enum input_type_t {
    INPUT_TYPE_CHAR = 0, 
    INPUT_TYPE_STR,
    INPUT_TYPE_INT,
    INPUT_TYPE_LONG,
    INPUT_TYPE_FLOAT,
};

typedef struct private_menu_t private_menu_t;
struct private_menu_t {
    /**
     * @brief pub interface
     */
    menu_t pub;

    /**
     * @brief header for menu
     */
    char *header;

    /**
     * @brief separator
     */
    char separator;

    /**
     * @brief menu width
     */
    unsigned int menu_width;

    /**
     * @brief start index
     */
    unsigned int start_index;

    /**
     * @brief menu index
     */
    unsigned int menu_index;

    /**
     * @brief choices
     */
    int *choices;

    /**
     * @brief menu choice count
     */
    int choice_count;

    /**
     * @brief menu choice count
     */
    int menu_count;

    /**
     * @brief whether support muti selected
     */
    unsigned int is_support_multi_selected;
};

METHOD(menu_t, init_menu_, void, private_menu_t *this, unsigned int menu_width, char *header, unsigned int start_index, unsigned int is_support_multi_selected)
{
    if (header) {
        this->header = header;
    } else {
        this->header = DFT_MENU_HEADER;
    }

    this->start_index = start_index;
    this->menu_index  = start_index;
    if (is_support_multi_selected >= 1) {
        this->is_support_multi_selected = 1;
    }

    this->menu_width = menu_width;
}

static void print_separator(FILE *fp, char separator, int width)
{
    while (width-- > 0) {
        fprintf(fp, "%c", separator);
    }
    fprintf(fp, "\n");
    fflush(fp);
}

METHOD(menu_t, show_menu_, void, private_menu_t *this, ...)
{
    char    *menu      = NULL;
    int     max_len    = 0;
    int     menu_len   = 0;
    int     header_len = 0;
    int     menu_index = this->start_index;
    va_list menu_list;

    va_start(menu_list, this);
    while (1) {
        menu = va_arg(menu_list, char *);
        if (!menu) {
            break;
        }

        menu_len = strlen(menu);
        if (menu_len > max_len) {
            max_len = menu_len;
        }
        this->menu_count++;
    }
    va_end(menu_list);

    max_len += 2 + DFT_MENU_BLANK_WIDTH * 2;
    /*
    if (max_len < DFT_MENU_WIDTH) {
        max_len = DFT_MENU_WIDTH;
    }
    */

    /**
     * print header
     */
    header_len = strlen(this->header);
    if (header_len > max_len) {
        max_len = header_len;
    }
    printf("%*s\n", header_len + (max_len - header_len) / 2, this->header);

    /**
     * print separator
     */
    print_separator(stdout, this->separator, max_len);

    /**
     * print menu
     */
    va_start(menu_list, this);
    while (1) {
        menu = va_arg(menu_list, char *);
        if (!menu) {
            break;
        }
        printf("%*s", DFT_MENU_BLANK_WIDTH, " ");
        printf("%d. %s\n", menu_index++, menu);
    }
    va_end(menu_list);

    /**
     * print separator
     */
    print_separator(stdout, this->separator, max_len);
    printf("  Please input your choice (%d - %d): \n", this->start_index, menu_index - 1);
}

METHOD(menu_t, show_head_, void, private_menu_t *this, char *head_info)
{
    int header_len = 0;

    if (!head_info) {
        head_info = this->header;
    }

    header_len = strlen(head_info);
    printf("%*s\n", header_len + (this->menu_width - header_len) / 2, head_info);

    /**
     * print separator
     */
    print_separator(stdout, this->separator, this->menu_width);
}

METHOD(menu_t, show_choice_, void, private_menu_t *this, char *choice)
{
    printf("%*s", DFT_MENU_BLANK_WIDTH, " ");
    printf("%d. %s\n", this->menu_index++, choice);

    this->menu_count++;
}

METHOD(menu_t, show_tips_, void, private_menu_t *this, char *tips)
{
    /**
     * print separator
     */
    print_separator(stdout, this->separator, this->menu_width);
    printf("  Please input your choice (%d - %d): \n", this->start_index, this->menu_index - 1);
}

static int check_alpha(char *string)
{
    char *p = string;

    while (*p != '\0' && *p != '\n') {
        if ((*p >= '0' && *p <= '9') || *p == ' ') {
            p++;
        } else {
            return 1;
        }
    }

    return 0;
}

static int check_choice(private_menu_t *this, int choices[], int choice)
{
    int i = 0;

    if (!choices) {
        return 0;
    }

    for (i = 0; i < this->menu_count; i++) {
        if (choices[i] == choice) {
            return 1;
        }
    }

    return 0;
}

METHOD(menu_t, get_choice, int, private_menu_t *this, int choices[], int *size)
{
    int  input_index  = 0;
    int  input_choice = 0;
    char buf[128]     = {0};
    char *result      = NULL;
    int  ret          = 0;

    /**
     * read input
     */
    if (!fgets(buf, sizeof(buf), stdin) || check_alpha(buf)) {
        *size = 0;
        return 1;
    }
    result = strtok(buf, " ");

    if (size) {
        memset(choices, -1, sizeof(int) * *size);
    }

    while (result) {
        input_choice = atoi(result);
        if (input_choice >= this->start_index && 
            input_choice < this->start_index + this->menu_count) {
            if (!size) {
                memcpy(choices, &input_choice, sizeof(input_choice));
                break;
            }

            if (check_choice(this, choices, input_choice)) {
                goto sep;
            }

            if (choices && input_index < *size) {
                choices[input_index++] = input_choice;
            }
        } else {
            // printf("No such choice!\n");
            ret = 1;
            break;
        }

        if (!this->is_support_multi_selected) {
            break;
        }

sep:
        result = strtok(NULL, " ");
    }

    /**
     * copy choices
     */
    if (size) {
        *size = input_index;
    }

    return ret;
}

METHOD(menu_t, vget_choice_, int, private_menu_t *this, int **choices, int *size)
{
    int  input_choice = 0;
    char buf[128]     = {0};
    char tmp[128]     = {0};
    char *result      = NULL;
    int  *pchoice     = NULL;
    int  ret          = 0;

    /**
     * read input
     */
    if (!fgets(buf, sizeof(buf), stdin) || check_alpha(buf)) {
        goto end;
    }
    memcpy(tmp, buf, sizeof(tmp));
    result = strtok(buf, " ");

    /**
     * choices count
     */
    this->choice_count = 0;
    while (result) {
        this->choice_count++;
        result = strtok(NULL, " ");
    }

    /**
     * malloc memory
     */
    if (this->choices) {
        free(this->choices);
    }
    this->choices      = NULL;
    this->choices = (int *)malloc(this->choice_count * sizeof(int));
    if (!this->choices) {
        return 1;
    }
    memset(this->choices, -1, sizeof(int) * this->choice_count);
    pchoice = this->choices;

    result = strtok(tmp, " ");
    while (result) {
        input_choice = atoi(result);
        if (input_choice >= this->start_index && 
            input_choice < this->start_index + this->menu_count) {

            *pchoice++ = input_choice;
        } else {
            this->choice_count = 0;
            ret = 1;
            break;
        }

        if (!this->is_support_multi_selected) {
            break;
        }

        result = strtok(NULL, " ");
    }

end:
    if (size) {
        *size = this->choice_count;
    }
    if (choices) {
        *choices = this->choices;
    }

    return ret;
}

METHOD(menu_t, menu_destroy_, void, private_menu_t *this)
{
    if (this->choices) {
        free(this->choices);
    }
    free(this);
}

/**
 * @brief menu_create 
 */
menu_t *menu_create()
{
    private_menu_t *this;

#ifndef _WIN32
    INIT(this, 
            .pub = {
            .init        = _init_menu_,
            .show_menu   = _show_menu_,
            .show_head   = _show_head_,
            .show_choice = _show_choice_,
            .show_tips   = _show_tips_,
            .get_choice  = _get_choice,
            .vget_choice = _vget_choice_,
            .destroy     = _menu_destroy_,
            },
            .header                    = NULL,
            .separator                 = DFT_MENU_SEPARATOR,
            .start_index               = DFT_MENU_START_INDEX,
            .is_support_multi_selected = DFT_MENU_MULTI_SELECTED,
            .choices                   = NULL,
            .choice_count              = 0,
            .menu_count                = 0,
        );
#else
    INIT(this, private_menu_t, 
            {
            init_menu_,
            show_menu_,
            show_head_,
            show_choice_,
            show_tips_,
            get_choice,
            vget_choice_,
            menu_destroy_,
            },
            NULL, 
            DFT_MENU_SEPARATOR,
            0,
            DFT_MENU_START_INDEX,
            DFT_MENU_START_INDEX,
            NULL,
            0,
            0,
            DFT_MENU_MULTI_SELECTED
        );
#endif

    return &this->pub;
}


typedef struct private_table_t private_table_t;
struct private_table_t {
    /**
     * @brief pub interface
     */
    table_t pub;

    /**
     * @brief count of table colum 
     */
    int col_cnt;

    /**
     * @brief width of colums
     */
    int *col_width;

    /**
     * @brief format of row output
     */
    char *fmt;

    /**
     * @brief fmt length
     */
    int fmt_len;

    /**
     * @brief ponter to fmt
     */
    char *cur;

    /**
     * @brief log file handler
     */
    FILE *fp;

    /**
     * @brief log switch
     */
    int log_onoff;
};

static int number_len(int n)
{
    int len = 0;
    while (n) {
        len++;
        n /= 10;
    }

    return len;
}

METHOD(table_t, init_table_, int, private_table_t *this, char *log_file, char *header, ...)
{
    va_list list;
    char    *col          = NULL, *type = NULL;
    int     width         = 0;
    int     total_width   = 0;
    int     len           = 0;
    char    last_ch       = '0';
    char    *pfmt         = NULL;
    char    *ptype        = NULL;
    char    *log          = NULL;
    char    log_path[128] = {0};

    /**
     * if has no header, return
     */
    if (!header) {
        return 1;
    }

    /**
     * check log file log
     */
    if (log_file && *log_file != '\0') {
#ifndef _WIN32
        SNPRINTF(log_path, sizeof(log_path) - 1, DFT_LOG_FILE_DIR "/%s", log_file);
#else
        SNPRINTF(log_path, sizeof(log_path) - 1, DFT_LOG_FILE_DIR "\\%s", log_file);
#endif
        log = log_path;
    } else {
#ifndef _WIN32
        if (!ACCESS(DFT_LOG_FILE_FLAG, R_OK)) {
#else 
            if (!ACCESS(DFT_LOG_FILE_FLAG, 0)) {
#endif
                log = DFT_LOG_FILE_PATH;
            }
        
    }

    /**
     * open log file if need
     */
    if (log) {
        this->fp = fopen(log, "a");
        if (this->fp) {
            this->log_onoff = 1;
            fprintf(this->fp, "\n");
            fflush(this->fp);
        }
    }

    va_start(list, header);
    while (1) {
        col = va_arg(list, char *);
        if (!col) {
            break;
        }

        type = va_arg(list, char *);
        if (!type) {
            break;
        }

        width = va_arg(list, int);
        this->fmt_len += strlen(type) + number_len(width) + 2;

        total_width += width + 1;
        this->col_cnt++;
    }
    va_end(list);
    this->fmt_len += this->col_cnt + 1;
    total_width--;

    /**
     * malloc memory for col and fmt
     */
    this->col_width = (int *)malloc(this->col_cnt * sizeof(int));
    if (!this->col_width) {
        return 1;
    }
    this->fmt = (char *)malloc(this->fmt_len);
    if (!this->fmt) {
        return 1;
    }
    memset(this->col_width, 0, this->col_cnt * sizeof(int));
    memset(this->fmt,       0, this->fmt_len);
    pfmt = this->cur = this->fmt;

    /**
     * print table header
     */
    len = strlen(header);
    if (total_width < len) {
        total_width = len;
    }
#ifndef _WIN32
    printf("\033[1;39m%*s\n", len + (total_width - len) / 2, header);
#else 
    printf("%*s\n", len + (total_width - len) / 2, header);
#endif
    if (this->log_onoff) {
        fprintf(this->fp, "%*s\n", len + (total_width - len) / 2, header);
        fflush(this->fp);
    }

    /**
     * print separator
     */
    print_separator(stdout, '=', total_width);
    if (this->log_onoff) {
        print_separator(this->fp, '=', total_width);
        fflush(this->fp);
    }

    len = 0;
    va_start(list, header);
    while (1) {
        col = va_arg(list, char *);
        if (!col) {
            break;
        }

        type = va_arg(list, char *);
        if (!type) {
            break;
        }
        ptype = type;

        width = va_arg(list, int);

        last_ch = *(type + strlen(type) - 1);
        if (last_ch == 'f') {
            len = sprintf(pfmt, "%s ", type);
            pfmt += len;
        } else {
            if (*ptype != '%') {
                return 1;
            }

            *pfmt++ = *ptype++;
            len = sprintf(pfmt, "%d", width);
            pfmt += len;
            while ((*pfmt++ = *ptype++) != '\0') {
            }
            pfmt--;
            *pfmt++ = ' ';
        }
        printf("%*s ", width, col);
        if (this->log_onoff) {
            fprintf(this->fp, "%*s ", width, col);
            fflush(this->fp);
        }
    }
    va_end(list);

#ifndef _WIN32
    printf("\033[0m\n");
#else 
    printf("\n");
#endif
    if (this->log_onoff) {
        fprintf(this->fp, "\n");
        fflush(this->fp);
    }

    return 0;
}

METHOD(table_t, show_row_, void, private_table_t *this, ...)
{
    va_list list;

    va_start(list, this);
    vprintf(this->fmt, list);
    va_end(list);
    printf("\n");

    if (this->log_onoff) {
        va_start(list, this);
        vfprintf(this->fp, this->fmt, list);
        va_end(list);

        fprintf(this->fp, "\n");
        fflush(this->fp);
    }
}

METHOD(table_t, show_column_, void, private_table_t *this, ...)
{
    va_list list;
    char    *pfmt = NULL;
    char    ch    = '\0';

    if (*this->cur == '\0' || !this->cur) {
        this->cur = this->fmt;
    }

    pfmt = this->cur;
    if (*this->cur == '%') {
        this->cur++;
    }

    while ((ch = *this->cur++) != '\0') {
        if (ch == '%') {
            break;
        }
    }

    *(--this->cur) = '\0';
    va_start(list, this);
    vprintf(pfmt, list);
    va_end(list);

    if (this->log_onoff) {
        va_start(list, this);
        vfprintf(this->fp, pfmt, list);
        va_end(list);
        fflush(this->fp);
    }

    *this->cur = ch;
    if (ch == '\0') {
        printf("\n");
        if (this->log_onoff) {
            fprintf(this->fp, "\n");
            fflush(this->fp);
        }
    }
}

METHOD(table_t, table_destroy_, void, private_table_t *this)
{
    if (this->col_width) {
        free(this->col_width);
    }
    if (this->fmt) {
        free(this->fmt);
    }
    if (this->fp) {
        fclose(this->fp);
    }
    free(this);
}

table_t *table_create()
{
    private_table_t *this;

#ifndef _WIN32
    INIT(this,
            .pub = {
            .init        = _init_table_,
            .show_row    = _show_row_,
            .show_column = _show_column_,
            .destroy     = _table_destroy_,
            },
            .col_cnt   = 0,
            .col_width = NULL,
            .fmt       = NULL,
            .fp        = NULL,
            .log_onoff = 0,
        );
#else
    INIT(this, private_table_t, 
            {
            init_table_,
            show_row_,
            show_column_,
            table_destroy_,
            },
            0,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
        );
#endif

    return &this->pub;
}

#ifndef _WIN32
typedef enum color_status_t color_status_t;
enum color_status_t {
    COLOR_STATUS_STAERT = 1,
    COLOR_STATUS_PASER,
    COLOR_STATUS_END
};

typedef struct color_info_t color_info_t;
struct color_info_t {
    int id;
    char key;
    char *value;
};
typedef enum color_id_t color_id_t;
enum color_id_t {
    COLOR_ID_BLACK  = 0,
    COLOR_ID_RED    ,
    COLOR_ID_GREEN  ,
    COLOR_ID_YELLOW ,
    COLOR_ID_BLUE   ,
    COLOR_ID_PINK   ,
    COLOR_ID_CYAN   ,
    COLOR_ID_WHITE  ,
    COLOR_ID_NORMAL 
};

typedef enum color_key_t color_key_t;
enum color_key_t {
    COLOR_KEY_BLACK  = 'h',
    COLOR_KEY_RED    = 'r',
    COLOR_KEY_GREEN  = 'g',
    COLOR_KEY_YELLOW = 'y',
    COLOR_KEY_BLUE   = 'b',
    COLOR_KEY_PINK   = 'p',
    COLOR_KEY_CYAN   = 'c',
    COLOR_KEY_WHITE  = 'w',
    COLOR_KEY_NORMAL = 'n'
};

static color_info_t clr_info[] = {
    {COLOR_ID_BLACK,  COLOR_KEY_BLACK,  "\033[30m"},
    {COLOR_ID_RED,    COLOR_KEY_RED,    "\033[31m"},
    {COLOR_ID_GREEN,  COLOR_KEY_GREEN,  "\033[32m"},
    {COLOR_ID_YELLOW, COLOR_KEY_YELLOW, "\033[33m"},
    {COLOR_ID_BLUE,   COLOR_KEY_BLUE,   "\033[34m"},
    {COLOR_ID_PINK,   COLOR_KEY_PINK,   "\033[35m"},
    {COLOR_ID_CYAN,   COLOR_KEY_CYAN,   "\033[36m"},
    {COLOR_ID_WHITE,  COLOR_KEY_WHITE,  "\033[37m"},
    {COLOR_ID_NORMAL, COLOR_KEY_NORMAL, "\033[0m" },
};

static char *parser_format(char *fmt)
{
    char *p       = fmt;
    char *result  = NULL;
    char *presult = NULL;
    int  status   = COLOR_STATUS_STAERT;
    int  clr_cnt  = 0;
    int  len      = 0;
    int  color_id = 0;

    while (*p != '\0') {
        len++;
        switch (status) {
            case COLOR_STATUS_STAERT:
                if (*p == '[') {
                    status = COLOR_STATUS_PASER;
                }
                break;
            case COLOR_STATUS_PASER:
                switch (*p) {
                    case COLOR_KEY_BLACK:
                    case COLOR_KEY_RED:
                    case COLOR_KEY_GREEN:
                    case COLOR_KEY_YELLOW:
                    case COLOR_KEY_BLUE:
                    case COLOR_KEY_PINK:
                    case COLOR_KEY_CYAN:
                    case COLOR_KEY_WHITE:
                    case COLOR_KEY_NORMAL:
                        status = COLOR_STATUS_END;
                        break;
                    default:
                        status = COLOR_STATUS_STAERT;
                        break;
                }
                break;
            case COLOR_STATUS_END:
                clr_cnt++;
                status = COLOR_STATUS_STAERT;
                break;
            default:
                break;
        }
        p++;
    }

    /**
     * malloc memory for format
     */
    len = clr_cnt * 8 + len - clr_cnt * 3 + 1;
    result = (char *)malloc(len);
    if (!result) {
        return NULL;
    }
    memset(result, 0, len);
    presult = result;

    /**
     * parser format
     */
    p = fmt;
    while (*p != '\0') {
        switch (status) {
            case COLOR_STATUS_STAERT:
                if (*p == '[') {
                    status = COLOR_STATUS_PASER;
                }
                *presult++ = *p;
                break;
            case COLOR_STATUS_PASER:
                switch (*p) {
                    case COLOR_KEY_BLACK:
                        color_id = COLOR_ID_BLACK;
                        break;
                    case COLOR_KEY_RED:
                        color_id = COLOR_ID_RED;
                        break;
                    case COLOR_KEY_GREEN:
                        color_id = COLOR_ID_GREEN;
                        break;
                    case COLOR_KEY_YELLOW:
                        color_id = COLOR_ID_YELLOW;
                        break;
                    case COLOR_KEY_BLUE:
                        color_id = COLOR_ID_BLUE;
                        break;
                    case COLOR_KEY_PINK:
                        color_id = COLOR_ID_PINK;
                        break;
                    case COLOR_KEY_CYAN:
                        color_id = COLOR_ID_CYAN;
                        break;
                    case COLOR_KEY_WHITE:
                        color_id = COLOR_ID_WHITE;
                        break;
                    case COLOR_KEY_NORMAL:
                        color_id = COLOR_ID_NORMAL;
                        break;
                    default:
                        *presult++ = *p;
                        break;
                }

                len = sprintf(presult - 1, "%s", clr_info[color_id].value);
                presult += len - 1;
                status = COLOR_STATUS_END;
                break;
            case COLOR_STATUS_END:
                status = COLOR_STATUS_STAERT;
                break;
            default:
                break;
        }
        p++;
    }

    return result;
}

/**
 * @brief color print
 *
 * @param fmt [in] printf format
 * @param ... [in]
 */
void cprintf(char *fmt, ...)
{
    va_list list;
    char *new_fmt = NULL;
    new_fmt = parser_format(fmt);

    va_start(list, fmt);
    vprintf(new_fmt, list);
    va_end(list);

    free(new_fmt);
}
#endif

#ifdef _WIN32
typedef enum win_clr_st_t win_clr_st_t;
enum win_clr_st_t {
    WIN_CLR_ST_NOR = 0,
    WIN_CLR_ST_START,
    WIN_CLR_ST_CLR,
    WIN_CLR_ST_END,
    WIN_CLR_ST_OUT,
};

void set_console_color(unsigned short wAttributes)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        return;

    SetConsoleTextAttribute(hConsole, wAttributes);
}

/**
 * @brief color print
 *
 * @param fmt [in] printf format
 * @param ... [in]
 */
void cprintf(char *fmt, ...)
{
    char c          = '\0';
    char t[3]       = {0};
    char out[1024]  = {0};
    char *p         = NULL;
    int  len        = 0;
    win_clr_st_t st = WIN_CLR_ST_NOR;
    va_list list;

    if (!fmt) {
        return;
    }

    va_start(list, fmt);
    len = vsnprintf(out, sizeof(out), fmt, list);
    out[len] = '\0';
    va_end(list);

    p = out;
    while (*p != '\0') {
        switch (st) {
            case WIN_CLR_ST_NOR:
                if (*p == '[') {
                    t[0] = *p;
                    st = WIN_CLR_ST_START;
                } else {
                    printf("%c", *p);
                }
                break;

            case WIN_CLR_ST_START:
                t[1] = *p;
                st = WIN_CLR_ST_END;
                break;

            case WIN_CLR_ST_END:
                if (*p != ']') {
                    printf("%c%c%c", t[0], t[1], *p);
                    st = WIN_CLR_ST_NOR;
                    break;
                }
                switch (t[1]) {
                    case 'r':
                        set_console_color(FOREGROUND_RED);
                        break;
                    case 'b':
                        set_console_color(FOREGROUND_BLUE);
                        break;
                    case 'g':
                        set_console_color(FOREGROUND_GREEN);
                        break;
                    case 'y':
                        set_console_color(FOREGROUND_RED|FOREGROUND_GREEN);
                        break;
                    case 'c':
                        set_console_color(FOREGROUND_BLUE|FOREGROUND_GREEN);
                        break;
                    case 'n':
                        set_console_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                        break;

                    default:
                        printf("%c%c%c", t[0], t[1], *p);
                        break;
                }
                memset(t, '\0', sizeof(t));
                st = WIN_CLR_ST_NOR;
                break;

            case WIN_CLR_ST_OUT:
                break;

            default:
                break;
        }
        p++;
    }
}

#endif

typedef struct private_progress_t private_progress_t;
struct private_progress_t {
    /**
     * @brief public interface
     */
    progress_t public;

    /**
     * @brief max value of progress
     */
    int max;

    /**
     * @brief progress offset
     */
    int offset;

    /**
     * @brief title of progress
     */
    char *title;

    /**
     * @brief style of progress
     */
    char *style;
};

METHOD(progress_t, init_progress_, void, private_progress_t *this, char *title, int max)
{
    if (title) {
        this->title = title;
    } else {
        this->title = "";
    }
    this->max   = max;
}

METHOD(progress_t, progress_show_, void, private_progress_t *this, int bit)
{
    if (bit > this->max) {
        bit = this->max;
    }

#ifndef _WIN32
    printf("%s \033[?25l\033[42m\033[1m%*s\033[0m %d%%\033[?25h", this->title, bit, " ", 100 * bit / this->max);
#endif
    if (bit >= this->max) {
        printf("\n");
    } else {
        printf("\r");
    }
    fflush(stdout);
}

METHOD(progress_t, progress_destory_, void, private_progress_t *this)
{
    free(this);
}

progress_t *progress_create()
{
    private_progress_t *this = NULL;

#ifndef _WIN32
    INIT(this,
        .public = {
            .init    = _init_progress_,
            .show    = _progress_show_,
            .destroy = _progress_destory_,
        },
    );
#else
    INIT(this, private_progress_t, 
        {
            init_progress_,
            progress_show_,
            progress_destory_,
        },
    );
#endif

    return &this->public;
}
