/*
    Copyright (c) 1998--2006 Benhur Stein
    
    This file is part of librastro.

    librastro is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    librastro is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
    for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with librastro; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02111 USA.
*/
#ifndef _RASTRO_H_
#define _RASTRO_H_
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/param.h>          /* for MAXHOSTNAMELEN */
#include <sys/types.h>
#include <sys/types.h>
#include <pthread.h>
#include <dirent.h>
#include <strings.h>
#include <string.h>
#include <rastro_config.h>

#define RST_DEFAULT_BUFFER_SIZE 102400 //in bytes
#define RST_MAX_INPUT_SIZE 1000

#define RST_MICROSECONDS 1000000
#define RST_NANOSECONDS  1000000000
#ifdef HAVE_CLOCKGETTIME
#define RST_CLOCK_RESOLUTION RST_NANOSECONDS
#elif HAVE_GETTIMEOFDAY
#define RST_CLOCK_RESOLUTION RST_MICROSECONDS
#endif
#define RST_EVENT_TYPE_MASK 0x3fff      /* 14 bits */
#define RST_EVENT_INIT (-1 & RST_EVENT_TYPE_MASK)
#define RST_EVENT_STOP (-2 & RST_EVENT_TYPE_MASK)

/*
  Basic keywords for event generation
*/
#define LETTER_UINT8 c
#define LETTER_UINT16 w
#define LETTER_UINT32 i
#define LETTER_UINT64 l
#define LETTER_FLOAT f
#define LETTER_DOUBLE d
#define LETTER_STRING s

#define LETTER_UINT8_QUOTE 'c'
#define LETTER_UINT16_QUOTE 'w'
#define LETTER_UINT32_QUOTE 'i'
#define LETTER_UINT64_QUOTE 'l'
#define LETTER_FLOAT_QUOTE 'f'
#define LETTER_DOUBLE_QUOTE 'd'
#define LETTER_STRING_QUOTE 's'

/*
  CAT concatenates the function
      rst_event_+LETTER_UINT64+LETTER_UINT64+LETTER_STRING+_ptr
*/
#define CAT(x,y,z,w) x##y##y##z##w
#define XCAT(x,y,z,w) CAT(x,y,z,w)
#define STR(x) #x
#define XSTR(x) STR(x)

/* Each event has a type that identifies it */
typedef u_int16_t type_t;
typedef unsigned long long timestamp_t;

typedef struct rst_counters {
  int n_uint8;
  int n_uint16;
  int n_uint32;
  int n_uint64;
  int n_float;
  int n_double;
  int n_string;
} rst_counters_t;

#define RST_MAX_FIELDS_PER_TYPE 15
#define RST_MAX_STRLEN 100

/* Used to read an event from a file */
typedef struct rst_event {
  rst_counters_t ct;
  /* Types read from the event */
  char v_string[RST_MAX_FIELDS_PER_TYPE][RST_MAX_STRLEN];
  u_int8_t v_uint8[RST_MAX_FIELDS_PER_TYPE];
  u_int16_t v_uint16[RST_MAX_FIELDS_PER_TYPE];
  u_int32_t v_uint32[RST_MAX_FIELDS_PER_TYPE];
  u_int64_t v_uint64[RST_MAX_FIELDS_PER_TYPE];
  float v_float[RST_MAX_FIELDS_PER_TYPE];
  double v_double[RST_MAX_FIELDS_PER_TYPE];
  /* The event type */
  type_t type;
  /* Filename ids */
  u_int64_t id1;
  u_int64_t id2;
  double timestamp; //in seconds
  struct rst_file *file; // rst file from which this event was read from
} rst_event_t;

typedef struct rst_ct {
  double a;
  timestamp_t loc0;
  timestamp_t ref0;
} rst_ct_t;

typedef struct rst_file {
  int fd;
  rst_ct_t sync_time;
  char *rst_buffer_ptr;
  char *rst_buffer;
  int rst_buffer_size;
  int rst_buffer_used;
  char *hostname;
  u_int64_t id1;
  u_int64_t id2;
  timestamp_t hour;
  timestamp_t resolution; //clock resolution
  rst_event_t event;
  char *filename; //this filename
  int id; //to be used by the output of rastro_read
} rst_file_t;

typedef struct rst_rastro {
  rst_file_t **files;
  int n;    //active files (with events)

  rst_file_t **emptyfiles;
  int size; //number of empty files
} rst_rastro_t;

/*
 * rst_buffer_t is used only for writing
 */
typedef struct {
  int write_first_hour;
  timestamp_t rst_t0;
  int rst_fd;
  char *rst_buffer_ptr;
  char *rst_buffer;
  size_t rst_buffer_size;
  u_int64_t id1;
  u_int64_t id2;
} rst_buffer_t;

#define RST_OK  (1==1)
#define RST_NOK (0==1)

/*
  Writing Interface
*/

/*
 * Initializes internal structures necessary for librastro. Allocates space for
 * the buffer and opens files for writting. The filename is rastro-id1-id2.rst.
 * Akypuera uses id1 as the rank being traced and id2 as zero.
 */
void rst_init(u_int64_t id1, u_int64_t id2);
/* Same as rst_init, but with an existing buffer pointed to by ptr. */
void rst_init_ptr (rst_buffer_t *ptr, u_int64_t id1, u_int64_t id2);
/*
 * Same as rst_init, but with custom timestamping function (default is
 * clock_gettime when available, else gettimeofday) and resolution (should
 * match the timestamping function).
 */
void rst_init_timestamp (u_int64_t id1,
                         u_int64_t id2,
                         timestamp_t (*stamping) (void),
                         timestamp_t (*resolution) (void));
/* Same as rst_init_timestamp but with an existing buffer pointed to by ptr. */
void rst_init_timestamp_ptr (rst_buffer_t *ptr,
                             u_int64_t id1,
                             u_int64_t id2,
                             timestamp_t (*stamping) (void),
                             timestamp_t (*resolution) (void));
/*
 * Writes the buffer to disk and resets the pointer to the beginning of the
 * buffer, printing a msg to stderr on failure.
 */
void rst_flush(rst_buffer_t * ptr);
/* Flushes the buffer to disk, frees internal structures and closes files. */
void rst_finalize(void);
/* Same as rst_finalize but for a buffer pointed to by ptr. */
void rst_finalize_ptr (rst_buffer_t *ptr);
/*
 * Adds a timestamped event of type type to the event buffer (without any
 * data, such as in the case of rst_event_X)
 */
void rst_event(u_int16_t type);
/* Same as rst_event but adds to an event buffer pointed to by ptr. */
void rst_event_ptr(rst_buffer_t * ptr, u_int16_t type);
/*
 * The following two functions are used by the files automatically generated by
 * librastro and are unlikely to be used otherwise.
 */
/*
 * Timestamps the event with header header and adds it to ptr's buffer, should
 * be followed by a rst_endevent.
 */
void rst_startevent(rst_buffer_t *ptr, u_int32_t header);
/*
 * Aligns the ptr's buffer. Flushes to disk and warns the user in case the
 * buffer exceeded the maximum capacity defined by the environment variable
 * RST_BUFFER_SIZE.
 */
void rst_endevent(rst_buffer_t * ptr);

/*
  Reading Interface
*/
/* All functions that return return RST_OK on success, RST_NOK otherwhise. */
/*
 * Opens filename and attribute it to rastro. Allocate a buffer of size
 * buffer_size for reading. If syncfilename != NULL, processes the timesync
 * file of that name.
 */
int rst_open_file(rst_rastro_t *rastro, int buffer_size, char *filename, char *syncfilename);
/* Read one event from rastro. */
int rst_decode_event(rst_rastro_t *rastro, rst_event_t *event);
/* Closes the file, frees the buffer, etc. */
void rst_close(rst_rastro_t *rastro);
/* Prints information about the event. */
void rst_print_event(rst_event_t *event);

/*
 Generate Interface
*/
/* FIXME These functions do not verify if header is of sufficient length */
int rst_generate_function_header (char *types, char *header, int header_len);
int rst_generate_function_implementation (char *types, char *implem, int implem_len);
/* Generates a header for the definitions file */
int rst_generate_header (char *types[], int types_len, char *header, int header_len);
int rst_generate_functions (char *types[], int types_len, char *implem, int implem_len, char *header_filename);
/* Generates header_name.h and .c files for the types FIXME fixed sz header */
int rst_generate (char *types[], int types_len, FILE *header, FILE *implem, char *header_name);

/*
  These are necessary to generated codes
 */

// Aligns pointer p to 4-byte-aligned address
#define ALIGN_PTR(p) ((void *)(((intptr_t)(p)+(4-1))&(~(4-1))))

/* Adds val to ptr->rst_buffer_ptr and increases it by sizeof(type) */
#define RST_PUT(ptr, type, val)						\
	do {								\
		type *p = (type *)ptr->rst_buffer_ptr;			\
		*p++ = (type)(val);					\
		ptr->rst_buffer_ptr = (char *)p;			\
	} while (0)
#define RST_PUT_STR(ptr, str) 						\
	do {                  						\
		char *__s1 = (char *)ptr->rst_buffer_ptr;		\
		char *__s2 = (char *)str;                               \
		while ((*__s1++ = *__s2++) != '\0') 			\
			;						\
		ptr->rst_buffer_ptr = ALIGN_PTR(__s1); 	\
	} while(0)

#ifndef LIBRASTRO_THREADED
extern rst_buffer_t *rst_global_buffer;
#define RST_PTR (rst_global_buffer)
#define RST_SET_PTR(ptr) (rst_global_buffer = ptr)
#else
extern pthread_key_t rst_key;
#define RST_PTR ((rst_buffer_t *) pthread_getspecific(rst_key))
#define RST_SET_PTR(ptr) pthread_setspecific(rst_key, (void *) ptr)
#endif

#endif                          //_RASTRO_H_
