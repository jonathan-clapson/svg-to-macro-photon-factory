#include <stdio.h>
#include <stdarg.h>

FILE *fp;
char *name;

/**
 * Initialises the macro writer
 *
 * @param[in]	file_name the name of the file the macrowriter should write to
 * 
 * @return returns 0 on success, -1 on failure
 */
int macro_writer_init(const char *file_name)
{
	fp = fopen(file_name, "w");
	if (!fp)
		return -1;
		
	name = (char *)file_name;
	printf("Macro file %s open\n", name);
	return 0;
}

/**
 * Printf style function. Prepends macro comment tag and appends '\n'
 *
 * @param[in]	format the printf style format string
 * @param[in]	... data items as needed for printf format
 * 
 * @return returns 0
 */
int macro_writer_comment(const char *format, ...) {
	va_list args;
    va_start( args, format );
    fprintf(fp, "Comment;");
    vfprintf(fp, format, args );
    fprintf(fp, "\n");
    va_end( args );
	
	return 0;
}

/**
 * Writes a command to move the stage position without using the laser
 *
 * @param[in]	x the x coordinate
 * @param[in]	y the y coordinate
 * 
 * @return returns 0
 */
int macro_writer_move_to(int x, int y)
{
	if (!fp)
		return (-1);
	fprintf(fp, "Line;500;1000000000;0;False;27500000;12500050;Disabled;10000;2\n");
	
	return 0;
}

/**
 * Writes a command to move the stage position linearly while using the laser
 *
 * @param[in]	x0 the start x coordinate
 * @param[in]	y the start y coordinate
 * @param[in]	x1 the end x coordinate
 * @param[in]	y1 the end y coordinate
 * 
 * @return returns 0
 */
int macro_writer_write_line(int x0, int y0, int x1, int y1)
{
	if (!fp)
		return -1;
	
	fprintf(fp, "Line;500;1000000000;0;False;27500000;12500050;Disabled;10000;2\n");
	
	return 0;
}

/**
 * Writes a command to move the stage position in an arc while using the laser
 *
 * @param[in]	x0 the start x coordinate
 * @param[in]	y the start y coordinate
 * @param[in]	x1 the end x coordinate
 * @param[in]	y1 the end y coordinate
 * 
 * @return returns 0
 */
int macro_writer_write_arc()
{
	if (!fp)
		return -1;
		
	fprintf(fp, "arc\n");
	return 0;
}

/**
 * Closes the macro writer
 * 
 * @return returns 0
 */
int macro_writer_close()
{
	fclose(fp);
	printf("Macro file %s closed\n", name);
	name = (char *)"";
	return 0;
}
