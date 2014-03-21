#include "laser_types.h"
#include "macroreader.h"
#include "macroerror.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("One file expected\n");
		return -1;
	}
	
	mr_init(argv[1]);
	
	mr_inst_ptr instruction;
	
	int read_ret = M_ERR_SUCCESS;
	for (int i=0; read_ret != M_ERR_EOF; i++) {
		enum m_commands_t command_type;
		read_ret = mr_read(instruction, command_type);
		if (instruction != NULL) {
			printf("here\n");
			fflush(stdout);
			if (command_type == m_relative) {
				m_line_t *line = (m_line_t *) instruction;
				printf("line x: %s\n", line->x);
			}
			mr_free (instruction);
		}
		else 
			printf("Null inst\n");
	}
	
	mr_close();
}
