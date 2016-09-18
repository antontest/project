#ifndef __PRINT_H__
#define __PRINT_H__

#define DFT_MENU_BLANK_WIDTH       4
#define DFT_MENU_WIDTH             50
#define DFT_MENU_MULTI_SELECTED    0
#define DFT_MENU_START_INDEX       0U
#define DFT_MAX_MENU_SELECTED_ITEM 20
#define DFT_MENU_HEADER            "Menu"
#define DFT_MENU_SEPARATOR         '='

#ifndef _WIN32
#define DFT_LOG_FILE_DIR           "/tmp"
#define DFT_LOG_FILE_FLAG          "/tmp/skf.txt"
#define DFT_LOG_FILE_PATH          "/tmp/skf_perform.txt"
#else
#define DFT_LOG_FILE_DIR           "d:"
#define DFT_LOG_FILE_FLAG          "d:\\skf.txt"
#define DFT_LOG_FILE_PATH          "d:\\skf_perform.txt"
#endif

typedef struct menu_t menu_t;
struct menu_t  {
	/**
	* @brief init menu 
	*
	* @param header      [in] header tips
	* @param start_index [in] start index for menu 
	* @param is_support_multi_selected [in] is_support_multi_selected
	*/
	void (*init) (menu_t *this, unsigned int menu_width, char *header, unsigned int start_index, unsigned int is_support_multi_selected);

	/**
	* @brief show menu
	*
	* @param ... [in] menu
	*/
	void (*show_menu) (menu_t *this, ...);

	/**
	 * @brief show menu header info
	 *
	 * @param head_info [in] header info
	 */
	void (*show_head) (menu_t *this, char *head_info);

	/**
	 * @brief show menu choice
	 *
	 * @param choice [in] choice
	 */
	void (*show_choice) (menu_t *this, char *choice);

	/**
	 * @brief show menu choice
	 *
	 * @param tips [in] choice
	 */
	void (*show_tips) (menu_t *this, char *tips);

	/**
	* @brief get choices from stdin
	*
	* @param choices[] [out]     choices which selected
	* @param size      [in/out]  buffer size of choices; count of choices selected
	*
	* @return 0, if succ; other, if failed.
	*/
	int (*get_choice) (menu_t *this, int choices[], int *size);

	/**
	* @brief get choices from stdin, return pointer
	*
	* @param choices[] [out]     choices which selected
	* @param size      [in/out]  buffer size of choices; count of choices selected
	*
	* @return 0, if succ; other, if failed.
	*/
	int (*vget_choice) (menu_t *this, int **choices, int *size);

	/**
	* @brief destroy instance and free memory
	*
	* @param 
	*/
	void (*destroy) (menu_t *this);
};

/**
* @brief create menu instance
*/
menu_t *menu_create();

typedef struct table_t table_t;
struct table_t {
	/**
	* @brief init table
	*
	* @param header [in] header of table
	* @param ... [in] colum infor
	*/
	int (*init) (table_t *this, char *log_file, char *header, ...);    

	/**
	* @brief show one line
	*
	* @param ... [in] line info
	*/
	void (*show_row) (table_t *this, ...);

	/**
	* @brief show one colum
	*
	* @param ...
	*/
	void (*show_column) (table_t *this, ...);

	/**
	* @brief destroy instance and free memory
	*/
	void (*destroy) (table_t *this);
};

/**
* @brief create table instance
*/
table_t *table_create();

/**
* @brief color print
*
* @param fmt [in] printf format
* @param ... [in]
*/
void cprintf(char *fmt, ...);

typedef struct progress_t progress_t;
struct progress_t {
    /**
     * @brief init progress bar
     *
     * @param title [in] progress bar title
     * @param max   [in] max length of progress bar
     */
    void (*init) (progress_t *this, char *title, int max);

    /**
     * @brief show progress bar
     *
     * @param bit [in] progress
     */
    void (*show) (progress_t *this, int bit);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (progress_t *this);
};

/**
 * @brief create progress instance
 */
progress_t *progress_create();


#endif /* __PRINT_H__ */
