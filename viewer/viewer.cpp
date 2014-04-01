#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

#include "laser_types.h"
#include "macroreader.h"
#include "macroerror.h"

#include "shape_math.h"

#define LIST_SIZE 500

/* screen ratio currently needs to be 1:1 as otherwise circular arc turns into elliptical arc and we have to do maths xP */
const double screen_width = 1024;
const double screen_height = 1024;

const double paper_width = 200000000;
const double paper_height = 150000000;

const double paper_ratio = paper_height/paper_width;

const double x_shift = screen_width/2;
const double y_shift = screen_height/2;

/*
 * Scaling
 *  -6 = 1000 = um
 * const double nm_to_mm = 1000;
 */
const double nm_to_minus4 = 200000;
/* -3 = 1000000 = mm 
 *const double nm_to_mm = 1000000;
 */
const double scale_factor = nm_to_minus4;

const double draw_width = paper_width/scale_factor;
const double draw_height = paper_height/scale_factor;

const double left_border = (screen_width/2) - (draw_width/2);
const double right_border = screen_width - left_border;
const double top_border = (screen_height/2) - (draw_height/2);
const double bottom_border = screen_height - top_border;

/* show the undrawable area */
const double blank_top = (1.0-paper_ratio)/2*screen_height;
const double blank_bottom = screen_height - blank_top;

/* these are the origins set by the laser after homing */
double x_origin = +31718800;
double y_origin = -1414600;

void get_screen_coords(double &screen_x, double &screen_y, double laser_x, double laser_y)
{
	screen_x = laser_x / scale_factor;
	screen_x += x_shift;
	screen_y = laser_y / scale_factor;
	screen_y += y_shift;
}
void get_screen_offset(double &screen_x, double &screen_y, double laser_x, double laser_y)
{
	screen_x = laser_x / scale_factor;
	screen_y = laser_y / scale_factor;
}
void scale_to_screen(double &screen_radius, double laser_radius)
{
	screen_radius = laser_radius/scale_factor;
}

void get_circle_point(double &xret, double &yret, double x_center, double y_center, double radius, double t_angle){
	/* circle is parametrically defined as x=r*cos(t)+xcenter, y=r*sin(t)+ycenter 0<t<=2*pi */
	xret = radius * cos(t_angle) + x_center;
	yret = radius * sin(t_angle) + y_center;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("One file expected\n");
		return -1;
	}
	
	ALLEGRO_DISPLAY * display = NULL;
	
	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro\n");
		return -1;
	}
	if (!al_init_primitives_addon()) {
		fprintf(stderr, "failed to initilize primitivies addon\n");
		return -1;
	}
		
	display = al_create_display(screen_width, screen_height);
	if (!display) {
		fprintf(stderr, "failed to create display");
		return -1;
	}
	/* clear the drawing area */
	al_clear_to_color(al_map_rgb(255,255,255));
	/* block out the area that isn't paper */
	al_draw_filled_rectangle(0,0,screen_width,top_border, al_map_rgb(0,0,0));
	al_draw_filled_rectangle(0,screen_height,screen_height,bottom_border, al_map_rgb(0,0,0));
	al_draw_filled_rectangle(0,0,left_border, screen_height, al_map_rgb(0,0,0));
	al_draw_filled_rectangle(right_border, 0, screen_width, screen_height, al_map_rgb(0,0,0));
	
	//gnuplot_ctrl *gnu_plot = gnuplot_init();	
	mr_init(argv[1]);
	
	mr_inst_ptr instruction;
	/* line coordinates */
	double laser_x_new, laser_y_new, laser_x_old, laser_y_old;
	double screen_x_new, screen_y_new, screen_x_old, screen_y_old;
	/* arc coordinates */
	double start_theta, end_theta;
	double laser_x_center, laser_y_center, laser_radius;
	double laser_x_rad_offs, laser_y_rad_offs;
	
	double screen_x_center, screen_y_center, screen_radius;
	
	int read_ret = M_ERR_SUCCESS;
	/* get commands one by one from file */
	for (int i=0; read_ret != M_ERR_EOF; i++) {
		enum m_commands_t command_type;
		read_ret = mr_read(instruction, command_type);
		if (instruction != NULL) {
			printf("command_type: %s\n", m_move_commands_str[command_type]);
			if (command_type == m_relative) {
				m_line_t *line = (m_line_t *) instruction;
				laser_x_new = atof(line->x);
				laser_y_new = atof(line->y);
				get_screen_offset(screen_x_new, screen_y_new, laser_x_new, laser_y_new);
				
				if (atof(line->spacing) != 0.0) {
					al_draw_line(screen_x_old, screen_y_old, screen_x_old+screen_x_new, screen_y_old+screen_y_new, al_map_rgb(0,0,0), 2);
					printf("drawing from (%f,%f) to (%f,%f)\n", laser_x_old, laser_y_old, laser_x_new+laser_x_old, laser_y_new+laser_y_old);
					printf("drawing from (%f,%f) to (%f,%f)\n", screen_x_old, screen_y_old, screen_x_old+screen_x_new, screen_y_old+screen_y_new);
					
				}
					
				/* copy across current position */
				screen_x_old = screen_x_new;
				screen_y_old = screen_y_new;
				laser_x_old = laser_x_new;
				laser_y_old = laser_y_new;
				mr_free(instruction);
			}
			if (command_type == m_absolute) {
				m_line_t *line = (m_line_t *) instruction;
				laser_x_new = atof(line->x);
				laser_y_new = atof(line->y);
				get_screen_coords(screen_x_new, screen_y_new, laser_x_new, laser_y_new);
				//printf("drawing from (%f,%f) to (%f,%f)\n", screen_x_old, screen_y_old, screen_x_new, screen_y_new);
				if (atof(line->spacing) != 0.0)
					al_draw_line(screen_x_old, screen_y_old, screen_x_new, screen_y_new, al_map_rgb(0,0,0), 2);
					
				/* copy across current position */
				screen_x_old = screen_x_new;
				screen_y_old = screen_y_new;
				laser_x_old = laser_x_new;
				laser_y_old = laser_y_new;
				mr_free(instruction);
			}
			if (command_type == m_origin) {
				m_line_t *line = (m_line_t *) instruction;
				laser_x_new = atof(line->x);
				laser_y_new = atof(line->y);
				get_screen_coords(screen_x_new, screen_y_new, laser_x_new, laser_y_new);
				//printf("drawing from (%f,%f) to (%f,%f)\n", screen_x_old, screen_y_old, screen_x_new, screen_y_new);
				if (atof(line->spacing) != 0.0)
					al_draw_line(screen_x_old, screen_y_old, screen_x_new, screen_y_new, al_map_rgb(0,0,0), 2);
				
				/* copy across current position */
				screen_x_old = screen_x_new;
				screen_y_old = screen_y_new;
				laser_x_old = laser_x_new;
				laser_y_old = laser_y_new;
				mr_free(instruction);
			}
			if (command_type == m_arc) {
				m_arc_t *arc = (m_arc_t *) instruction;	

				start_theta = atof(arc->start_angle);
				end_theta = atof(arc->end_angle);		
				
				laser_radius = atof(arc->radius);				
				
				/* get arc center point */	
				/* x^2 + y^2 = r^2
				 * r is known x and y are unknown
				 */
				//printf("edge at %f,%f\n", laser_x_old, laser_y_old);
				//printf("radx: %f, rady%f\n", laser_radius*cos(start_theta), laser_radius*sin(start_theta));
				
				laser_x_rad_offs = laser_radius * cos(start_theta);
				laser_y_rad_offs = laser_radius * sin(start_theta);				
				
				//printf("center shift by %f,%f\n", laser_x_rad_offs, laser_y_rad_offs);
				
				laser_x_center = laser_x_old - laser_x_rad_offs;
				laser_y_center = laser_y_old - laser_y_rad_offs;
				
				laser_x_new = laser_radius * cos(end_theta) + laser_x_center;
				laser_y_new = laser_radius * sin(end_theta) + laser_y_center;
				
				get_screen_coords(screen_x_center, screen_y_center, laser_x_center, laser_y_center);
				scale_to_screen(screen_radius, laser_radius);
				//printf("lx:%f, ly:%f, lrad:%f\n", laser_x_center, laser_y_center, laser_radius);
				//printf("x:%f, y:%f, radius:%f\n", screen_x_center, screen_y_center, screen_radius);
				if ( strcmp(arc->laser_on, "True") == 0) {
					for (double i = start_theta*2*3.14159/180; i<end_theta*2*3.14159/180; i+=0.1) {
						get_circle_point(screen_y_new, screen_x_new, screen_x_center, screen_y_center, screen_radius, i);
					}
					
					al_draw_arc(screen_x_center, screen_y_center, screen_radius, start_theta*2*3.14159/180, end_theta*2*3.14159/180, al_map_rgb(0,0,0), 2);
					
				}
					
				/* copy across current position */
				get_circle_point(screen_y_new, screen_x_new, screen_x_center, screen_y_center, screen_radius, end_theta*2*3.14159/180);
				laser_x_old = laser_x_new;
				laser_y_old = laser_y_new;
					
				mr_free(instruction);
			}
				
		}
		else 
			printf("Null inst\n");
	}
	
	mr_close();
	
	al_flip_display();
	al_rest(10.0);
	
	return 0;

}
