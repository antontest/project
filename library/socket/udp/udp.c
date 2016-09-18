#include <udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <utils/utils.h>
#include <host/host.h>

typedef struct private_udp_t private_udp_t;
struct private_udp_t {
    /**
     * @brief public interface
     */
    udp_t public;

    /**
     * @brief socket fd
     */
    int fd;

    /**
     * socket family
     */
    int family;

    /**
     * @brief socket host
     */
    host_t *host;
};
#define udp_fd     this->fd
#define udp_host   this->host
#define udp_famliy this->family

METHOD(udp_t, socket_, int, private_udp_t *this, int family)
{
    /**
     * close socket if created
     */
    if (udp_fd > 0) close(udp_fd);

    /**
     * save family
     */
    if (family >= 0) {
        udp_famliy = family;
    }

    /**
     * create udp socket instance
     */
    udp_fd = socket(udp_famliy, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("socket()");
        return -1;
    }

    return udp_fd;
}

METHOD(udp_t, bind_, int, private_udp_t *this, char *ip, int port)
{
    int ret = -1;

    /**
     * create host
     */
    if (udp_host) udp_host->destroy(udp_host);
    udp_host = host_create_from_string_and_family(ip ? ip : "%any", udp_famliy, port);
    if (!udp_host) {
        printf("create host failed\n");
        return -1;
    }

    ret = bind(udp_fd, udp_host->get_sockaddr(udp_host), sizeof(struct sockaddr));
    if (ret < 0) perror("bind()");
    return ret;
}

METHOD(udp_t, connect_, int, private_udp_t *this)
{
    int ret = connect(udp_fd, udp_host->get_sockaddr(udp_host), sizeof(struct sockaddr));
    if (ret < 0) perror("bind()");
    return ret;
}

METHOD(udp_t, sendto_, int, private_udp_t *this, void  *buf, int size, char *dst_ip, int dst_port)
{
    int ret          = 0;
    host_t *dst_host = NULL;

    if (dst_ip == NULL || dst_port < 0) return -1;

    /**
     * create destination host
     */
    dst_host = host_create_from_string_and_family(dst_ip ? dst_ip : "%any", udp_famliy, dst_port);

    /**
     * sendto message
     */
    ret = sendto(udp_fd, buf, size, 0, dst_host->get_sockaddr(dst_host), sizeof(struct sockaddr));
    if (ret < 0) perror("sendto()");
    if (dst_host) dst_host->destroy(dst_host);

    return ret;
}

METHOD(udp_t, send_, int, private_udp_t *this, void *buf, int size)
{
    int ret = send(udp_fd, buf, size, 0);
    if (ret < 0) perror("send()");
    return ret;
}

METHOD(udp_t, recvfrom_, int, private_udp_t *this, void  *buf, int size, char *src_ip, int src_port)
{
    int ret          = 0;
    host_t *host     = NULL;
    host_t *src_host = NULL;

    /**
     * create destination host
     */
    if (src_ip && src_port > 0) {
        src_host = host_create_from_string_and_family(src_ip, udp_host->get_family(udp_host), src_port);
        host = src_host;
    } else {
        host = udp_host;
    }

    /**
     * recvfrom message
     */
    ret = recvfrom(udp_fd, buf, size, 0, host->get_sockaddr(host), host->get_sockaddr_len(host));
    if (ret < 0) perror("sendto()");
    if (src_host) src_host->destroy(src_host);

    return ret;
}

METHOD(udp_t, recv_, int, private_udp_t *this, void *buf, int size)
{
    int ret = recv(udp_fd, buf, size, 0);
    if (ret < 0) perror("recv()");
    return ret;
}

METHOD(udp_t, close_, int, private_udp_t *this)
{
    return close(udp_fd);
}

METHOD(udp_t, destroy_, void, private_udp_t *this)
{
    if (udp_fd > 0) close(udp_fd);
    if (udp_host) udp_host->destroy(udp_host);
    free(this);
}

udp_t *udp_create()
{
    private_udp_t *this;

    INIT(this, 
        .public = {
        .socket   = _socket_,
        .bind     = _bind_,
        .connect  = _connect_,
        .close    = _close_,
        .destroy  = _destroy_,

        .sendto   = _sendto_,
        .send     = _send_,
        .recvfrom = _recvfrom_,
        .recv     = _recv_,
        },
        .fd     = -1,
        .host   = NULL,
        .family = AF_INET,
    );

    return &this->public;
}
