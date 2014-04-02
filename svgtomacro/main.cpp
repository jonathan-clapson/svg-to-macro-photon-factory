#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>

#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "macrowriter.h"
#include "macroerror.h"
#include "shape_math.h"
#include <macroconfig.h>

/* need to do these
 * Line <line> -ish
 * Rectangle <rect> -ish
 * Circle <circle> -ish
 * Ellipse <ellipse> -ish
 * Polyline <polyline> -needs to be done, used for connected items
 * Polygon <polygon> -not used?
 * Path <path> -ish
 */
 
using namespace std;

/* switch default output name to be default input with mac extension */
char out_file[255] = ""; /* output file name - default is output-mac */
char in_file[255] = ""; /* input file name */
FILE *out_fp; /* output file pointer */

/**
 * Displays help message on commandline
 */
void help(void)
{
	printf("Welcome to the svg to macro converter\n");
	printf("This program takes the following input format:\n");
	printf("converter [options] input_file\n");
	printf("This converter is designed to convert from svg to ??? macro format\n");
	printf("Options are as follows\n");
	printf("-o = set the output file name\n");
}

/**
 * Processes commandline arguments
 *
 * @param[in]	argc commandline argument count
 * @param[in]	argv commandline arguments
 */
void process_cmd_opts(int argc, char *argv[])
{	
	int index;
	int c;
	while ( (c=getopt(argc, argv, "o:")) != -1) {
		switch (c) {
		case 'o':
			printf("o\n");
			strncpy(out_file, optarg, sizeof(out_file));
			break;
		case '?':
			help();
			break;
		default:
			abort();
		}
	}
	
	if (argc - optind != 1) {
		help();
		exit(EXIT_SUCCESS);
	}
	
	/* handle non-option argument */
	for (index = optind; index < argc; index++) {
		strncpy(in_file, argv[index], sizeof(in_file));
	}
}

/**
 * Gets a node from a namespace using xpath
 *
 * @param[in]	doc the xml document to get nodes from
 * @param[in]	xpath the xpath to match
 * @param[in]	ns the namespace to look in
 * 
 * @return returns a list of nodes which match xpath in xmlXPathObject format
 */
xmlXPathObjectPtr get_node_set (xmlDocPtr doc, xmlChar *xpath, xmlChar *ns){
	
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	/* get a context */
	context = xmlXPathNewContext(doc);
	printf("checking %s\n", xpath);
	
	xmlXPathRegisterNs(context,  BAD_CAST "svg", BAD_CAST ns);
	
	result = xmlXPathEvalExpression(xpath, context);
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
                printf("No result\n");
		return NULL;
	}
	
	return result;
	
}

/**
 * Gets a double property from an xml node
 *
 * @param[in]	node the node to retrieve a property from
 * @param[in]	property_name the property name to retrieve
 * 
 * @return returns the propery value. In error case, returns 0
 */
double xml_get_double_param(xmlNodePtr node, xmlChar *property_name)
{
	double value;
	
	xmlChar *value_char = xmlGetProp(node, property_name);
	/* if it fails output an error and clean up */
	if (!value_char) {
		xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
		fprintf(stderr, "WARN: %s has no %s! Setting to 0\n", id, property_name);
		xmlFree(id);
		return 0;		
	}	
	
	/* otherwise output value and clean up */
	value = atof((char *)value_char);
	
	xmlFree(value_char);
	
	return value;
}

/**
 * Gets a list of layer nodes using xpath
 *
 * @param[in]	doc the xml document to get nodes from
 * 
 * @return returns a list of nodes which match xpath in xmlXPathObject format
 */
xmlXPathObjectPtr get_layers(xmlDocPtr doc){
	/* select all children of root node which have a title */
	//return get_node_set(doc, (xmlChar *)"/svg:svg/svg:g");
	return get_node_set(doc, (xmlChar *)"/svg:svg/svg:g", (xmlChar *)"http://www.w3.org/2000/svg");
}

void shift_coordinate_file_abs_to_macro(double &x, double &y)
{
	/* shift coordinates */
	x -= mw_paper_half_width_um;
	y -= mw_paper_half_height_um;	
		
	/* change from um to nm */
	x *= 1000;
	y *= 1000;	
}

point_t shift_coordinate_file_abs_to_macro(point_t point)
{
	
	/* shift coordinates */
	point.x -= mw_paper_half_width_um;
	point.y -= mw_paper_half_height_um;	
		
	/* change from um to nm */
	point.x *= 1000;
	point.y *= 1000;	
	
	return point;
}

void shift_coordinate_file_rel_to_macro(double &x, double &y)
{		
	/* change from um to nm */
	x *= 1000;
	y *= 1000;	
}

point_t shift_coordinate_file_rel_to_macro(point_t point)
{
	/* change from um to nm */
	point.x *= 1000;
	point.y *= 1000;	
	
	return point;
}


/**
 * Examines an svg line node. Converts data to macro format and writes
 *
 * @param[in]	node the node to retrieve a property from
 * 
 * @return returns 0
 */
int process_line(xmlNodePtr node){
	double fill = 0;
	double x1,y1,x2,y2;	
	
	//<line id="svg_3" y2="378" x2="469" y1="133" x1="51" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	/* retrieve and convert data */
	xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
	mw_comment("LINE: %s", id);
	printf("processing line %s\n", id);
	
	/* line coordinates are absolute */	
	x1 = 	xml_get_double_param(node, (xmlChar*)"x1");	
	y1 = 	xml_get_double_param(node, (xmlChar*)"y1");	
	x2 = 	xml_get_double_param(node, (xmlChar*)"x2");	
	y2 = 	xml_get_double_param(node, (xmlChar*)"y2");	
	fill = 	xml_get_double_param(node, (xmlChar*)"fill");	
	
	mw_svgedit_helper_draw_line(x1, y1, x2, y2);
	
	xmlFree(id);
		
	return 0;
}

/**
 * Examines an svg ellipse node. Converts data to macro format and writes
 *
 * @param[in]	node the node to retrieve a property from
 * 
 * @return returns 0
 */
int process_ellipse(xmlNodePtr node){
	point_t start_point = {0.0, 0.0}; /* keeps track of the point to close to - this should probably be a list I think you can do subpaths within subpaths? :S */
	point_t current_point = {0.0, 0.0};
	point_t conv_point = {0.0, 0.0};
	
	double fill = 0;
	struct m_line_t line;
	point_t center, radii;
	
	
	xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
	mw_comment("ELLIPSE: %s", id);
	printf("processing ellipse %s\n", id);
	
	center.x = 	xml_get_double_param(node, (xmlChar*)"cx");	
	center.y = 	xml_get_double_param(node, (xmlChar*)"cy");	
	radii.x = 	xml_get_double_param(node, (xmlChar*)"rx");	
	radii.y = 	xml_get_double_param(node, (xmlChar*)"ry");	
	fill = 	xml_get_double_param(node, (xmlChar*)"fill");
	
	/* get start point */
	ellipse_get_x_y(current_point, center, radii, 0.00);
	current_point = shift_coordinate_file_abs_to_macro(current_point);
	
	mw_line_populate(m_absolute, line, current_point.x, current_point.y, M_LASER_OFF);
	mw_line_exec(line);
	
	/* fit line segments to ellipse */				
	double step_size = 0.1;
	for (double t = 0.00+step_size; t <= 2*3.14159; t+=step_size) {		
		/* get a new point along ellipse */
		ellipse_get_x_y(current_point, center, radii, t);
		
		conv_point = shift_coordinate_file_abs_to_macro(current_point);		
		
		mw_line_populate(m_absolute, line, conv_point.x, conv_point.y, M_LASER_ON);
		mw_line_exec(line);
	}		
	
	xmlFree(id);
	return 0;
}
	
/**
 * Examines an svg rectangle node. Converts data to macro format and writes
 *
 * @param[in]	node the node to retrieve a property from
 * 
 * @return returns 0
 */	
int process_rect(xmlNodePtr node){
	double fill = 0;
	/* svg rectangle representation */
	double width,height,x,y;	
	
	/* used for converting to lines */
	double x1, x2, y1, y2;
	
	//<rect id="svg_5" height="218" width="315" y="121" x="89" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	/* retrieve and convert data */
	xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
	mw_comment("RECT: %s", id);
	printf("processing rect %s\n", id);
	
	width = xml_get_double_param(node, (xmlChar*)"width");
	height = xml_get_double_param(node, (xmlChar*)"height");
	x = xml_get_double_param(node, (xmlChar*)"x");
	y = xml_get_double_param(node, (xmlChar*)"y");
	fill = 	xml_get_double_param(node, (xmlChar*)"fill");	
	
	/* FIXME: reorder these for speed */
	/* line1 goes from x0 to width at fixed y position */
	x1 = x;
	x2 = x + width;
	y1 = y;
	y2 = y;			
	mw_svgedit_helper_draw_line(x1, y1, x2, y2);
	/* line2 goes from x0 to width at fixed y position + offset */
	x1 = x;
	x2 = x + width;
	y1 = y + height;
	y2 = y + height;
	mw_svgedit_helper_draw_line(x1, y1, x2, y2);
	/* line3 goes from y0 to height at fixed x position */
	x1 = x;
	x2 = x;
	y1 = y;
	y2 = y + height;
	mw_svgedit_helper_draw_line(x1, y1, x2, y2);
	/* line4 goes from y0 to height at fixed x position + offset */
	x1 = x + width;
	x2 = x + width;
	y1 = y;
	y2 = y + height;
	mw_svgedit_helper_draw_line(x1, y1, x2, y2);
	
	xmlFree(id);
	return 0;
}

/**
 * Examines an svg circle node. Converts data to macro format and writes
 *
 * @param[in]	node the node to retrieve a property from
 * 
 * @return returns 0
 */
int process_circle(xmlNodePtr node){
	double radius;
	double y;
	double x;
	double fill;
	
	xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
	mw_comment("CIRCLE: %s", id);
	printf("processing circle %s\n", id);	
	
	//<circle id="svg_1" r="203.21663" cy="240" cx="300" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="#FFFFFF"/>
		 
	radius = xml_get_double_param(node, (xmlChar*)"r");
	x = xml_get_double_param(node, (xmlChar*)"cx");
	y = xml_get_double_param(node, (xmlChar*)"cy");
	fill = 	xml_get_double_param(node, (xmlChar*)"fill");	
	
	mw_svgedit_helper_draw_circle(radius, (long)x, (long)y);	
	
	xmlFree(id);
	 
	return 0;
}

/**
 * Gets a double beginning at start of path
 *
 * @param[in]	path a string containing doubles seperated by space or comma
 * @param[in]	d_val the double to store the result in
 * 
 * @return returns how many characters were read from the path string to read the double
 */
int path_get_double(char *path, double &d_val)
{
	int i = 0 ;
	string value("");
	
	/* skip to command */
	while(*path == ' ' || *path == ',');
	
	for (i=0; path[i] == '.' || path[i] == '-' || (path[i] >= '0' && path[i] <= '9'); i++) {
			value += path[i];
	}
	d_val = atof(value.c_str());
	
	/* skip spacers */
	if (path[i] == ' ' || path[i] == ',') {
		i++;
	}
	return i;
}

/**
 * Examines an svg path node. Converts data to macro format and writes
 *
 * @param[in]	node the node to retrieve a property from
 * 
 * @return returns 0
 */
int process_path(xmlNodePtr node){
	/* position tracking */
	point_t start_point = {0.0, 0.0}; /* keeps track of the point to close to - this should probably be a list I think you can do subpaths within subpaths? :S */
	point_t current_point = {0.0, 0.0}, last_point = {0.0, 0.0};
	point_t conv_point = {0.0, 0.0}; /* used to convert to macrowriter coordinates before pushing out */
	/* bezier variables */
	point_t bez_start, bez_inter1, bez_inter2, bez_end;
	
	/* first command needs to be absolute, set it by default in case something strange happens */
	enum m_commands_t move_type = m_absolute;
	
	/* used to store line start and end points */
	double x1_line, y1_line, x2_line, y2_line; 
	
	//<path d="m47,308c0,-1 0,-8 0,-11c0,-5 0,-9 1,-14c1,-5 1,-10 2,-15c1,-5 1.88134,-10.02521 3,-15c0.90456,-4.02266 2.16796,-8.92033 4,-13c2.20607,-4.91257 4.95915,-9.43547 7,-14c2.08127,-4.65492 3.72399,-9.22273 7,-13c2.77979,-3.20512 5,-6 8,-9c2,-2 4.07844,-3.78986 7,-5c4.13171,-1.71141 7,-4 10,-5c3,-1 5.54916,-2.95517 9,-4c3.82837,-1.15913 8,0 13,0c6,0 11.07179,0.69124 16,2c6.11269,1.62331 11,4 16,6c5,2 9.08743,3.79393 14,6c4.07967,1.83203 9.37201,3.3851 13,6c3.4418,2.48071 7.16119,6.7464 12,10c4.14925,2.78995 8.64117,6.55037 13,9c4.6946,2.63832 8.37201,4.3851 12,7c3.4418,2.48071 6.16119,5.7464 11,9c4.14925,2.78995 7.74283,6.34457 13,10c4.7874,3.3288 9.2126,6.6712 14,10c5.25717,3.65543 9.55801,7.07764 15,11c4.58908,3.30762 8.54123,8.51871 13,12c4.59598,3.58844 10.21262,6.6712 15,10c5.25717,3.65543 8.96539,7.04669 13,10c6.94141,5.08105 10.21167,6.71411 13,9c2.18735,1.79318 7.07843,4.78986 10,6c2.06586,0.85571 7,3 10,3c6,0 10,0 12,0c6,0 9.92767,-0.23172 16,-2c7.91736,-2.30554 12.05707,-3.83392 21,-7c8.05417,-2.85141 11.8176,-5.43863 21,-9c8.84488,-3.43048 13.45938,-3.98267 21,-8c7.5921,-4.04477 10.22769,-6.69565 17,-12c7.25821,-5.68494 10.54947,-8.70004 18,-13c10.46518,-6.03986 21,-9 25,-11c8,-4 14.08743,-6.79395 19,-9c4.07965,-1.83203 6,-3 8,-5l1,-1l2,-1l1,0" id="svg_1" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
	char *path_string = (char *) xmlGetProp(node, (xmlChar*)"d");
	/* keep track of first char. If moveto is the first char (probably should always be), then it's an absolute irrespective of case */
	char *first_char = path_string;
	int err;
	
	mw_comment("PATH: %s", id);
	printf("processing path %s\n", id);
	enum m_commands_t command = m_command_invalid;
	
	struct m_line_t line;
		
	/* process the path string */
	while(*path_string) {
		switch(*path_string) {
		case 'M': 
		case 'm': /* moveto */
			if ( (*path_string == 'M') || (path_string == first_char) ) {
				command = m_absolute;
				printf("forcing absolute move\n");
			} else {
				command = m_relative;
			}
			
			printf("moveto command: %s\n", m_move_commands_str[command]);
				
			path_string++;
			/* copy current point to last point */
			memcpy(&last_point, &current_point, sizeof(last_point));
			printf("current abs point: %f %f\n", last_point.x, last_point.y);
			
			/* update current point */			
			path_string += path_get_double(path_string, current_point.x);
			path_string += path_get_double(path_string, current_point.y);
			
			if (command == m_relative) {
				printf("relative move by: %f %f\n", current_point.x, current_point.y); 
			} else {
				printf("abs move to: %f %f\n", current_point.x, current_point.y);
			}
			
				
			
			/* if its a relative move we need to shift current_point to the correct absolute coordinate */			
			if (command == m_relative) {
				current_point.x += last_point.x;
				current_point.y += last_point.y;
			}
			memcpy(&start_point, &current_point, sizeof(start_point));
			printf("current point: %f %f\n", current_point.x, current_point.y);
			printf("start point: %f %f\n", start_point.x, start_point.y);
			
			/*if (command == m_absolute) {
				conv_point = shift_coordinate_file_abs_to_macro(current_point);	
			} else {
				conv_point = shift_coordinate_file_rel_to_macro(current_point);
				printf("move offset: %f %f\n", current_point.x, current_point.y);
				printf("move offset mac: %f %f\n", conv_point.x, conv_point.y);
			}*/

			conv_point = shift_coordinate_file_abs_to_macro(current_point);	
			
			mw_line_populate(m_absolute, line, conv_point.x, conv_point.y, M_LASER_OFF);
			err = mw_line_exec(line);
			if (err) {
				fprintf(stderr, "err on element %c\n", (command == m_relative)?'m':'M');
				exit(0);
			}
			
			break;
		
		case 'L': 
		case 'l': /* draw line to */
			if (*path_string == 'L')
				command = m_absolute;
			else
				command = m_relative;
			
			path_string++;
			/* copy current point to last point */
			memcpy(&last_point, &current_point, sizeof(last_point));					
			
			/* update current point */
			path_string += path_get_double(path_string, current_point.x);
			path_string += path_get_double(path_string, current_point.y);
			printf("lineto: %f %f\n", current_point.x, current_point.y);
			
			if (command == m_absolute) {
				conv_point = shift_coordinate_file_abs_to_macro(current_point);	
			} else {
				conv_point = shift_coordinate_file_rel_to_macro(current_point);
			}
				
			printf("shift lineto: %f %f\n", conv_point.x, conv_point.y);						
			mw_line_populate(command, line, conv_point.x, conv_point.y, M_LASER_ON);			
			err = mw_line_exec(line);
			if (err) {
				fprintf(stderr, "err on element %c\n", (command == m_relative)?'l':'L');
				exit(0);
			}			
			break;
			
		case 'C':	
		case 'c': /* curve to. This is a cubic bezier curve */
			if (*path_string == 'C')
				command = m_absolute;
			else
				command = m_relative;
			path_string++;
			/* copy current point to last point */
			memcpy(&last_point, &current_point, sizeof(last_point));
			memcpy(&bez_start, &current_point, sizeof(bez_start));
			
			/* update bezier points */			
			path_string += path_get_double(path_string, bez_inter1.x);
			path_string += path_get_double(path_string, bez_inter1.y);
			path_string += path_get_double(path_string, bez_inter2.x);
			path_string += path_get_double(path_string, bez_inter2.y);
			path_string += path_get_double(path_string, bez_end.x);
			path_string += path_get_double(path_string, bez_end.y);
			
			printf("bezier begins at %f %f\n", bez_start.x, bez_start.y);
			/* if this is a relative bezier all given coordinates are relatives to current point. Shift to absolute */
			
			if (command == m_absolute) {
				fprintf(stderr, "svg-edit never output an absolute bezier for me. This is very likely broken");
			}
			
			if (command == m_relative) {
				bez_inter1.x += bez_start.x;
				bez_inter1.y += bez_start.y;
				bez_inter2.x += bez_start.x;
				bez_inter2.y += bez_start.y;
				bez_end.x += bez_start.x;
				bez_end.y += bez_start.y;
			}
			
			printf("bezier ends at %f %f\n", bez_end.x, bez_end.y);
			printf("iterating bezier. x:%f y:%f z:%f w:%f\n", bez_inter1.x, bez_inter1.y, bez_inter2.x, bez_inter2.y);
			
			/* fit line segments to bezier */				
			for (double t = 0.00; t <= 1.00; t+=0.1) {
				/* get a new point along bezier curve */				
				cubic_bezier_get_x_y(current_point, bez_start, bez_inter1, bez_inter2, bez_end, t);
				
				/*if (command == m_relative) {
					current_point.x+=last_point.x;
					current_point.y+=last_point.y;
				}*/
				printf("pts: %f %f\n", current_point.x, current_point.y);
				conv_point = shift_coordinate_file_abs_to_macro(current_point);				
				printf("ptsmac: %f %f\n", conv_point.x, conv_point.y);
				mw_line_populate(m_absolute, line, conv_point.x, conv_point.y, M_LASER_ON);
				err = mw_line_exec(line);
				if (err) {
					fprintf(stderr, "err on element %c\n", (command == m_relative)?'c':'C');
					exit(0);
				}
			}
			/* update position to end of bezier curve */
			memcpy(&last_point, &current_point, sizeof(last_point));	
			break;
			
		case 'z': case 'Z': /* draw line back to start point */
			path_string++;
			/* update last point */
			memcpy(&last_point, &current_point, sizeof(last_point));
			/* copy closing point to current point */
			memcpy(&current_point, &start_point, sizeof(last_point));
			printf("closing line s:%f %f, e:%f %f\n", last_point.x, last_point.y, start_point.x, start_point.y);
			
			conv_point = shift_coordinate_file_abs_to_macro(current_point);				
			mw_line_populate(m_absolute, line, conv_point.x, conv_point.y, M_LASER_ON);			
			err = mw_line_exec(line);
			if (err) {
				fprintf(stderr, "err on element %c\n", (command == m_relative)?'z':'Z');
				exit(0);
			}
			break;
		default: /* unexpected or unaccounted for */
			/* currently havent deal with Capitol (absolute) versions */
			/* also haven't dealt with q, s, a, h, v, t */			
			fprintf(stderr, "don't know how to deal with command %c\n", *path_string);
			return -1;
		}
	}	
	
	xmlFree(id);
	return 0;
}

int main (int argc, char *argv[])
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodeSetPtr nodeset;
	
	process_cmd_opts(argc, argv);
	
	/* if outfile hasn't been given a name, set it to infile.mac */
	if (*out_file == '\0')
		sprintf(out_file, "%s.mac", in_file);
		
	printf("Using output file %s\n", out_file);
	printf("Using input file %s\n", in_file);
	
	if (mw_init(out_file) != M_ERR_SUCCESS)	
		return -1;
	
	mw_comment("Setup counters for absolute/origin movement");	
	/* 
	 * home x, y and rot axes. Haven't done z as then need to implement autofocus. 
	 * Would be cool to do but thats a bit of work 
	 */
	m_home_t home;
	mw_home_stage_init(home);
	mw_home_stage_populate(home, 3, m_axes_x, m_axes_y, m_axes_rot);
	mw_home_stage_exec(home);
	
	/*
	 * shift to camera offset
	 */
	m_line_t line;
	mw_line_init(line);
	mw_line_populate(m_relative, line, -31718800, -1414600, M_LASER_OFF);
	mw_line_exec(line);
	
	/*
	 * shift from camera offset to beam offset
	 */
	mw_line_init(line);
	mw_line_populate(m_relative, line, 61857000, 96590, M_LASER_OFF);
	mw_line_exec(line);
		
	/*
	 * set abs and origin coordinates ?what is the difference between abs and origin? :S 
	 */
	m_zero_t zero;
	mw_zero_counters_init(zero);
	mw_zero_counters_populate(zero, 3, m_axes_x, m_axes_y, m_axes_rot);
	mw_zero_counters_exec(zero);
	
	doc = xmlParseFile(in_file);
	if (doc == NULL) {
		fprintf(stderr, "%s not parsed successfully.\n", in_file);
		return EXIT_FAILURE;
	}
		
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr, "empty file\n");
		xmlFreeDoc(doc);
		return EXIT_FAILURE;
	}
	printf("xml name: %s\n", cur->name);
	
	/* check we've retrieved an svg file */
	if (xmlStrcmp(cur->name, (const xmlChar *)"svg") != 0) {
		fprintf(stderr, "%s is not an svg file\n", in_file);
		return -1;
	}
	
	/* get drawing area width */
	xmlChar *width = xmlGetProp(cur, (xmlChar*)"width");
	printf("width: %s\n", width);
	xmlFree(width);
	
	/* get drawing area height */
	xmlChar *height = xmlGetProp(cur, (xmlChar*)"height");
	printf("height: %s\n", height);
	xmlFree(height);
	
	xmlXPathObjectPtr layers = get_layers(doc);
	fflush(stdout);
	if (layers) {
		nodeset = layers->nodesetval;
		for (int i=0; i < nodeset->nodeNr; i++) {
			printf("g%d\n", i);
			xmlNodePtr g_node = nodeset->nodeTab[i];
			xmlNodePtr g_child = g_node->xmlChildrenNode;
			
			/* iterate through each child of g */
			while (g_child) {				
				printf("child_name: %s\n", g_child->name);
				if (xmlStrcmp(g_child->name, (xmlChar *)"line") == 0)
					process_line(g_child);
				else if (xmlStrcmp(g_child->name, (xmlChar *)"rect") == 0)
					process_rect(g_child);
				else if (xmlStrcmp(g_child->name, (xmlChar *)"circle") == 0)
					process_circle(g_child);
				else if (xmlStrcmp(g_child->name, (xmlChar *)"ellipse") == 0)
					process_ellipse(g_child);
				else if (xmlStrcmp(g_child->name, (xmlChar *)"path") == 0)
					process_path(g_child);
				else if (xmlStrcmp(g_child->name, (xmlChar *)"text") == 0) /* not actually sure what all of these are */
					NULL;
				else
					fprintf(stderr, "Unknown element %s\n", g_child->name);
				
				g_child = g_child->next;
			}
		}
		xmlXPathFreeObject (layers);
	}	
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	mw_close();
	
	return 0;
}
