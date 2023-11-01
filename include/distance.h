#ifndef DIST_H
#define DIST_H

//Distance calculation functions

#include "point.h"
#include <cmath>

double euclidean_distance(struct point, struct point);

double manhattan_distance(struct point, struct point);

#endif