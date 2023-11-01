#ifndef DIST_H
#define DIST_H

//Distance calculation functions

#include <cmath>
#include "point.h"

double euclidean_distance(struct point, struct point);

double manhattan_distance(struct point, struct point);

#endif