#include "revert_string.h"
#include <string.h>
void RevertString(char *str)
{
	int s=strlen(str);
	for (int i=0;i<s/2;i++) {
		char temp=str[i];
		str[i]=str[s-1-i];
		str[s-1-i]=temp;
	}
}

