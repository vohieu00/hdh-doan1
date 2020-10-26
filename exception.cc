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
//-----------------------------------------------------------------------
//https://gricad-gitlab.univ-grenoble-alpes.fr/systeme/nachos/tree/master
//https://github.com/dangkhoasdc/nachos/blob/master/userprog/exception.cc
//-----------------------------------------------------------------------
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
	case PageFaultException:
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		interrupt->Halt();
		break;

	case ReadOnlyException:
		DEBUG('a', "\n Write attempted to page marked 'read-only'");
		printf("\n\n Write attempted to page marked 'read-only'");
		interrupt->Halt();
		break;

	case BusErrorException:
		DEBUG('a', "\n Translation resulted invalid physical address");
		printf("\n\n Translation resulted invalid physical address");
		interrupt->Halt();
		break;

	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\nInteger overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		interrupt->Halt();
		break;

	case NumExceptionTypes: 
		DEBUG('a', "\n Number exception types");
		printf("\n\n Number exception types");
		interrupt->Halt();
		break;

	case SyscallException:
		switch (type) {
		case SC_Halt:
			DEBUG('a', "\n Shut down, initiated by user program.");
			printf("\n\n Shutdown, initiated by user program.");
			interrupt->Halt();
			return;
		case SC_Create:
		{
			int virtAddr;
			char* filename;

			DEBUG('a', "\n SC CreateFile call...");
			DEBUG('a', "\n  Reading virtual address of filename");

			virtAddr = machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");
			filename = User2System(virtAddr, MaxFileLength + 1);

			if (strlen(filename) == 0)
			{
				printf("\nFile name is wrong!!!");
				machine->WriteRegister(2, -1); 
				delete filename;
				break;
			}

			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1);
				delete filename;
				return;
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
		case SC_Open: 
		{
			printf("Calling SC_Open\n");
			int virtAddr, mode;
			char* filename;
				
			virtAddr = machine->ReadRegister(4);
			filename = User2System(virtAddr,MaxFileLength+1);
				
			mode = machine->ReadRegister(5);
			printf("Mode = %d\n", mode);
				
			if(mode < 0 || mode > 1) {
				printf("Khong ton tai mode tren\n");
				machine->WriteRegister(2, -1);
					
			}
				
			else if(fileSystem->currentSize >= 10) {
				printf("Khong the mo file vi bang mo ta file da het o nho\n");
				machine->WriteRegister(2, -1);
			}
			else if(mode == 0 || mode == 1) {
			
				if(fileSystem->Open(filename, mode) == NULL) {
					printf("File khong ton tai\n");
					machine->WriteRegister(2, -1);
				}
				if(mode == 0) {
					printf("Mo file %s thanh cong (mode = doc va ghi)\n", filename);
					machine->WriteRegister(2, 0);
				}
				else {
					printf("Mo file %s thanh cong (mode = doc)\n", filename);
					machine->WriteRegister(2, 0);
				}
				printf("OpenFileID cua %s la : %d\n", filename, (fileSystem->currentSize) -1);
			}
			break;
			
		}
		case SC_Close: 	
		{
			int openFileId;	
			openFileId = machine->ReadRegister(4);
			if(openFileId < 2 || openFileId > 9 || openFileId >= fileSystem->currentSize) {
				printf("OpenFileId %d khong hop le\n", openFileId);
				machine->WriteRegister(2, -1);
			}
			else {
				fileSystem->CloseFileId(openFileId);
				printf("Close fileID : %d thanh cong\n", openFileId);
				machine->WriteRegister(2, 0);
			}	

			ProgramCounter();
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
			if (ID < 0 || ID > 10)
			{
				printf("\n\n Out of mode table. Cannot read file.");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;
			}
			if (fileSystem->fileIndex[ID] == NULL ) 
			{
				printf("\nNon-existence");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;
			}
			if (fileSystem->fileIndex[ID]->type == 3) //file stdout
			{
				int len = gSynchConsole->Read(buff, size);
				System2User(virtAddr, len, buff);
				machine->WriteRegister(2, len);
				ProgramCounter();
				delete buff;
				return;
			}
			//Copy buffer from System memory space to User memory space
			buff = User2System(virtAddr, size);
			if (fileSystem->fileIndex[ID]->type == 2)
			{
				int len = gSynchConsole->Read(buff, size);
				System2User(virtAddr, len, buff);
				machine->WriteRegister(2, len);
				delete buff;
				ProgramCounter();
				return;
			}
			PrePos = fileSystem->fileIndex[ID]->GetCurrentPos();
			if ((fileSystem->fileIndex[ID]->Read(buff, size))> 0)
			{	
				NewPos = fileSystem->fileIndex[ID]->GetCurrentPos();
				System2User(virtAddr, NewPos- PrePos, buff);
				machine->WriteRegister(2, NewPos - PrePos);
			}
			else
			{	printf("\n\n Empty file");
				machine->WriteRegister(2, -2);
			}
			delete buff;
			ProgramCounter();
			return;
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
			if (fileSystem->fileIndex[ID] == NULL ) 
			{
				printf("\nNon-existence");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;
			}
			//read-only file and stdin file
			if (fileSystem->fileIndex[ID]->type == 1 || fileSystem->fileIndex[ID]->type == 2)
			{
				printf("\n Cannot not write this file.");
				machine->WriteRegister(2, -1);
				ProgramCounter();
				return;	
			}
			buff = User2System(virtAddr, size);
			PrePos = fileSystem->fileIndex[ID]->GetCurrentPos();
			if (fileSystem->fileIndex[ID]->type == 0 && fileSystem->fileIndex[ID]->Write(buff, size) > 0)
			{
				NewPos = fileSystem->fileIndex[ID]->GetCurrentPos();
				machine->WriteRegister(2, NewPos - PrePos);
				ProgramCounter();
				delete buff;
				return;
			}
			if (fileSystem->fileIndex[ID]->type == 3)
			{
				int i;
				for (i =0; i<strlen(buff); i++)
				{
					if (buff[i] != 0 && buff[i]!= '\n')
					{
						gSynchConsole->Write(buff + i, 1);
					}
					else
					{
						break;
					}
				}
				buff[i] = '\n';
				gSynchConsole->Write(buff+1, 1);
				machine->WriteRegister(2, i-1);
				ProgramCounter();
				delete buff;
				return;
			}

			//Copy buffer from System memory space to User memory space
			
		}
		case SC_ReadInt:
		{	
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
			DEBUG('a', "\n Print a integer to console.");
			printf("\n\n Print a integer to console.");
			
			break;
		}
		case SC_PrintStr:
		{
			int buffAddr = machine->ReadRegister(4);
			int i = 0;
			char* buff = new char[MaxLineLength];
			buff = User2System(buffAddr, MaxLineLength);

			while (buff[i] != 0 && buff[i] != '\n')
			{
				gSynchConsole->Write(buff+i, 1);
				i++;
			}

			buff[i]='\n';
			gSynchConsole->Write(buff+i, 1);
			delete[] buff;
			break;			
		}
		case SC_ReadStr:
		{
			char *buff = new char[MaxLineLength];
			if (buff == NULL) break;
			int bufAddrUser = machine->ReadRegister(4);
			int len = machine->ReadRegister(5);
			int size = gSynchConsole->Read(buff, len);
			System2User(bufAddrUser, size, buff);
			delete[] buff;
			return;
		}
		default:
			printf("\n Unexpected user mode exception (%d%d)", which, type);
			break;
			interrupt->Halt();
		}
		ProgramCounter();
	}
}


