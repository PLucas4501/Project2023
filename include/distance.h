#ifndef DIST_H
#define DIST_H

//Distance calculation functions

#include "point.h"
#include <cmath>

float euclidean_distance(struct point, struct point);

float manhattan_distance(struct point, struct point);

#endif