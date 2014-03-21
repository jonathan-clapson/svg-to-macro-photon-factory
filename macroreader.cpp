#include <cstring>
#include <cstdlib>

#include "macroreader.h"
#include "laser_types.h"
#include "macroerror.h"

FILE *fp;
char *name;

#define LINE_END "\r\n"

#define MR_READ_LEN 256/* this must always be bigger than the size of the largest command word */
#define CMD_TABLE_NUM_ENTRIES 5

char* mr_get_param(char *dest, char *line)
{
	/* FIXME: do something about this function, as it is it can't report errors. ?Does it need to? */
	for (int i=0; i<strlen(line); i++) {
		if (*line == ';') {
			line ++;
			break;
		}
		dest[i] = *line;
		line ++;
	}
	return line;
}

int mr_read_arc_params(m_arc_t *dest, char *line)
{
	/* read in all arc params */
	line = mr_get_param(dest->one, line);
	
	return 0;
}
int mr_read_line_params(m_line_t *dest, char *line)
{
	/* read in all line params */
	line = mr_get_param(dest->one, line);
	line = mr_get_param(dest->vector_speed, line);
	line = mr_get_param(dest->vector_accel, line);
	line = mr_get_param(dest->settle_delay, line);
	line = mr_get_param(dest->false_c, line);
	line = mr_get_param(dest->x, line);
	line = mr_get_param(dest->y, line);
	line = mr_get_param(dest->rotation, line);
	line = mr_get_param(dest->spacing, line);
	line = mr_get_param(dest->wait, line);	
	
	return 0;
}

char* mr_get_command(char *line, enum m_commands_t &command){
	command = m_command_invalid;	
	for (int i=0; i<CMD_TABLE_NUM_ENTRIES; i++) {
			if (strncmp(line, m_move_commands_str[i], strlen(m_move_commands_str[i]) ) == 0 ) {
				command = (enum m_commands_t) i;
				break;
			}
	}
	
	if (command == -1) {
		printf("MR_ERROR: cannot find command in line %s\n", line);
	}
	
	//move line pointer past command
	line += strlen(m_move_commands_str[command]);
	//move line pointer past ';'
	line ++;
	
	return line;
}

int mr_read(mr_inst_ptr inst_ptr, enum m_commands_t &command_type) {
	/* well this is what i wanted to do, but i couldn't figure out how to get it to do it
	 * char line[MR_READ_LEN];
	 * so instead we allocate memory then free it...
	 */
	char *line_memory = (char *) malloc(MR_READ_LEN);
	char *line = line_memory; /* get a copy of line_memory that we can muck round with */
	inst_ptr = NULL;
	
	m_arc_t * arc_ptr = NULL;
	m_line_t * line_ptr = NULL;
	
	fgets(line, MR_READ_LEN, fp);
	if (feof(fp)) {
		return M_ERR_EOF;
	}
	command_type = m_command_invalid;
	line = mr_get_command(line, command_type);
	if (command_type < 0) {
		free(line_memory);
		return (int) command_type;
	}
	switch(command_type) {
		case m_arc: /* ArcMove */
			printf("arc\n");
			arc_ptr = (m_arc_t *) malloc(sizeof(m_arc_t));
			mr_read_arc_params(arc_ptr, line);
			
			inst_ptr = (mr_inst_ptr) arc_ptr;
			break;
		
		case m_origin: /* Origin Move */
			printf("origin\n");
			line_ptr = (m_line_t *) malloc(sizeof(m_line_t));
			mr_read_line_params(line_ptr, line);	
			inst_ptr = (mr_inst_ptr) line_ptr;	
			break;
		case m_relative: /* Relative Move */
			printf("rel\n");
			line_ptr = (m_line_t *) malloc(sizeof(m_line_t));
			mr_read_line_params(line_ptr, line);
			printf("linex: %s\n", line_ptr->x);
			inst_ptr = (mr_inst_ptr) line_ptr;	
			break;
		case m_absolute: /* Absolute Move */
			printf("abs\n");
			line_ptr = (m_line_t *) malloc(sizeof(m_line_t));
			mr_read_line_params(line_ptr, line);
			inst_ptr = (mr_inst_ptr) line_ptr;				
			break;
		
		case m_comment: /* Comment */
			/* the reset of the line is the comment */
			puts(line);
			
			//line_ptr = (mr_inst_type) line_ptr;
			break;
	}
	
	free(line_memory);
	return M_ERR_SUCCESS;
	
}
void mr_free(mr_inst_ptr ptr) {
	free( (void *) ptr);
}


/**
 * Initialises the macro reader
 *
 * @param[in]	file_name the name of the file the macrowriter should write to
 * 
 * @return returns 0 on success, -1 on failure
 */
int mr_init(const char *file_name)
{
	fp = fopen(file_name, "r");
	if (!fp)
		return -1;
		
	name = (char *)file_name;
	
	char first_line[MR_READ_LEN];
	
	fgets (first_line, sizeof(first_line), fp );
	if (strcmp(first_line, "[Macro List Data]\r\n") != 0 ) {
		printf("MR_ERROR: File has incorrect format\n");
		mr_close();		
	}
	
	printf("Macro file %s open\n", name);
	
	return 0;
}

/**
 * Closes the macro reader
 * 
 * @return returns 0
 */
int mr_close()
{
	fclose(fp);
	printf("Macro file %s closed\n", name);
	name = (char *)"";
	return 0;
}
