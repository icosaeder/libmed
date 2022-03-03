
/*
 * endiannes.h - a very small helper for endianness conversion
 */

#ifndef ENDIANNES_H
#define ENDIANNES_H

#include <stdint.h>
#include <arpa/inet.h>

/* FIXME: At the moment host is assumed to be LE system */

/* Endiannes-specific types. */
typedef uint16_t __le16;
typedef uint32_t __le32;

typedef uint16_t __be16;
typedef uint32_t __be32;

#if defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
	#error "Only LE targets are supported, sorry :("
#endif

#define cpu_to_be16 htons
#define cpu_to_be32 htonl
#define be16_to_cpu ntohs
#define be32_to_cpu ntohl

#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)

#endif /* ENDIANNES_H */
