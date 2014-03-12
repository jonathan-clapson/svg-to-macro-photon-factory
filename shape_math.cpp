#include <cmath>

#include "shape_math.h"

void cubic_bezier_get_x_y(
	point_t &result, 
	point_t start,
	point_t inter1,
	point_t inter2, 
	point_t end,
	double t)
{
	/* 
	 * cubic equation
	 * a*(1-t)^3 + 3b*(1-t)^2*t + 3c*(1-t)*t^2 + d*t^3, evaluated between 0 and 1
	 */
	 
	 double b1, b2, b3, b4;
	 
	 b1 = pow( (1-t), 3) ;
	 b2 = 3 * pow( (1-t), 2) * t;
	 b3 = 3 * (1-t) * pow(t, 2);
	 b4 = pow(t, 3);
	 
	 result.x = start.x * b1 + inter1.x * b2 + inter2.x * b3 + end.x * b4;
	 result.y = start.y * b1 + inter1.y * b2 + inter2.y * b3 + end.y * b4;
}

void ellipse_get_x_y(
	point_t &result,
	point_t center,
	length_t radii,
	double t)
{
	result.x = center.x + radii.x * cos(t);
	result.y = center.y + radii.y * sin(t);	
}
	
