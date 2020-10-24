// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MaxFileLength 32
#define MAX_INT_LENGTH 9
#define MASK_GET_NUM 0xF
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
// Input - 
//Output -
//Purpose: Read data from file

void ProgramCounter()
{
	int step = 4; 
	int count = machine->ReadRegister(PCReg); //PC Register is used
	machine->WriteRegister(PrevPCReg, count); // Assign PC Regiter to PrePCReg
	count = machine->ReadRegister(NextPCReg); 
	machine->WriteRegister(PCReg, count);
	machine->WriteRegister(NextPCReg, count + step);
}

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space


char* User2System(int virtAddr, int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	switch (which)
	{
	case NoException:
		return;
	case SyscallException:
		switch (type) {
		case SC_Halt:
			DEBUG('a', "\n Shutd own, initiated by user program.");
			printf("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			break;
		case SC_Create:
		{
			int virtAddr;
			char* filename;

			DEBUG('a', "\n SC Create call...");
			DEBUG('a', "\n  Reading virtual address of filename");

			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1);
				delete filename;
				return;
				//break;
			}
			DEBUG('a', "\n Finish reading filename.");
			if (!fileSystem->Create(filename, 0))
			{
				printf("\n Error care file '%s' ", filename);
				machine->WriteRegister(2, -1);
				delete filename;
				return;
			}
			machine->WriteRegister(2, 0);
			delete filename;
			break;
		}
		case SC_Read: //int Read(char *buffer, int size, OpenFileId id);
		{			
			//Read "size" bytes from the open file into "buffer
			int PrePos;
			int NewPos;
			char* buff;
			int virtAddr = machine->ReadRegister(4);
			int size = machine->ReadRegister(5);
			int ID = machine->ReadRegister(6);
			if (fileSystem == NULL ) 
			{
				printf("\nNon-existence");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;
			}
			//Copy buffer from System memory space to User memory space
			buff = User2System(virtAddr, size);
			delete buff;
			break;
		}
		case SC_Write:// int Write(char *buffer, int size, OpenFileId id);
		{
			//Write "size" bytes from "buffer" to the open file
			int PrePos;
			int NewPos;
			char* buff;
			int virtAddr = machine->ReadRegister(4);
			int size = machine->ReadRegister(5);
			int ID = machine->ReadRegister(6);
			if (fileSystem == NULL ) 
			{
				printf("\nNon-existence");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;
			}
			//Copy buffer from System memory space to User memory space
			buff = User2System(virtAddr, size);
			delete buff;
			break;
		}
		case SC_ReadInt:
		{	//https://gricad-gitlab.univ-grenoble-alpes.fr/systeme/nachos/tree/master
			// read integer number from console
			// DEBUG('a', "\n Read a integer from console.");
			// printf("\n\n Read a integer from console.");
			int number = 0;
			int len = 0;
			int sign = 0;
			int i = 0;
			char* buff = new char[MAX_INT_LENGTH];
			len = gSynchConsole->Read(buff, MAX_INT_LENGTH);
			sign = buff[0] == '-' ? 1:0;
			for (i = sign; i < len; i++)
			{
				number = number * 10 + (int)(buff[i] & MASK_GET_NUM);
			}
			number = buff[0] == '-' ? -1*number : number;
			machine->WriteRegister(2, number);
			delete buff;
			break;
		}
		case SC_PrintInt:
		{
			// print integer number from console
			// DEBUG('a', "\n Print a integer to console.");
			// printf("\n\n Print a integer to console.");
			break;
		}
		default:
			printf("\n Unexpected user mode exception (%d%d)", which, type);
			break;
			interrupt->Halt();
		}
		ProgramCounter();
	}
}

