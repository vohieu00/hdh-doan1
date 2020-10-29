#include "syscall.h"
#include "copyright.h"
#define maxlen 32

int main()
{
	int a;
	char file[MaxFileLength];
	PrintStr("Nhap ten file: ");
	ReadStr(file, MaxFileLength);
	a = Create(file);
//	Halt();
}


