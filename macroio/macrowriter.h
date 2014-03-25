#ifndef __MACRO_WRITER_H__
#define __MACRO_WRITER_H__

#include <stdio.h>
#include <stdarg.h>

#include "laser_types.h"

int mw_svgedit_helper_draw_line(int x0, int y0, int x1, int y1);
int mw_svgedit_helper_draw_circle(long radius, long x, long y);

int mw_line_init(struct m_line_t &line);
int mw_line_populate(enum m_commands_t coord_type, struct m_line_t &line, long x_nm, long y_nm, unsigned laser_on);
int mw_line_exec(struct m_line_t line);

int mw_arc_init(struct m_arc_t &arc);
int mw_arc_populate(struct m_arc_t &arc, long radius, double start_angle, double end_angle, unsigned laser_on);
int mw_arc_exec(struct m_arc_t arc);

int mw_beam_align_exec(m_beam_align_t& beam_align);
int mw_beam_align_populate(struct m_beam_align_t &beam_align, enum m_beam_align_commands_t command, enum m_beam_align_values_t value);
int mw_beam_align_init(m_beam_align_t& beam_align);

int mw_zero_counters_init(m_zero_t& zero);
int mw_zero_counters_populate(m_zero_t& zero, int n_args, ...);
int mw_zero_counters_exec(m_zero_t& zero);

int mw_home_stage_init(m_home_t& home);
int mw_home_stage_populate(m_home_t& home, int n_args, ...);
int mw_home_stage_exec(m_home_t& home);

int mw_comment(const char *format, ...);

int mw_init(const char *file_name);
int mw_close();


#endif /* __MACRO_WRITER_H__ */
