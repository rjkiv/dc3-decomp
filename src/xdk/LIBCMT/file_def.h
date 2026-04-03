#pragma once
#include "types_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long __file_handle;
typedef unsigned long fpos_t;
typedef struct _FILE _FILE, *P_FILE;

#define __ungetc_buffer_size 2

enum __file_kinds {
    __closed_file,
    __disk_file,
    __console_file,
    __unavailable_file
};

enum __open_modes {
    __must_exist,
    __create_if_necessary,
    __create_or_truncate
};

enum __file_orientation {
    __unoriented,
    __char_oriented,
    __wide_oriented
};

enum __io_modes {
    __read = 1,
    __write = 2,
    __read_write = 3,
    __append = 4
};

typedef struct __file_modes {
    unsigned int open_mode : 2;
    unsigned int io_mode : 3;
    unsigned int buffer_mode : 2;
    unsigned int file_kind : 3;

#ifdef _MSL_WIDE_CHAR
    unsigned int file_orientation : 2;
#endif

    unsigned int binary_io : 1;
} __file_modes;

enum __io_states {
    __neutral,
    __writing,
    __reading,
    __rereading
};

typedef struct __file_state {
    unsigned int io_state : 3;
    unsigned int free_buffer : 1;
    unsigned char eof;
    unsigned char error;
} __file_state;

typedef void *__ref_con;
typedef void (*__idle_proc)(void);
typedef int (*__pos_proc)(
    __file_handle file, fpos_t *position, int mode, __ref_con ref_con
);
typedef int (*__io_proc)(
    __file_handle file, unsigned char *buff, size_t *count, __ref_con ref_con
);
typedef int (*__close_proc)(__file_handle file);

typedef struct _iobuf { /* Size=0x20 */
    /* 0x0000 */ char *_ptr;
    /* 0x0004 */ int _cnt;
    /* 0x0008 */ char *_base;
    /* 0x000c */ int _flag;
    /* 0x0010 */ int _file;
    /* 0x0014 */ int _charbuf;
    /* 0x0018 */ int _bufsiz;
    /* 0x001c */ char *_tmpfname;
} FILE;

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2

#ifndef SEEK_SET
/* define standard C file pointer location names */
#define SEEK_SET (0)
#define SEEK_CUR (1)
#define SEEK_END (2)
#endif

#define stdin &(__iob_func()[0])
#define stdout &(__iob_func()[1])
#define stderr &(__iob_func()[2])

#define _STATIC_FILES 4

// extern FILE __files[];
extern FILE *__iob_func(void);

#ifdef __cplusplus
}
#endif
