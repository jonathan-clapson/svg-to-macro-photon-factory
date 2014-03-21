#include "laser_types.h"

const char m_move_commands_str [m_num_move_commands][M_BUF_LEN] = {
	"RelMove",
	"OriginMove",
	"AbsMove",
	"ArcMove",
	"Comment"
};

const char m_axes_str [m_axes_num_axes][M_AXES_STR_LEN] = {	
	"", /* null axis */
	"1", /* x axis */
	"2", /* y axis */
	"3", /* rot axis */
	"4", /* z axis */
	"5", /* var attenuator axis */
};
