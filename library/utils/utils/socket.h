#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#else 
#include <WinSock2.h>
#endif

/**
 * socket
 */
#ifndef _WIN32 
typedef int SOCKET;
typedef socklen_t SOCKLEN_T;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in6 SOCKADDR_IN6;
typedef struct sockaddr_storage SOCKADDR_STORAGE;
#else
typedef int SOCKLEN_T;
#endif

#ifndef _WIN32
#define CLOSE(fd) close(fd)
#define IOCTL(fd, cmd, arg) ioctl(fd, cmd, arg)
#else
#define CLOSE(fd) closesocket(fd)
#define IOCTL(fd, cmd, arg) ioctlsocket(fd, cmd, arg)
#endif /* _WIN32 */

#define IOCTL_READ_BYTES(fd, arg) IOCTL(fd, FIONREAD, &arg)

#endif /* __SOCKET_H__ */
