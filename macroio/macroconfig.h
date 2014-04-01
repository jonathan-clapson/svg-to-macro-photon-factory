#ifndef __MACRO_CONFIG_H__
#define __MACRO_CONFIG_H__

#include <stdio.h>
#include <stdarg.h>

#include "laser_types.h"

/* Define the paper width and height to limit output format */
const int mw_paper_width_um = 130000;
const int mw_paper_height_um = 130000;

const double mw_paper_half_width_um = mw_paper_width_um/2;
const double mw_paper_half_height_um = mw_paper_height_um/2;
const double mw_paper_half_width_nm = mw_paper_width_um*1000/2;
const double mw_paper_half_height_nm = mw_paper_height_um*1000/2;
#endif /* __MACRO_CONFIG_H__ */
