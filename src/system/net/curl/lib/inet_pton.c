/* This is from the BIND 4.9.4 release, modified to compile by itself */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "setup.h"

#ifndef HAVE_INET_PTON

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "inet_pton.h"
#define IN6ADDRSZ 16
#define INADDRSZ 4
#define INT16SZ 2
/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static int inet_pton4(const char *src, unsigned char *dst);
#ifdef ENABLE_IPV6
static int inet_pton6(const char *src, unsigned char *dst);
#endif

/* int
 * inet_pton(af, src, dst)
 *      convert from presentation format (which usually means ASCII printable)
 *      to network format (which is usually some kind of binary format).
 * return:
 *      1 if the address was valid for the specified address family
 *      0 if the address wasn't valid (`dst' is untouched in this case)
 *      -1 if some other error occurred (`dst' is untouched in this case, too)
 * notice:
 *      On Windows we store the error in the thread errno, not
 *      in the winsock error code. This is to avoid losing the
 *      actual last winsock error. So use macro ERRNO to fetch the
 *      errno this function sets when returning (-1), not SOCKERRNO.
 * author:
 *      Paul Vixie, 1996.
 */
int Curl_inet_pton(int af, const char *src, void *dst) {
    switch (af) {
    case AF_INET:
        return (inet_pton4(src, (unsigned char *)dst));
    default:
        SET_ERRNO(WSAEAFNOSUPPORT);
        return (-1);
    }
    /* NOTREACHED */
}

/* int
 * inet_pton4(src, dst)
 *      like inet_aton() but without all the hexadecimal and shorthand.
 * return:
 *      1 if `src' is a valid dotted quad, else 0.
 * notice:
 *      does not touch `dst' unless it's returning 1.
 * author:
 *      Paul Vixie, 1996.
 */
static int inet_pton4(const char *src, unsigned char *dst) {
    static const char digits[] = "0123456789";
    int saw_digit, octets, ch;
    unsigned char tmp[INADDRSZ], *tp;

    saw_digit = 0;
    octets = 0;
    tp = tmp;
    *tp = 0;
    while ((ch = *src++) != '\0') {
        const char *pch;

        if ((pch = strchr(digits, ch)) != NULL) {
            unsigned int val = *tp * 10 + (unsigned int)(pch - digits);

            if (saw_digit && *tp == 0)
                return (0);
            if (val > 255)
                return (0);
            *tp = (unsigned char)val;
            if (!saw_digit) {
                if (++octets > 4)
                    return (0);
                saw_digit = 1;
            }
        } else if (ch == '.' && saw_digit) {
            if (octets == 4)
                return (0);
            *++tp = 0;
            saw_digit = 0;
        } else
            return (0);
    }
    if (octets < 4)
        return (0);
    memcpy(dst, tmp, INADDRSZ);
    return (1);
}
#endif /* HAVE_INET_PTON */
