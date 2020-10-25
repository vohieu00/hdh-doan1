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
				printf("Calling SC_Create\n");
				int virtAddr, MaxFileLength = 32;
				char* filename;
				
				virtAddr = machine->ReadRegister(4);
				filename = User2System(virtAddr,MaxFileLength+1);
				if (filename == NULL) {
					printf("\n Not enough memory in system");
					machine->WriteRegister(2,-1);
					delete filename;
					return;
				}
				
				if (!fileSystem->Create(filename,0)) {
					printf("\n Error create file '%s'",filename);
					machine->WriteRegister(2,-1);
					delete filename;
					return;
				}
				
				advancePC();
				machine->WriteRegister(2,0);
				delete filename;
				
				break; 
			}
			case SC_Open: 
			{

				printf("Calling SC_Open\n");
				int virtAddr, MaxFileLength = 32, mode;
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

			}
			advancePC();
			break;
			
		}	
	}	
}
