#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

char out_file[255] = "output.mac";
char in_file[255] = "";

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

xmlXPathObjectPtr get_node_set (xmlDocPtr doc, xmlChar *xpath){
	
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	xmlChar *nsList = (xmlChar *)"xmlns=\"http://www.w3.org/2000/svg\"";

	/* get a context */
	context = xmlXPathNewContext(doc);
	printf("checking %s\n", xpath);
	
	xmlXPathRegisterNs(context,  BAD_CAST "svg", BAD_CAST "http://www.w3.org/2000/svg");
	
	result = xmlXPathEvalExpression(xpath, context);
	if(xmlXPathNodeSetIsEmpty(result->nodesetval)){
		xmlXPathFreeObject(result);
                printf("No result\n");
		return NULL;
	}
	
	return result;
	
}

xmlXPathObjectPtr get_layers(xmlDocPtr doc){
	/* select all children of root node which have a title */
	//return get_node_set(doc, (xmlChar *)"/svg:svg/svg:g");
	return get_node_set(doc, (xmlChar *)"/svg:svg/svg:g");
}

int process_line(xmlNodePtr line_node){
	int fill = 0;
	int x1,y1,x2,y2;	
	
	//<line id="svg_3" y2="378" x2="469" y1="133" x1="51" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="none"/>
	
	/* The following are probably unneeded. Remember to free these if they are needed */
	/*xmlChar *stroke-opacity = xmlGetProp(line_node, (xmlChar*)"stroke-opacity");
	xmlChar *stroke-linecap = xmlGetProp(line_node, (xmlChar*)"stroke-linecap");
	xmlChar *stroke-linejoin = xmlGetProp(line_node, (xmlChar*)"stroke-linejoin");
	xmlChar *stroke-dasharray = xmlGetProp(line_node, (xmlChar*)"stroke-dasharray");
	xmlChar *stroke-width = xmlGetProp(line_node, (xmlChar*)"stroke-width");
	xmlChar *stroke = xmlGetProp(line_node, (xmlChar*)"stroke");*/
	
	/* retrieve and convert data */
	xmlChar *id = xmlGetProp(line_node, (xmlChar*)"id");
	printf("processing line %s\n", id);
	
	xmlChar *x1_char = xmlGetProp(line_node, (xmlChar*)"x1");
	x1 = atoi((char *)x1_char);
	xmlFree(x1_char);
	xmlChar *y1_char = xmlGetProp(line_node, (xmlChar*)"y1");	
	y1 = atoi((char *)y1_char);
	xmlFree(y1_char);
	xmlChar *x2_char = xmlGetProp(line_node, (xmlChar*)"x2");
	x2 = atoi((char *)x2_char);
	xmlFree(x2_char);
	xmlChar *y2_char = xmlGetProp(line_node, (xmlChar*)"y2");
	y2 = atoi((char *)y2_char);
	xmlFree(y2_char);		
	
	xmlChar *fill_char = xmlGetProp(line_node, (xmlChar*)"fill");
	if ( xmlStrcmp(fill_char, (xmlChar*) "none") != 0 )
		fill = 1;
	else
		fill = 0;
		
	xmlFree(fill_char);
	
	FILE *fp = fopen(out_file, "w");
	fprintf(fp, "Line;500;1000000000;0;False;27500000;12500050;Disabled;10000;2");
	fclose(fp);
	
	/*xmlFree(stroke-linecap);
	xmlFree(stroke-linejoin);
	xmlFree(stroke-dasharray);
	xmlFree(stroke-width);
	xmlFree(stroke);*/
	xmlFree(id);
		
	return 0;
}
int process_circle(xmlNodePtr circle_node){
	printf("processing circle\n");
	
	 //<circle id="svg_1" r="203.21663" cy="240" cx="300" stroke-opacity="null" stroke-linecap="null" stroke-linejoin="null" stroke-dasharray="null" stroke-width="5" stroke="#000000" fill="#FFFFFF"/>
	 
	return 0;
}
int process_path(xmlNodePtr path_node){
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
		fflush(stdout);
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
	
	return 0;
}
