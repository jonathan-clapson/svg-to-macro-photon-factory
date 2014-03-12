#ifndef __MACRO_WRITER_H__
#define __MACRO_WRITER_H__

int macro_writer_init(const char *file_name);

int macro_writer_move_to(int x, int y);

int macro_writer_write_line(int x0, int y0, int x1, int y1);

int macro_writer_write_arc();

int macro_writer_close();

#endif /* __MACRO_WRITER_H__ */
