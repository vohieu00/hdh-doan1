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
//----------------------------------------------------------------------
char* User2System(int virtAddr,int limit) {
	int i;// index
	int oneChar;
 	char* kernelBuf = NULL;

 	kernelBuf = new char[limit +1];//need for terminal string
 	if (kernelBuf == NULL) {
 		return kernelBuf;
	}

 	memset(kernelBuf,0,limit+1);

 	//printf("\n Filename u2s:");
 	for (i = 0 ; i < limit ;i++) {
 		machine->ReadMem(virtAddr+i,1,&oneChar);
 		kernelBuf[i] = (char)oneChar;
 		//printf("%c",kernelBuf[i]);
 		if (oneChar == 0) {
 			break;
		}
 	}
 	return kernelBuf;
} 

int System2User(int virtAddr,int len,char* buffer) { 
 	if (len < 0) return -1;
 	if (len == 0)return len;
 	int i = 0;
 	int oneChar = 0 ;
 	do {
 		oneChar= (int) buffer[i];
 		machine->WriteMem(virtAddr+i,1,oneChar);
 		i++;
 	} while(i < len && oneChar != 0);
 	return i;
}

void advancePC() {
   	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}


void ProgramCounter()
{
	int step = 4; 
	int count = machine->ReadRegister(PCReg); //PC Register is used
	machine->WriteRegister(PrevPCReg, count); // Assign PC Regiter to PrePCReg
	count = machine->ReadRegister(NextPCReg); 
	machine->WriteRegister(PCReg, count);
	machine->WriteRegister(NextPCReg, count + step);

}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	switch (which) 
	{
		case NoException:
			return;
		case PageFaultException:
			printf("PageFaultException\n");
			interrupt->Halt();
			break;
		case ReadOnlyException:
			printf("ReadOnlyException\n");
			interrupt->Halt();
			break;
		case BusErrorException:
			printf("BusErrorException\n");
			interrupt->Halt();
			break;
		case AddressErrorException:
			printf("AddressErrorException\n");
			interrupt->Halt();
			break;
		case OverflowException:
			printf("OverflowException\n");
			interrupt->Halt();
			break;
		case IllegalInstrException:
			printf("IllegalInstrException\n");
			interrupt->Halt();
			break;
		case NumExceptionTypes:
			printf("NumExceptionTypes\n");
			interrupt->Halt();
			break;


		case SyscallException:

 			switch (type) 
		{
			case SC_Halt: {
 				printf ("\nShutdown, initiated by user program.\n");
 				interrupt->Halt();
 				break;
			}
			case SC_Create: {
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
				int virtAddr, mode, retVal;
				char* filename;
				
				virtAddr = machine->ReadRegister(4);
				filename = User2System(virtAddr,MaxFileLength+1);
				
				mode = machine->ReadRegister(5);
				if(fileSystem->currentSize >= 10) {
					printf("Khong the mo file vi bang mo ta file da het o nho\n");
					machine->WriteRegister(2, -1);
					break;
				}


				if(strcmp(filename, "stdin") == 0) {
					if(mode == 2) {
						printf("Mo thanh cong file stdin (Id = 0)\n");
						machine->WriteRegister(2, 0);
						break;
					}
					else {
						printf("Mode khong dung cho stdin\n");
						machine->WriteRegister(2, -1);
						break;
					}
				}

				else if(strcmp(filename, "stdout") == 0) {
					if(mode == 3) {
						printf("Mo thanh cong file stdout (Id = 1)\n");
						machine->WriteRegister(2, 1);
						break;
					}
					else {
						printf("Mode khong dung cho stdout\n");
						machine->WriteRegister(2, -1);
						break;
					}
				}
				else if(mode == 0 || mode == 1) {

					if(fileSystem->Open(filename, mode) == NULL) {
						printf("File khong ton tai\n");
						machine->WriteRegister(2, -1);
						break;
					}

					else if(mode == 0) {
						printf("Mo file %s thanh cong (mode = doc va ghi)\n", filename);
					}
					else {
						printf("Mo file %s thanh cong (mode = doc)\n", filename);
					}
					retVal = fileSystem->recentId;
					printf("OpenFileID cua %s la : %d\n", filename, retVal);
					printf("CurrentSize : %d\n", fileSystem->currentSize);
					machine->WriteRegister(2, retVal);
					break;
				}
				
			}
			case SC_Close: 
			{
				int openFileId;	
				openFileId = machine->ReadRegister(4);
				if(openFileId < 0 || openFileId > 9) {
					printf("OpenFileId %d khong hop le\n", openFileId);
					machine->WriteRegister(2, -1);
					break;
				}
				else {
					fileSystem->CloseFileId(openFileId);
					printf("Close fileID : %d thanh cong\n", openFileId);
					machine->WriteRegister(2, 0);
					break;
				}	

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
				//ProgramCounter();
				break;
			}
			if (fileSystem->fileIndex[ID] == NULL ) 
			{
				printf("\nNon-existence\n");
				machine->WriteRegister(2, -1);
				//ProgramCounter();
				break;
			}
			if (fileSystem->fileIndex[ID]->type == 3) //file stdout
			{
				//int len = gSynchConsole->Read(buff, size);
				//System2User(virtAddr, len, buff);
				printf("Cannot read file stdout.");
				machine->WriteRegister(2, -1);
				//ProgramCounter();
				break;
			}
			//Copy buffer from System memory space to User memory space
			buff = User2System(virtAddr, size);
			if (fileSystem->fileIndex[ID]->type == 2) //file stdin
			{
				int len = gSynchConsole->Read(buff, size);
				System2User(virtAddr, len, buff);
				machine->WriteRegister(2, len);
				delete buff;
				//ProgramCounter();
				break;
			}
			PrePos = fileSystem->fileIndex[ID]->GetCurrentPos();
			if ((fileSystem->fileIndex[ID]->Read(buff, size))> 0)
			{	
				NewPos = fileSystem->fileIndex[ID]->GetCurrentPos();
				System2User(virtAddr, NewPos- PrePos, buff);
				machine->WriteRegister(2, NewPos - PrePos);
				
			}
			else
			{	//printf("\n\n Empty file\n");
				machine->WriteRegister(2, -2);
				
			}
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
				//printf("\n%d", virtAddr);
				int size = machine->ReadRegister(5);
				//printf("\n%d", size);
				int ID = machine->ReadRegister(6);
				//printf("\n%d\n", ID);
				if (ID < 0 || ID > 10)
				{
					printf("\n\n Out of mode table. Cannot write file.");
					machine->WriteRegister(2, -1);
					//ProgramCounter();
					break;
				}
				if (fileSystem->fileIndex[ID] == NULL ) 
				{
					printf("\nNon-existence\n");
					machine->WriteRegister(2, -1);
					//ProgramCounter();
					break;
				}
				//read-only file or stdin file (mode = 2)
				if (fileSystem->fileIndex[ID]->type == 1 || fileSystem->fileIndex[ID]->type == 2)
				{
					printf("\n Cannot not write this file.");
					machine->WriteRegister(2, -1);
					//ProgramCounter();
					break;	
				}
				buff = User2System(virtAddr, size);
				PrePos = fileSystem->fileIndex[ID]->GetCurrentPos();
				//read & write file
				if (fileSystem->fileIndex[ID]->type == 0 
						&& fileSystem->fileIndex[ID]->Write(buff, size) > 0) 
				{
					NewPos = fileSystem->fileIndex[ID]->GetCurrentPos();
					//printf("%d	%d\n", NewPos, PrePos);
					machine->WriteRegister(2, NewPos - PrePos);
					//ProgramCounter();
					delete buff;
					break;
				}
				if (fileSystem->fileIndex[ID]->type == 3) //stdout file
				{
					int i;
					for (i =0; i<strlen(buff); ++i)
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
					gSynchConsole->Write(buff +i, 1);
					machine->WriteRegister(2, i-1);
					//ProgramCounter();
					delete buff;
					break;
					
				}
				
			
			}
			case SC_Seek:
			{
				int pos = machine->ReadRegister(4);
				int fileID = machine->ReadRegister(5);
				//printf("\n\n%d", fileID);
				if(fileID < 0 || fileID > 9)
				{
					printf("\nID is out of range");
					machine->WriteRegister(2, -1);
					//ProgramCounter();
					break;
				}
				if(fileSystem->fileIndex[fileID] == NULL) 
				{
					printf("\nFile does not exist");
					machine->WriteRegister(2, -1);
					//ProgramCounter();
					break;
				}
				//printf("%d", fileID);
				if(fileID == 0 || fileID == 1)
				{
					printf("\nCan not seek to file");
					machine->WriteRegister(2, -1);			
				}
			
				pos = (pos == -1) ? fileSystem->fileIndex[fileID]->Length() : pos;
				if(pos > fileSystem->fileIndex[fileID]->Length() || pos < 0)
				{
					printf("\nCan not seek file to this position");
					machine->WriteRegister(2, -1); 
				}
				else
				{
					fileSystem->fileIndex[fileID]->Seek(pos);
					machine->WriteRegister(2, pos);
				}
				//ProgramCounter();
				break;
			}
			
			
			case SC_PrintStr:
			{
				
				int buffAddr = machine->ReadRegister(4);
				int i = 0;
				char* buff;
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
				char* buff;
				int virtAddr = machine->ReadRegister(4);
				int len = machine->ReadRegister(5);
				buff = User2System(virtAddr, len);
				gSynchConsole->Read(buff, len);
				System2User(virtAddr, len, buff);
				delete[] buff;
				//ProgramCounter();
				break;
			}

		}
		ProgramCounter();
	}	
}

