// SPDX-License-Identifier: GPL-3.0-only

#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "system.h"

/* Debug output */

static int verbosity = DEBUG_LEVEL;

void s_dprintf(int level, const char *fmt, ...)
{
	va_list ap;
	if (level > verbosity)
		return;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void s_set_verbosity(int level)
{
	verbosity = level;
}

/* Sockets */

int s_connect(int *sockfd, const char *addr, int port)
{
	int fd, ret;
	struct sockaddr_in serv_addr = {0};

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return -errno;

	*sockfd = fd;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	ret = inet_pton(AF_INET, addr, &serv_addr.sin_addr);
	if (ret <= 0)
		return ret ? -errno : -EINVAL;

	ret = connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	if (ret) {
		close(fd);
		return -errno;
	}
	return 0;
}

ssize_t s_send(int sockfd, void *buf, size_t len, int flags)
{
	ssize_t ret = send(sockfd, buf, len, flags);
	return ret < 0 ? -errno : ret;
}

ssize_t s_recv(int sockfd, void *buf, size_t len, int flags)
{
	ssize_t ret = 0;
	size_t rcv_len = 0;
	int attempt = 100;

	while (rcv_len < len && attempt--) {
		ret = recv(sockfd, buf, len - rcv_len, flags);
		if (ret > 0)
			rcv_len += ret;
		if (ret < 0)
			return -errno;
	}

	return (ret < 0 || !attempt) ? -errno : ret;
}

int s_flush(int sockfd)
{
	ssize_t tmp, ret = 0;
	int len;
	uint8_t *buf;

	ret = ioctl(sockfd, FIONREAD, &len);
	s_dprintf(SPEW, "flush len is %d\n", len);

	/* FIXME: can oom? */
	buf = malloc(len);

	do {
		tmp = recv(sockfd, buf, len, MSG_DONTWAIT);
		ret += tmp;
	} while (tmp >= 0);

	ret -= tmp; /* error code */

	free(buf);
	return ret;
}

int s_close(int fd)
{
	return close(fd) ? -errno : 0;
}
