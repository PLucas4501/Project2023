#ifndef DATA_IMPORT_H
#define DATA_IMPORT_H

#include <iostream>
#include <cstdio>
#include "vector.h"
#include "point.h"

#define IMPORT_PATH "../datasets/"
#define EXPORT_PATH "../solved/"

void binary(const char *, const unsigned int, vector<struct point>&);

#endif