#include <cstring>
#include <cstdlib>

#include "macroreader.h"
#include "laser_types.h"
#include "macroerror.h"

FILE *mr_fp;
char *mr_name;

#define LINE_END "\r\n"

#define MR_READ_LEN 256 /* this must always be bigger than the size of the largest command word */
#define CMD_TABLE_NUM_ENTRIES 5


/**
 * Gets a parameter from a ';' seperated parameter string
 *
 * @param[in]	destination c string to copy to parameter to, destination can be NULL in which case this function will not copy data but will still return a line pointer at the end of the string
 * @param[in]	line the ';' seperated parameter string
 * 
 * @return returns the address of the next element in line
 */
char* mr_get_param(char *dest, char *line)
{
	//printf("mr_param line in: %s\n", line);
	fflush(stdout);
	/* FIXME: do something about this function, as it is it can't report errors. ?Does it need to? */
	for (int i=0; *line!='\0'; i++) {
		//printf("strlen: %d, i: %d\n", strlen(line), i);
		if (*line == ';') {
			if (dest != NULL)
				dest[i] = '\0';
			line ++;
			//printf("got ';', breaking");
			break;
		}
		if (dest != NULL)
			dest[i] = *line;
		line ++;
	}
	//printf("mr_param line out: %s\n", line);
	fflush(stdout);
	return line;
}

int mr_read_arc_params(m_arc_t *dest, char *line)
{
	/* read in all arc params */
	line = mr_get_param(dest->one, line);
	line = mr_get_param(dest->x_mapping, line);
	line = mr_get_param(dest->y_mapping, line);
	line = mr_get_param(dest->laser_on, line);
	line = mr_get_param(NULL, line);
	line = mr_get_param(dest->vector_speed, line);
	line = mr_get_param(dest->vector_accel, line);
	line = mr_get_param(dest->settle_delay, line);
	line = mr_get_param(dest->radius, line);
	line = mr_get_param(dest->start_angle, line);
	line = mr_get_param(dest->end_angle, line);
	line = mr_get_param(dest->repeat_rate, line);
	line = mr_get_param(dest->spacing, line);
	line = mr_get_param(dest->wait, line);
	
	return 0;
}
int mr_read_line_params(m_line_t *dest, char *line)
{
	/* read in all line params */
	line = mr_get_param(dest->one, line);
	//printf("dest one: %s\n", dest->one);
	//printf("Processing line: %s\n", line);
	/* clear the empty param */
	line = mr_get_param(NULL, line);
	line = mr_get_param(dest->vector_speed, line);
	//printf("dest speed: %s\n", dest->vector_speed);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->vector_accel, line);
	//printf("dest accel: %s\n", dest->vector_accel);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->settle_delay, line);
	//printf("dest delay: %s\n", dest->settle_delay);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->false_c, line);
	//printf("dest false: %s\n", dest->false_c);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->x, line);
	//printf("dest x: %s\n", dest->x);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->y, line);
	//printf("dest y: %s\n", dest->y);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->rotation, line);
	//printf("dest rot: %s\n", dest->rotation);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->spacing, line);
	//printf("dest space: %s\n", dest->spacing);
	//printf("Processing line: %s\n", line);
	line = mr_get_param(dest->wait, line);
	//printf("dest wait: %s\n", dest->wait);
	//printf("Processing line: %s\n", line);
	
	fflush(stdout);
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
		fprintf(stderr, "MR_ERROR: cannot find command in line %s\n", line);
	}
	
	//move line pointer past command
	line += strlen(m_move_commands_str[command]);
	//move line pointer past ';'
	line ++;
	
	return line;
}

int mr_read(mr_inst_ptr &inst_ptr, enum m_commands_t &command_type) {
	/* well this is what i wanted to do, but i couldn't figure out how to get it to do it
	 * char line[MR_READ_LEN];
	 * so instead we allocate memory then free it...
	 */
	char *line_memory = (char *) malloc(MR_READ_LEN);
	char *line = line_memory; /* get a copy of line_memory that we can muck round with */
	inst_ptr = NULL;
	
	m_arc_t* arc_ptr = NULL;
	m_line_t* line_ptr = NULL;
	
	fgets(line, MR_READ_LEN, mr_fp);
	if (feof(mr_fp)) {
		return M_ERR_EOF;
	}
	command_type = m_command_invalid;
	//printf("Processing line: %s\n", line);
	line = mr_get_command(line, command_type);
	//printf("Got command: %d, Processing line: %s\n", command_type, line);
	//fflush(stdout);
	if (command_type < 0) {
		free(line_memory);
		return (int) command_type;
	}
	//printf("here2\n");
	//fflush(stdout);
	switch(command_type) {
		case m_arc: /* ArcMove */
			fprintf(stderr, "arc\n");
			arc_ptr = (m_arc_t *) malloc(sizeof(m_arc_t));
			mr_read_arc_params(arc_ptr, line);			
			inst_ptr = (mr_inst_ptr) arc_ptr;
			break;		
		case m_origin: /* Origin Move */
			fprintf(stderr, "origin\n");
			line_ptr = (m_line_t *) malloc(sizeof(m_line_t));
			mr_read_line_params(line_ptr, line);	
			inst_ptr = (mr_inst_ptr) line_ptr;	
			break;
		case m_relative: /* Relative Move */
			fprintf(stderr, "rel\n");
			line_ptr = (m_line_t *) malloc(sizeof(m_line_t));
			mr_read_line_params(line_ptr, line);
			//printf("lineax: %s\n",  line_ptr->x);
			inst_ptr = (mr_inst_ptr) line_ptr;	
			//printf("instptr ax: %s\n", ((m_line_t *)inst_ptr)->x);
			break;
		case m_absolute: /* Absolute Move */
			fprintf(stderr, "abs\n");
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
Processing line: 
here
Processing line: ShowSpawned,True

MR_ERROR: cannot find command in line ShowSpawned,True

Got command: -1, Processing line: howSpawned,True


 */
int mr_init(const char *file_name)
{
	mr_fp = fopen(file_name, "r");
	if (!mr_fp)
		return -1;
		
	mr_name = (char *)file_name;
	
	char first_line[MR_READ_LEN];
	
	fgets (first_line, sizeof(first_line), mr_fp );
	if (strcmp(first_line, "[Macro List Data]\r\n") != 0 ) {
		printf("MR_ERROR: File has incorrect format\n");
		mr_close();		
	}
	
	printf("Macro file %s open\n", mr_name);
	
	return 0;
}

/**
 * Closes the macro reader
 * 
 * @return returns 0
 */
int mr_close()
{
	fclose(mr_fp);
	printf("Macro file %s closed\n", mr_name);
	mr_name = (char *)"";
	return 0;
}
