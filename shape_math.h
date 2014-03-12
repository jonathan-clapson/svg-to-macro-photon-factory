#ifndef __SHAPE_MATH_H__
#define __SHAPE_MATH_H__

#include <cmath>

struct point_t {
	double x;
	double y;
};

void cubic_bezier_get_x_y(
	struct point_t &result, 
	struct point_t start,
	struct point_t inter1,
	struct point_t inter2, 
	struct point_t end,
	double t);

#endif /* __SHAPE_MATH_H__ */
