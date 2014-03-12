#ifndef __SHAPE_MATH_H__
#define __SHAPE_MATH_H__

#include <cmath>

struct point_t {
	double x;
	double y;
};

typedef struct point_t point_t;
typedef struct point_t length_t;

void cubic_bezier_get_x_y(
	struct point_t &result, 
	struct point_t start,
	struct point_t inter1,
	struct point_t inter2, 
	struct point_t end,
	double t);
	
void ellipse_get_x_y(
	point_t &result,
	point_t center,
	length_t radii,
	double t);

#endif /* __SHAPE_MATH_H__ */
