#include <stdarg.h>
#include <cstring>
#include <cstdio>

#include "macrowriter.h"
#include "macroerror.h"

FILE *fp;
char *name;

#define LINE_END "\r\n"

/* FIXME: the following two helper functions shouldn't be doing coordinate converstion */

/** 
 * helper function to be able to write lines in a more traditional line drawing format
 */
int mw_svgedit_helper_draw_line(int x0, int y0, int x1, int y1)
{
	struct m_line_t line;
	
	/*
	 * svgedit uses coordinates with (0,0) being top left as defined in svg spec 
	 * laser uses coordinates with (0,0) being the center of the stage 
	 */
	x0 -= 100000;
	y0 -= 75000;
	x1 -= 100000;
	y1 -= 75000;
	
	/* 
	 * svgedit uses 1 unit = 1 micrometer. 
	 * (This isn't in svn unit specs which is why it's left as a 'unit' rather than a distance)
	 * laser functions expect nanometers
	 */	
	mw_line_populate(m_absolute, line, x0*1000, y0*1000, M_LASER_OFF);
	mw_line_exec(line);
	
	mw_line_populate(m_absolute, line, x1*1000, y1*1000, M_LASER_ON);
	mw_line_exec(line);
	
	return M_ERR_SUCCESS;
}

/** 
 * helper function to be able to arcs in a more traditional line drawing format
 */
int mw_svgedit_helper_draw_circle(long radius, long x, long y)
{
	struct m_line_t line;
	
	/*
	 * svgedit uses coordinates with (0,0) being top left as defined in svg spec 
	 * laser uses coordinates with (0,0) being the center of the stage 
	 */	
	x -= 100000;
	y -= 100000;
	
	/* need to shift stage so we are cutting the line not the center */
	x += radius;
	
	mw_line_populate(m_absolute, line, x*1000, y*1000, M_LASER_OFF);
	mw_line_exec(line);	
	
	double start_angle = 0;
	double end_angle = 360;
	
	struct m_arc_t arc;
	mw_arc_populate(arc, radius*1000, start_angle, end_angle, M_LASER_ON);
	mw_arc_exec(arc);
	
	return M_ERR_SUCCESS;
}

/** 
 * initialises a line structure
 * 
 * @param[in] arc reference to an arc_t to initialise
 * 
 * @return returns 0
 */
int mw_line_init(struct m_line_t &line)
{
	/* RelMove;1;;Disabled;Disabled;Disabled;False;Disabled;Disabled;Disabled;Disabled;2 */
	/* relative movement is probably the least intrusive in case of screw up, use that by default */
	strncpy(line.coordinate_type, m_move_commands_str[m_relative], M_BUF_LEN); 
	strncpy(line.one, "1", M_BUF_LEN);
	strncpy(line.vector_speed, "Disabled", M_BUF_LEN);
	strncpy(line.vector_accel, "Disabled", M_BUF_LEN);
	strncpy(line.settle_delay, "0", M_BUF_LEN);
	strncpy(line.false_c, "False", M_BUF_LEN);
	strncpy(line.x, "Disabled", M_BUF_LEN);
	strncpy(line.y, "Disabled", M_BUF_LEN);
	strncpy(line.rotation, "Disabled", M_BUF_LEN);
	strncpy(line.spacing, "Disabled", M_BUF_LEN);
	strncpy(line.wait, "2", M_BUF_LEN);
	
	return M_ERR_SUCCESS;
}

/** 
 * initialises an arc structure
 * 
 * @param[in] arc reference to an arc_t to initialise
 * 
 * @return returns 0
 */
int mw_arc_init(struct m_arc_t &arc) {
	/* ArcMove;1;1;2;False;;299999997;1999999980;0;0;0.0000;0.0000;;;2 */
	strncpy(arc.one, "1", M_BUF_LEN);
	strncpy(arc.x_mapping, "Disabled", M_BUF_LEN);
	strncpy(arc.y_mapping, "Disabled", M_BUF_LEN);
	strncpy(arc.laser_on, "False", M_BUF_LEN);
	strncpy(arc.vector_speed, "299999997", M_BUF_LEN);
	strncpy(arc.vector_accel, "1999999980", M_BUF_LEN);	
	strncpy(arc.settle_delay, "0", M_BUF_LEN);
	strncpy(arc.radius, "0", M_BUF_LEN);
	strncpy(arc.start_angle, "0.0000", M_BUF_LEN);
	strncpy(arc.end_angle, "0.0000", M_BUF_LEN);
	strncpy(arc.repeat_rate, "", M_BUF_LEN);
	strncpy(arc.spacing, "", M_BUF_LEN);
	strncpy(arc.wait, "1", M_BUF_LEN);
	
	return M_ERR_SUCCESS;
}

/**
 * Initialises a home stage structure
 *
 * @param[in]	home a home structure to initialise
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_home_stage_init(m_home_t& home)
{
	memset(&home, 0, sizeof(m_home_t));
	return M_ERR_SUCCESS;
}

/**
 * Initialises a zero counter structure
 *
 * @param[in]	home a home structure to initialise
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_zero_counters_init(m_zero_t& zero)
{
	return mw_home_stage_init(zero);
}

/**
 * Configures a struct line_t to move linearly
 * 
 * Coordinate system (0,0) is center of stage
 *
 * @param[in]	line a reference to a struct line_t to populate
 * @param[in]	x the start x coordinate in nanometers
 * @param[in]	y the start y coordinate in nanometers 
 * @param[in]	laser_on whether the laser should be on while this move is performed
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_line_populate(enum m_commands_t coord_type, struct m_line_t &line, long x_nm, long y_nm, unsigned laser_on)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;	
		
	mw_line_init(line);
	
	printf("type: %s\n", m_move_commands_str[coord_type]);
	
	strncpy(line.coordinate_type, m_move_commands_str[coord_type], M_BUF_LEN);
	/* if its a relative move sometimes we want to not move in one coordinate and move in the other, in this case we leave one coordinate disabled */
	if ( (coord_type != m_relative) || (x_nm != 0) ) {
		snprintf(line.x, M_BUF_LEN, "%ld", x_nm);
		strncpy(line.vector_accel, "1500000000", M_BUF_LEN);
		if (laser_on == M_LASER_ON) {
			strncpy(line.repeat_rate , "500", M_BUF_LEN);
		} else {
			strncpy(line.vector_speed, "200000000", M_BUF_LEN);
			strncpy(line.vector_accel, "1500000000", M_BUF_LEN);			
		}
	}
	if ( (coord_type != m_relative) || (y_nm != 0) ) {		
		snprintf(line.y, M_BUF_LEN, "%ld", y_nm);
		strncpy(line.vector_accel, "1500000000", M_BUF_LEN);
		if (laser_on == M_LASER_ON) {
			strncpy(line.repeat_rate , "500", M_BUF_LEN);
		} else {
			strncpy(line.vector_speed, "200000000", M_BUF_LEN);
		}
	}
	if (laser_on) {
		strncpy(line.repeat_rate, "500", M_BUF_LEN);
		strncpy(line.spacing, "10000", M_BUF_LEN);
	}	
	
	return M_ERR_SUCCESS;
}

/** 
 * constructs an arc structure with parameters
 * 
 * @param[in] arc reference to an arc_t to initialise and fill in
 * @param[in] radius the radius of the arc in nano meters
 * @param[in] start_angle the angle to start the arc from (x axis = 0 degrees)
 * @param[in] end_angle the angle to end the arc at (x axis = 0 degrees)
 * 
 * @return returns 0
 */
int mw_arc_populate(struct m_arc_t &arc, long radius, double start_angle, double end_angle, unsigned laser_on)
{
	mw_arc_init(arc);
	
	strncpy(arc.x_mapping, m_axes_str[m_axes_x], M_BUF_LEN);
	strncpy(arc.y_mapping, m_axes_str[m_axes_y], M_BUF_LEN);
	
	if (start_angle != 0.0) {
		snprintf(arc.start_angle, M_BUF_LEN, "%f", start_angle);
	}
	if (end_angle != 0.0) {
		snprintf(arc.end_angle, M_BUF_LEN, "%f", end_angle);
	}
	if (arc.radius != 0) {
		snprintf(arc.radius, M_BUF_LEN, "%ld", radius);
	}
	if (laser_on) {
		strncpy(arc.laser_on, "True", M_BUF_LEN);
		strncpy(arc.spacing, "10000", M_BUF_LEN);
		strncpy(arc.repeat_rate, "500", M_BUF_LEN);
	}
	
	return M_ERR_SUCCESS;
}

/**
 * Fills out a home stages/zero counter/home_zero structure
 *
 * @param[in]	home_zero a home stages/zero counter/home_zero structure to fill out
 * @param[in]	n_args the number of axes that are being set with this call
 * @param[in]	args a va_list of axes args
 * 
 * @return returns M_ERR_SUCCESS on success, M_ERR_ARG_INVALID on invalid argument input
 */
int mw_home_zero_populate(struct m_home_zero_t& home_zero, int n_args, va_list args)
{
	for (int i=0; i<n_args; i++) {
		enum m_axes axis = (enum m_axes) va_arg(args, int);
		switch(axis) {
			case m_axes_x:				
				strncpy(home_zero.x, m_axes_str[m_axes_x], M_BUF_LEN);
				break;
			case m_axes_y:
				strncpy(home_zero.y, m_axes_str[m_axes_y], M_BUF_LEN);
				break;
			case m_axes_rot:
				strncpy(home_zero.rot, m_axes_str[m_axes_rot], M_BUF_LEN);
				break;
			case m_axes_z:
				strncpy(home_zero.z, m_axes_str[m_axes_z], M_BUF_LEN);
				break;
			case m_axes_var_atten:
				strncpy(home_zero.var_atten, m_axes_str[m_axes_var_atten], M_BUF_LEN);
				break;
			default:
				goto error;
		}
	}
	return M_ERR_SUCCESS;
	
error:
	va_end(args);
	return M_ERR_ARG_INVALID;
}
/**
 * Fills out a home stages structure
 *
 * @param[in]	home a home stages structure to fill out
 * @param[in]	n_args the number of axes that are being set with this call
 * @param[in]	... each additional argument should be a m_axes to home
 * 
 * @return returns M_ERR_SUCCESS on success, M_ERR_ARG_INVALID on invalid argument input
 */
int mw_home_stage_populate(m_home_t& home, int n_args, ...)
{
	va_list args;
	va_start(args, n_args);
			
	int result = mw_home_zero_populate(home, n_args, args);
	
	va_end(args);
	
	return result;
}
/**
 * Fills out a zero counter structure
 *
 * @param[in]	zer a zero counter structure to fill out
 * @param[in]	n_args the number of axes that are being set with this call
 * @param[in]	... each additional argument should be a m_axes to home
 * 
 * @return returns M_ERR_SUCCESS on success, M_ERR_ARG_INVALID on invalid argument input
 */
int mw_zero_counters_populate(m_zero_t& zero, int n_args, ...)
{
	va_list args;
	va_start(args, n_args);
			
	int result = mw_home_zero_populate(zero, n_args, args);
	
	va_end(args);
	
	return result;
}

/**
 * Writes a command to move the stage position in an arc while using the laser
 *
 * @param[in]	x0 the start x coordinate
 * @param[in]	y the start y coordinate
 * @param[in]	x1 the end x coordinate
 * @param[in]	y1 the end y coordinate
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_arc_exec(struct m_arc_t arc)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
	/* arc with nothing:
	 * ArcMove;1;1;2;False;;299999997;1999999980;0;0;0.0000;0.0000;;;2
	 * radius of 100.0um
	 * ArcMove;1;1;2;False;;299999997;1999999980;0;100000;0.0000;0.0000;;;2
	 * 45 degrees start + radius of 100.0um
	 * ArcMove;1;1;2;False;;299999997;1999999980;0;100000;0.0000;0.0000;;;2
	 * 125 degees end + 45 degrees end + radius of 100.0um
	 * ArcMove;1;1;2;False;;299999997;1999999980;0;100000;45.0000;0.0000;;;2
	 * laser on at 250um intervals + 125 degees end + 45 degrees end + radius of 100.0um
	 * ArcMove;1;1;2;True;;;1999999980;0;100000;45.0000;125.0000;500;250000;2
	 * 
	 * Format is:
	 * ArcMove;1;1;2;[LaserOn];;[299999997:missing];1999999980;0;[radius(nm)];[start angle];[end angle];[missing:500];250000;2
	 */
	 
	fprintf(fp, "ArcMove;%s;%s;%s;%s;;%s;%s;%s;%s;%s;%s;%s;%s;%s"LINE_END, 
		arc.one,
		arc.x_mapping,
		arc.y_mapping,
		arc.laser_on,
		arc.vector_speed,
		arc.vector_accel,		
		arc.settle_delay,
		arc.radius,
		arc.start_angle,
		arc.end_angle,
		arc.repeat_rate,
		arc.spacing,
		arc.wait
	);
	
	return M_ERR_SUCCESS;
}

int mw_line_exec(struct m_line_t line)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
		
	/* a null move instruction:
	 * RelMove;1;;Disabled;Disabled;Disabled;False;Disabled;Disabled;Disabled;Disabled;2
	 * Move x by 0.1um:
	 * RelMove;1;;299999997;1999999980;0;False;100;Disabled;Disabled;Disabled;2
	 * Move y by 0.1um:
	 * RelMove;1;;299999997;1999999980;0;False;Disabled;100;Disabled;Disabled;2
	 * Move in y by 1um and in x by 0.5um
	 * RelMove;1;;299999997;1999999980;0;False;500;1000;Disabled;Disabled;2
	 * a same move but firing at 1um intervals is:
	 * RelMove;1;;500;1999999980;0;False;500;1000;Disabled;100000;2
	 * a same move but firing at 100um intervals is: ???	 *
	 * 
	 * From this it seems a relative move is: 
	 * RelMove;1;;299999997;1999999980;0;False;[x(nm)];[y(nm)];Disabled;[laser spacing(nm)];2
	 */
	
	fprintf(fp, "%s;%s;;%s;%s;%s;%s;%s;%s;%s;%s;%s"LINE_END,
		line.coordinate_type,
		line.one,
		line.vector_speed,
		line.vector_accel,
		line.settle_delay,
		line.false_c,
		line.x, 
		line.y,
		line.rotation,
		line.spacing,
		line.wait
	);
	
	return M_ERR_SUCCESS;		
}

/**
 * Writes an instruction to zero the x or y counter
 *
 * @param[in]	counter the counter to write to ('x' or 'y')
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_zero_counter(char counter)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
	
	if (counter != 1 && counter != 2) {
		return M_ERR_ARG_INVALID;
	}
		
	fprintf(fp, "ZeroCounters;%1d"LINE_END,  (counter == 'x') ? '1' : '2');	
	return M_ERR_SUCCESS;
}



/**
 * Writes an instruction to home axes
 *
 * @param[in]	home a home structure to write out
 * 
 * @return returns M_ERR_SUCCESS on success, M_ERR_FILE_NOT_OPEN on file not open
 */
int mw_home_stage_exec(m_home_t& home)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
		
	fputs("HomeStages", fp);
	
	/* axes start from x=1 */
	for (int i=1; i<m_axes_num_axes; i++) {
		if (strncmp(home.values[i-1], m_axes_str[i], M_AXES_STR_LEN) == 0) {
			fprintf(fp, ";%s", m_axes_str[i]);
		}
	}
	fputs(LINE_END, fp);
	
	return M_ERR_SUCCESS;	
}

/**
 * Writes an instruction to zero counters
 *
 * @param[in]	zero a zero counter structure to write out
 * 
 * @return returns M_ERR_SUCCESS on success, M_ERR_FILE_NOT_OPEN on file not open
 */
int mw_zero_counters_exec(m_zero_t& zero)
{
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
		
	fputs("ZeroCounters", fp);
	
	/* axes start from x=1 */
	for (int i=1; i<m_axes_num_axes; i++) {
		if (strncmp(zero.values[i-1], m_axes_str[i], M_AXES_STR_LEN) == 0) {
			fprintf(fp, ";%s", m_axes_str[i]);
		}
	}
	fputs(LINE_END, fp);
	
	return M_ERR_SUCCESS;	
}

/**
 * Printf style function. Prepends macro comment tag and appends '\n'
 *
 * @param[in]	format the printf style format string
 * @param[in]	... data items as needed for printf format
 * 
 * @return returns 0 on success, -1 on file not open
 */
int mw_comment(const char *format, ...) {
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
		
	va_list args;
    va_start( args, format );
    fprintf(fp, "Comment;");
    vfprintf(fp, format, args );
    fprintf(fp, LINE_END);
    va_end( args );
	
	return M_ERR_SUCCESS;
}

/**
 * Initialises the macro writer
 *
 * @param[in]	file_name the name of the file the macrowriter should write to
 * 
 * @return returns 0 on success, -1 on failure
 */
int mw_init(const char *file_name)
{
	fp = fopen(file_name, "w");
	if (!fp)
		return M_ERR_FILE_NOT_OPEN;
		
	name = (char *)file_name;
	
	fprintf(fp, "[Macro List Data]"LINE_END);	
	
	printf("Macro file %s open\n", name);	
	
	return M_ERR_SUCCESS;
}

/**
 * Closes the macro writer
 * 
 * @return returns 0
 */
int mw_close()
{
	/* not sure what this does, but the program generated it at the end of every file */
	fprintf(fp, "ShowSpawned,True"LINE_END);
	fclose(fp);
	printf("Macro file %s closed\n", name);
	name = (char *)"";
	return M_ERR_SUCCESS;
}
