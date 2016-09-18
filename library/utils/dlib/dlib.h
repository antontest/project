#ifndef __DLIB_H__
#define __DLIB_H__

/**
 * @brief Load api from shared library
 *
 * @param lib_name       [in] library name, can be NULL.
 * @param func_list_name [in] func name, can not be NULL.
 * @param func_hdl       [out] return value, list handler
 *
 * @return 0, if succ; other, if failed.
 */
int load_library(char *lib_name, char *func_list_name, void **func_hdl);

#endif /* __DLIB_H__ */
