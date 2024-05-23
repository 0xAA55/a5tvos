
#include <stdio.h>

int main(int argc, char** argv, char** envp)
{
	FILE *fp = fopen("/dev/tty1", "w");
	fprintf(fp, "Hello World!\n");
	fclose(fp);
	return 0;
}


