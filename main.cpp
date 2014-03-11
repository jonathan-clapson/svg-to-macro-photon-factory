#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "macrowriter.h"

/* need to do these
 * Circle <circle>
 * Ellipse <ellipse>
 * Polyline <polyline>
 * Polygon <polygon>
 * Path <path>
 */

/* switch default output name to be default input with mac extension */
char out_file[255] = "output.mac"; /* output file name - default is output-mac */
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
			strncpy(optarg, optarg, sizeof(optarg));
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
 * Gets a integer property from an xml node
 *
 * @param[in]	node the node to retrieve a property from
 * @param[in]	property_name the property name to retrieve
 * 
 * @return returns the propery value. In error case, returns 0
 */
int xml_get_int_param(xmlNodePtr node, xmlChar *property_name)
{
	int value;
	
	xmlChar *value_char = xmlGetProp(node, property_name);
	/* if it fails output an error and clean up */
	if (!value_char) {
		xmlChar *id = xmlGetProp(node, (xmlChar*)"id");
		fprintf(stderr, "WARN: %s has no %s! Setting to 0\n", id, property_name);
		xmlFree(id);
		return 0;		
	}	
	
	/* otherwise output value and clean up */
	value = atoi((char *)value_char);
	
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

int process_line(xmlNodePtr line_node){
	int fill = 0;
	int x1,y1,x2,y2;	
	
	//<line id="svg_3" y2="378" x2="469" y1="133" x1="51" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	/* retrieve and convert data */
	xmlChar *id = xmlGetProp(line_node, (xmlChar*)"id");
	printf("processing line %s\n", id);
	
	x1 = 	xml_get_int_param(line_node, (xmlChar*)"x1");	
	y1 = 	xml_get_int_param(line_node, (xmlChar*)"y1");	
	x2 = 	xml_get_int_param(line_node, (xmlChar*)"x2");	
	y2 = 	xml_get_int_param(line_node, (xmlChar*)"y2");	
	fill = 	xml_get_int_param(line_node, (xmlChar*)"fill");	
	
	macro_writer_write_line(x1, y1, x2, y2);		
	
	xmlFree(id);
		
	return 0;
}

int process_rect(xmlNodePtr rect_node){
	int fill = 0;
	/* svg rectangle representation */
	int width,height,x,y;	
	
	/* used for converting to lines */
	int x1, x2, y1, y2;
	
	//<rect id="svg_5" height="218" width="315" y="121" x="89" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	/* retrieve and convert data */
	xmlChar *id = xmlGetProp(rect_node, (xmlChar*)"id");
	printf("processing line %s\n", id);
	
	/* should probably check these aren't null? */
	width = xml_get_int_param(rect_node, (xmlChar*)"width");
	height = xml_get_int_param(rect_node, (xmlChar*)"height");
	x = xml_get_int_param(rect_node, (xmlChar*)"x");
	y = xml_get_int_param(rect_node, (xmlChar*)"y");
	fill = 	xml_get_int_param(rect_node, (xmlChar*)"fill");	
	
	/* FIXME: reorder these for speed */
	/* line1 goes from x0 to width at fixed y position */
	x1 = x;
	x2 = x + width;
	y1 = y;
	y2 = y;
	macro_writer_write_line(x1, y1, x2, y2);
	fflush(stdout);
	/* line2 goes from x0 to width at fixed y position + offset */
	x1 = x;
	x2 = x + width;
	y1 = y + height;
	y2 = y + height;
	macro_writer_write_line(x1, y1, x2, y2);
	/* line3 goes from y0 to height at fixed x position */
	x1 = x;
	x2 = x;
	y1 = y;
	y2 = y + height;
	macro_writer_write_line(x1, y1, x2, y2);
	/* line4 goes from y0 to height at fixed x position + offset */
	x1 = x + width;
	x2 = x + width;
	y1 = y;
	y2 = y + height;
	macro_writer_write_line(x1, y1, x2, y2);
	
	xmlFree(id);
	return 0;
}

int process_circle(xmlNodePtr circle_node){
	printf("processing circle\n");
	
	 //<circle id="svg_1" r="203.21663" cy="240" cx="300" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="#FFFFFF"/>
	 macro_writer_write_arc();	
	 
	return 0;
}
int process_path(xmlNodePtr path_node){
	//<path d="m47,308c0,-1 0,-8 0,-11c0,-5 0,-9 1,-14c1,-5 1,-10 2,-15c1,-5 1.88134,-10.02521 3,-15c0.90456,-4.02266 2.16796,-8.92033 4,-13c2.20607,-4.91257 4.95915,-9.43547 7,-14c2.08127,-4.65492 3.72399,-9.22273 7,-13c2.77979,-3.20512 5,-6 8,-9c2,-2 4.07844,-3.78986 7,-5c4.13171,-1.71141 7,-4 10,-5c3,-1 5.54916,-2.95517 9,-4c3.82837,-1.15913 8,0 13,0c6,0 11.07179,0.69124 16,2c6.11269,1.62331 11,4 16,6c5,2 9.08743,3.79393 14,6c4.07967,1.83203 9.37201,3.3851 13,6c3.4418,2.48071 7.16119,6.7464 12,10c4.14925,2.78995 8.64117,6.55037 13,9c4.6946,2.63832 8.37201,4.3851 12,7c3.4418,2.48071 6.16119,5.7464 11,9c4.14925,2.78995 7.74283,6.34457 13,10c4.7874,3.3288 9.2126,6.6712 14,10c5.25717,3.65543 9.55801,7.07764 15,11c4.58908,3.30762 8.54123,8.51871 13,12c4.59598,3.58844 10.21262,6.6712 15,10c5.25717,3.65543 8.96539,7.04669 13,10c6.94141,5.08105 10.21167,6.71411 13,9c2.18735,1.79318 7.07843,4.78986 10,6c2.06586,0.85571 7,3 10,3c6,0 10,0 12,0c6,0 9.92767,-0.23172 16,-2c7.91736,-2.30554 12.05707,-3.83392 21,-7c8.05417,-2.85141 11.8176,-5.43863 21,-9c8.84488,-3.43048 13.45938,-3.98267 21,-8c7.5921,-4.04477 10.22769,-6.69565 17,-12c7.25821,-5.68494 10.54947,-8.70004 18,-13c10.46518,-6.03986 21,-9 25,-11c8,-4 14.08743,-6.79395 19,-9c4.07965,-1.83203 6,-3 8,-5l1,-1l2,-1l1,0" id="svg_1" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	printf("processing path\n");
	return 0;
}


int main (int argc, char *argv[])
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodeSetPtr nodeset;
	
	process_cmd_opts(argc, argv);
	
	printf("Using output file %s\n", out_file);
	printf("Using input file %s\n", in_file);
	
	if (macro_writer_init(out_file) != 0)	
		return -1;
	
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
			//xmlChar *keyword = xmlNodeListGetString(doc, nodeset->nodeTab[i]->name, 1);
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
				else if (xmlStrcmp(g_child->name, (xmlChar *)"path") == 0)
					process_path(g_child);
				
				
				g_child = g_child->next;
			}
		}
		xmlXPathFreeObject (layers);
	}	
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
	
	macro_writer_close();
	
	return 0;
}
