#ifndef __UDP_H__
#define __UDP_H__
#include <sys/socket.h>

typedef struct udp_t udp_t;
struct udp_t {
    /**
     * @brief create socket
     *
     * @param family  [in] AF_INET, AF_INET6
     * @param ip      [in] ip address 
     * @param port    [in] port
     * @return         socket fd, if succ; -1, if failed
     */
    int (*socket) (udp_t *this, int family);

    /**
     * @brief server bind
     * @return 0, if succ; -1, if failed;
     */
    int (*bind) (udp_t *this, char *ip, int port);

    /**
     * @brief connect to server 
     * @return 0, if succ; -1, if failed;
     */
    int (*connect) (udp_t *this);

    /**
     * @brief sendto message
     *
     * @param buf      [in] message buffer 
     * @param size     [in] size of message buffer 
     * @param dst_ip   [in] ip address of server 
     * @param dst_port [in] port of server listening
     * @return         count of message sended, if succ; -1, if failed
     */
    int (*sendto) (udp_t *this, void *buf, int size, char *dst_ip, int dst_port);

    /**
     * @brief send message
     *
     * @param buf  [in] message buffer
     * @param size [in] size of message
     * @return     count of message sended, if succ; -1, if failed;
      */
    int (*send) (udp_t *this, void *buf, int size);

    /**
     * @brief recvfrom message
     *
     * @param buf      [out] message buffer 
     * @param size     [in]  size of buffer 
     * @param src_ip   [in]  source ip address 
     * @param src_port [in]  source port
     * @return         count of message recved, if succ; -1, if failed
     */
    int (*recvfrom) (udp_t *this, void *buf, int size, char *src_ip, int src_port);

    /**
     * @brief recv message
     *
     * @param buf  [out] message buffer
     * @param size [in]  size of message
     * @return     count of message recved, if succ; -1, if failed;
     */
    int (*recv) (udp_t *this, void *buf, int size);
    
    /**
     * @brief close tcp connection
     */
    int (*close) (udp_t *this);

    /**
     * @brief destroy instance and free memory
     */
    void (*destroy) (udp_t *this);
};

/**
 * @brief create udp instance
 */
udp_t *udp_create();

#endif /* __UDP_H__ */
