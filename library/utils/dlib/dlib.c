#include <stdio.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <dlfcn.h>
#else 
#include <windows.h>
#endif

/**
 * @brief Load api from shared library
 *
 * @param lib_name       [in] library name, can be NULL.
 * @param func_list_name [in] func name, can not be NULL.
 * @param func_hdl       [out] return value, list handler
 *
 * @return 0, if succ; other, if failed.
 */
int load_library(char *lib_name, char *func_list_name, void **func_hdl)
{
    int  ret           = 0;
    void *lib_hdl      = NULL;
    char lib_path[256] = {0};
#ifdef _WIN32
    char    *pos     = NULL;
    FARPROC list_hdl = NULL;
#else
    int (*list_hdl) (void **) = NULL;
#endif

    if (!func_hdl) {
        return 1;
    }

#ifndef _WIN32
    if (!getcwd(lib_path, sizeof(lib_path))) {
        printf("getcwd() failed.\n");
        return 1;
    }
    strcat(lib_path, "/");
    strcat(lib_path, lib_name);

    lib_hdl = dlopen(lib_path, RTLD_LAZY);
    if (!lib_hdl) {
        printf("dlopen() failed: %s.\n", dlerror());
        return 1;
    }

    list_hdl = dlsym(lib_hdl, func_list_name);
    if (!list_hdl) {
        printf("dlsym() failed: %s.\n", dlerror());
        return 1;
    }
    ret = list_hdl(func_hdl);
#else 

    GetModuleFileName(NULL, lib_path, sizeof(lib_path));
    pos = strrchr(lib_path, '\\');
    if (!pos) {
        return 1;
    }
    *(++pos) = '\0';
    strcat(lib_path, lib_name);

    lib_hdl = LoadLibrary(lib_path);
    if (!lib_hdl) {
        ret = GetLastError();
        printf("LoadLibrary() failed: %d\n", ret);
        return ret;
    }

    list_hdl = GetProcAddress(lib_hdl, func_list_name);
    if (!list_hdl) {
        ret = GetLastError();
        printf("GetProcAddress() failed: %d\n", ret);
        return 1;
    }
    ret = list_hdl(func_hdl);
#endif

    return ret;
}

