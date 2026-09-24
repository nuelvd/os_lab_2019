#include "swap.h"

void Swap(char *left, char *right)
{
	char k=*left;
	*left=*right;
	*right=k;
}
