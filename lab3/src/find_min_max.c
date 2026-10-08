#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  int tempmax=0, tempmin=array[0];
  for (int i=begin;i<end;i++) {
    if (array[i]>tempmax) {tempmax=array[i];}
    if (array[i]<tempmin) {tempmin=array[i];}
  }
  min_max.min=tempmin;
  min_max.max=tempmax;
  return min_max;
}
