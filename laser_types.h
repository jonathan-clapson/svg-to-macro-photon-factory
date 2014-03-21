#ifndef __LASER_TYPES_H__
#define __LASER_TYPES_H__

#include <cstdio>
#include <stdarg.h>

/* length to use for each string in a command */
#define M_BUF_LEN 15

/* used to enable or disable the laser during arc and line moving operations */
#define M_LASER_ON 1
#define M_LASER_OFF 0

/* for wait values */
#define M_NO_WAIT "1"
#define M_WAIT "2"

/* 
 * for x/y-mapping and for counter/homing
 * these things are used often enough that
 * conversion would make code a mess 
 */
enum m_axes {
	m_axes_null = 0,
	m_axes_x = 1,
	m_axes_y = 2,
	m_axes_rot = 3,
	m_axes_z = 4,
	m_axes_var_atten = 5,
	m_axes_num_axes = 6
};
#define M_AXES_STR_LEN 2
extern const char m_axes_str [m_axes_num_axes][M_AXES_STR_LEN];

/* 
 * move_commands and move_commands_str are closely tied.
 * make sure your indexes line up if you need to adjust for some reason 
 */
enum m_move_commands_t {
	m_relative = 0,
	m_origin,
	m_absolute,
	m_num_move_commands
};
/* defined in laser_types.cpp */
extern const char m_move_commands_str [m_num_move_commands][M_BUF_LEN];

/* holds instructions converted to character arrays */
struct m_arc_t {
	char one[M_BUF_LEN];
	char x_mapping[M_BUF_LEN];
	char y_mapping[M_BUF_LEN];
	char laser_on[M_BUF_LEN];
	char vector_speed[M_BUF_LEN];
	char vector_accel[M_BUF_LEN];
	char settle_delay[M_BUF_LEN];
	char radius[M_BUF_LEN];
	char start_angle[M_BUF_LEN];
	char end_angle[M_BUF_LEN];
	char repeat_rate[M_BUF_LEN];
	char spacing[M_BUF_LEN];
	char wait[M_BUF_LEN];	
};

/* holds instructions converted to character arrays */
struct m_line_t {
	char coordinate_type[M_BUF_LEN];
	char one[M_BUF_LEN];
	union {
		char vector_speed[M_BUF_LEN];
		char repeat_rate[M_BUF_LEN];
	};
	char vector_accel[M_BUF_LEN];
	char settle_delay[M_BUF_LEN];
	char false_c[M_BUF_LEN];
	char x[M_BUF_LEN];
	char y[M_BUF_LEN];
	char rotation[M_BUF_LEN];
	char spacing[M_BUF_LEN];
	char wait[M_BUF_LEN];	
};

/* type for holding homing and zero instructions */
struct m_home_zero_t {
	union {
		struct {
			char x[M_BUF_LEN];
			char y[M_BUF_LEN];
			char rot[M_BUF_LEN];
			char z[M_BUF_LEN];
			char var_atten[M_BUF_LEN];
		};		
		char values[m_axes_num_axes][M_BUF_LEN];
	};
	
};
typedef struct m_home_zero_t m_home_t;
typedef struct m_home_zero_t m_zero_t;

#endif /* __LASER_TYPES_H__ */
