/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2011, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#include "setup.h"

#include <curl/curl.h>
#include "urldata.h"
#include "share.h"
#include "sslgen.h"
#include "curl_memory.h"

/* The last #include file should be: */
#include "memdebug.h"

#undef curl_share_setopt

CURLSHcode curl_share_cleanup(CURLSH *sh) {
    struct Curl_share *share = (struct Curl_share *)sh;

    if (share == NULL)
        return CURLSHE_INVALID;

    if (share->lockfunc)
        share->lockfunc(
            NULL, CURL_LOCK_DATA_SHARE, CURL_LOCK_ACCESS_SINGLE, share->clientdata
        );

    if (share->dirty) {
        if (share->unlockfunc)
            share->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, share->clientdata);
        return CURLSHE_IN_USE;
    }

    if (share->hostcache) {
        Curl_hash_destroy(share->hostcache);
        share->hostcache = NULL;
    }

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_COOKIES)
    if (share->cookies)
        Curl_cookie_cleanup(share->cookies);
#endif

#ifdef USE_SSL
    if (share->sslsession) {
        unsigned int i;
        for (i = 0; i < share->nsslsession; ++i)
            Curl_ssl_kill_session(&(share->sslsession[i]));
        free(share->sslsession);
    }
#endif

    if (share->unlockfunc)
        share->unlockfunc(NULL, CURL_LOCK_DATA_SHARE, share->clientdata);
    free(share);

    return CURLSHE_OK;
}

CURLSHcode Curl_share_lock(
    struct SessionHandle *data, curl_lock_data type, curl_lock_access accesstype
) {
    struct Curl_share *share = data->share;

    if (share == NULL)
        return CURLSHE_INVALID;

    if (share->specifier & (1 << type)) {
        if (share->lockfunc) /* only call this if set! */
            share->lockfunc(data, type, accesstype, share->clientdata);
    }
    /* else if we don't share this, pretend successful lock */

    return CURLSHE_OK;
}

CURLSHcode Curl_share_unlock(struct SessionHandle *data, curl_lock_data type) {
    struct Curl_share *share = data->share;

    if (share == NULL)
        return CURLSHE_INVALID;

    if (share->specifier & (1 << type)) {
        if (share->unlockfunc) /* only call this if set! */
            share->unlockfunc(data, type, share->clientdata);
    }

    return CURLSHE_OK;
}
