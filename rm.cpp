#include <windows.h>

int main(int argc,char** argv)
{
	if(argc>1)
		DeleteFile(argv[1]);
	return 0;
}
