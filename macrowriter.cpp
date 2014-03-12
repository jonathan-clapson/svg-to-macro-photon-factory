#include <stdio.h>
#include <stdarg.h>

FILE *fp;
char *name;

int macro_writer_init(const char *file_name)
{
	fp = fopen(file_name, "w");
	if (!fp)
		return -1;
		
	name = (char *)file_name;
	printf("Macro file %s open\n", name);
	return 0;
}

int macro_writer_comment(const char *format, ...) {
	va_list args;
    va_start( args, format );
    fprintf(fp, "Comment;");
    vfprintf(fp, format, args );
    fprintf(fp, "\n");
    va_end( args );
	
	return 0;
}

int macro_writer_move_to(int x, int y)
{
	if (!fp)
		return (-1);
	fprintf(fp, "Line;500;1000000000;0;False;27500000;12500050;Disabled;10000;2\n");
	
	return 0;
}

int macro_writer_write_line(int x0, int y0, int x1, int y1)
{
	if (!fp)
		return -1;
	
	fprintf(fp, "Line;500;1000000000;0;False;27500000;12500050;Disabled;10000;2\n");
	
	return 0;
}

int macro_writer_write_arc()
{
	if (!fp)
		return -1;
		
	fprintf(fp, "arc\n");
	return 0;
}

int macro_writer_close()
{
	fclose(fp);
	printf("Macro file %s closed\n", name);
	name = (char *)"";
	return 0;
}
