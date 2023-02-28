/* SPDX-License-Identifier: GPL-3.0-only */

/*
 * system.h - small and simple OS abstraction layer.
 *
 * This modle is intended to simplify the porting of the library
 * that is built on it. It provides various methods like a small
 * wrapper around sockets (to hide e.g. winsock quirks shall the
 * library be ported to Windows), debug print helpers and so on.
 * Endianness conversion functions are also provided in endianness.h
 *
 * The implementations are provided in platform-specific C files.
 */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>

/* === Debug output === */

#if __GNUC__
#define __PRINTFLIKE(__fmt,__varargs) \
	__attribute__((__format__ (__printf__, __fmt, __varargs)))
#else
#define __PRINTFLIKE(__fmt,__varargs)
#endif

/* Debug levels to use with print functions. */
#define ALWAYS 0
#define CRITICAL 0
#define INFO 1
#define SPEW 2

#ifndef DEBUG_LEVEL
	#define DEBUG_LEVEL 0
#endif

/**
 * s_dprintf() - Debug printf with verbosity level.
 * @level:	Debug level.
 * @fmt:	Printf style format.
 * @...:	Values to print.
 *
 * The message will be printed to the platform's debug output
 * (e.g. stderr) if the level is less or equal of current
 * verbosity level. See s_set_verbosity() to change the level.
 */
void s_dprintf(int level, const char *fmt, ...) __PRINTFLIKE(2, 3);

/**
 * s_ddump_data() - Dump bytes from a buffer.
 * @level:      Debug level.
 * @prefix:     Prefix every line with this string.
 * @data:       Data to dump.
 * @len:        Amount of bytes to dump.
 */
static inline void s_ddump_data(int level, const char *prefix, const void *data, size_t len)
{
	size_t i;

	for (i = 0; i < len; ++i) {
		if (i % 16 == 0)
			s_dprintf(level, "\n%s 0x%04lx | ", prefix, i);
		s_dprintf(level, "%2x ", ((uint8_t*)data)[i]);
	}
	s_dprintf(level, "\n");
}

/**
 * s_set_verbosity() - Set verbosity level.
 * @level:	The level value to set.
 *
 * The default level is defined at compile time.
 */
void s_set_verbosity(int level);

/* === Sockets === */

/**
 * s_connect() - Create and connect an INET socket
 * @sockfd:	Pointer to an int to save the file descriptor.
 * @addr:	IP address string to connect to.
 * @port:	Remote port to connect to.
 *
 * The function will try to create and open a socket to the
 * given IP address and save the result to sockfd.
 *
 * Return: Zero or success, negative errno otherwise.
 */
int s_connect(int *sockfd, const char *addr, int port);

/**
 * s_send() - Send messages to the socket.
 * @sockfd:	File descriptor.
 * @buf:	Data buffer.
 * @len:	Buffer size.
 * @flags:	send flags.
 *
 * This function is almost identical to the classic
 * send with an exception of the error handling.
 *
 * Return: Data length on success or negative errno.
 */
ssize_t s_send(int sockfd, void *buf, size_t len, int flags);

/**
 * s_recv() - Receive messages from a socket.
 * @sockfd:	File descriptor.
 * @buf:	Data buffer.
 * @len:	Buffer size.
 * @flags:	recv flags.
 *
 * This function is almost identical to the classic
 * recv with an exception of the error handling.
 * It also guarantees the receival of exactly len bytes
 * if no error happens.
 *
 * Return: Data length on success or negative errno.
 */
ssize_t s_recv(int sockfd, void *buf, size_t len, int flags);

/**
 * s_flush() - Delete all pending data on the socket.
 * @sockfd:	File descriptor.
 *
 * This function is supposed to destroy pending data
 * (by e.g. receiving it until nothing is left in the
 * queue).
 *
 * Return: Bytes flushed or negative errno.
 */
int s_flush(int sockfd);

/**
 * s_close() - Close a file descriptor.
 * @fd:     File descriptor.
 *
 * Return: Zero or success, negative errno otherwise.
 */
int s_close(int fd);

/* == Serial ports == */

/**
 * s_serial() - Open a serial port and configure it.
 * @fd:     Pointer to the file descriptor to be returned.
 * @name:   Name of the device (i.e. tty file name).
 * @speed:  Baud rate to configure.
 * @parity: Parity mode to configure.
 *
 * Return: 0 on success and negative errno otherwise.
 */
int s_serial(int *fd, const char *name, int speed, int parity);

/** s_read() - Read from a file descriptor.
 * @fd:     File descriptor.
 * @buf:    Pointer to data buffer.
 * @count:  Amount of bytes to read.
 *
 * The function will guarantee a read of @count bytes.
 * Return: Amount of bytes read or negative errno.
 */
int s_read(int fd, void *buf, size_t count);

#endif /* SYSTEM_H */

