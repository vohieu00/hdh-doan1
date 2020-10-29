#include "syscall.h"
#include "copyright.h"

int main()
{
	char srcfile[MaxLineLength];
	char dstfile[MaxLineLength];
	int src, dst, start_src_pos, start_dst_pos, len;
	char line[MaxLineLength];
	PrintStr("Destination file name: ");
	ReadStr(dstfile, MaxFileLength);
	PrintStr("Source file name: ");
	ReadStr(srcfile, MaxFileLength);
	PrintStr(srcfile);
	PrintStr(dstfile);
	src = Open(srcfile, 1);
	if (src == -1)
	{
		PrintStr("Cannot open this file");
		return 0;
	}
	dst = Open(dstfile, 0);
	if (dst == -1)
	{
		PrintStr("Cannot open this file.");
		return 0;
	}
	start_dst_pos = Seek(-1, dst);//move to end of dst file
	start_src_pos = Seek(0, src);//move to start of dst file
	len = Read(line, MaxFileLength, src);
	while (len > 0)
	{	//PrintStr(line);
		Write(line, len, dst);
		len = Read(line, MaxFileLength, src);
	}
	Close(dst);
	Close(src);
	
	return 0;
}
