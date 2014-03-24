#ifndef __MACRO_READER_H__
#define __MACRO_READER_H__

#include <stdio.h>
#include <stdarg.h>

#include "laser_types.h"

typedef char* mr_inst_ptr;

int mr_line_read();
int mr_arc_read();
int mr_zero_counter(char counter);
int mr_comment(const char *format, ...);

int mr_read(mr_inst_ptr &ptr, enum m_commands_t &command_type);
void mr_free(mr_inst_ptr ptr);

int mr_init(const char *file_name);
int mr_close();


#endif /* __MACRO_WRITER_H__ */
