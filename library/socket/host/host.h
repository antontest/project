/**
 * @defgroup host host
 * @{ @ingroup networking
 */

#ifndef __SOCKET_HOST_H__
#define __SOCKET_HOST_H__

typedef struct host_t host_t;

#ifndef _WIN32
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <utils/utils.h>
#include <utils/socket.h>
#else
#include <WinSock2.h>
#include "utils.h"
#include "socket.h"
#endif

/**
 * Representates a Host
 *
 * Host object, identifies a address:port pair and defines some
 * useful functions on it.
 */
struct host_t {

	/**
	 * Build a clone of this host object.
	 *
	 * @return		cloned host
	 */
	host_t *(*clone) (host_t *this);

	/**
	 * Get a pointer to the internal sockaddr struct.
	 *
	 * This is used for sending and receiving via sockets.
	 *
	 * @return		pointer to the internal sockaddr structure
	 */
	SOCKADDR *(*get_sockaddr) (host_t *this);

	/**
	 * Get the length of the sockaddr struct.
	 *
	 * Depending on the family, the length of the sockaddr struct
	 * is different. Use this function to get the length of the sockaddr
	 * struct returned by get_sock_addr.
	 *
	 * This is used for sending and receiving via sockets.
	 *
	 * @return		length of the sockaddr struct
	 */
	SOCKLEN_T *(*get_sockaddr_len) (host_t *this);

	/**
	 * Gets the family of the address
	 *
	 * @return		family
	 */
	int (*get_family) (host_t *this);

	/**
	 * Checks if the ip address of host is set to default route.
	 *
	 * @return		TRUE if host is 0.0.0.0 or 0::0, FALSE otherwise
	 */
	int (*is_anyaddr) (host_t *this);

	/**
	 * Get the port of this host
	 *
	 * @return		port number
	 */
	unsigned short (*get_port) (host_t *this);

	/**
	 * Set the port of this host
	 *
	 * @param port	port number
	 */
	void (*set_port) (host_t *this, unsigned short port);

	/**
	 * Get the port of this host
	 *
	 * @return		port number
	 */
	char* (*get_ip) (host_t *this, char **ip, int size);

	/**
	 * Set the port of this host
	 *
	 * @param port	port number
	 */
	void (*set_ip) (host_t *this, const char *ip);

	/**
	 * Compare the ips of two hosts hosts.
	 *
	 * @param other	the other to compare
	 * @return		TRUE if addresses are equal.
	 */
	int (*ip_equals) (host_t *this, host_t *other);

	/**
	 * Compare two hosts, with port.
	 *
	 * @param other	the other to compare
	 * @return		TRUE if addresses and ports are equal.
	 */
	int (*equals) (host_t *this, host_t *other);

	/**
	 * Destroy this host object.
	 */
	void (*destroy) (host_t *this);
};

/**
 * Constructor to create a host_t object from an address string.
 *
 * @param string		string of an address, such as "152.96.193.130"
 * @param port			port number
 * @return				host_t, NULL if string not an address.
 */
host_t *host_create_from_string(char *string, unsigned short port);

/**
 * Same as host_create_from_string(), but with the option to enforce a family.
 *
 * @param string		string of an address
 * @param family		address family, or AF_UNSPEC
 * @param port			port number
 * @return				host_t, NULL if string not an address.
 */
host_t *host_create_from_string_and_family(char *string, int family, unsigned short port);

/**
 * Constructor to create a host_t from a DNS name.
 *
 * @param string		hostname to resolve
 * @param family		family to prefer, 0 for first match
 * @param port			port number
 * @return				host_t, NULL lookup failed
 */
host_t *host_create_from_dns(char *string, int family, unsigned short port);

/**
 * Constructor to create a host_t object from a sockaddr struct
 *
 * @param sockaddr		sockaddr struct which contains family, address and port
 * @return				host_t, NULL if family not supported
 */
host_t *host_create_from_sockaddr(struct sockaddr *sockaddr);

/**
 * Create a host from a CIDR subnet definition (1.2.3.0/24), return bits.
 *
 * @param string		string to parse
 * @param bits			gets the number of network bits in CIDR notation
 * @return				network start address, NULL on error
 */
host_t *host_create_from_subnet(char *string, int *bits);

/**
 * Create a netmask host having the first netbits bits set.
 *
 * @param family		family of the netmask host
 * @param netbits		number of leading bits set in the host
 * @return				netmask host
 */
host_t *host_create_netmask(int family, int netbits);

/**
 * Create a host without an address, a "any" host.
 *
 * @param family		family of the any host
 * @return				host_t, NULL if family not supported
 */
host_t *host_create_any(int family);

#endif /** HOST_H_ @}*/

