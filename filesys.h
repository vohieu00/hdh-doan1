// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.  This is provided in case the
//	multiprogramming and virtual memory assignments (which make use
//	of the file system) are done before the file system assignment.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

#include "copyright.h"
#include "openfile.h"



#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
				// calls to UNIX, until the real file system
				// implementation is available
class FileSystem {
  public:

    OpenFile** fileIndex; // bang mo ta file trong khoi tao cua fileSystem
    int currentSize; // max = 10
    int recentId; // Id vua moi duoc mo
    int flag; // co hieu bao hieu cho viec da mo file stdin, stdout
	
    FileSystem(bool format) {
	fileIndex = new OpenFile*[10];
	currentSize = 0;
	recentId = -1;
	flag = 0;
	for(int i = 0; i < 10; i++) {
		fileIndex[i] = NULL;
	}
	

	this->Create("stdin", 0); // con tro kieu FileSystem
	this->Create("stdout", 0);
	
	fileIndex[0] = this->Open("stdin", 2); // type = 2 cho stdin 
	fileIndex[1] = this->Open("stdout", 3);// type = 3 cho stdout
	
    }
	
    ~FileSystem() {
	
	currentSize = 0;
	recentId = -1;
	
	for(int i = 0; i < 10; i++) {
		if(fileIndex[i] != NULL && i != 2) { 
		// i = 2 la fileIndex tro toi vung nho openFile cua ten chuong trinh vd : /test/create.c
		// nen khong duoc xoa vung nho truoc khi ket thuc chuong trinh
			
			delete fileIndex[i];
			fileIndex[i] = NULL;
		}
	}
    	delete[] fileIndex; 
    }

    
    
    bool Create(char *name, int initialSize) { 
	int fileDescriptor = OpenForWrite(name);
	
	
	if (fileDescriptor == -1) return FALSE;
	Close(fileDescriptor); 
	return TRUE; 
	}

    OpenFile* Open(char *name) {

	int fileDescriptor = OpenForReadWrite(name, FALSE);
	int emptySlot;
	printf("Calling OpenFile without type\n");

	printf("FileName : %s\n", name);
	if (fileDescriptor == -1) return NULL;

	if(strcmp(name, "stdin") == 0) {
		fileIndex[0] = new OpenFile(fileDescriptor);
		currentSize++;
		recentId = 0;
		return fileIndex[0];
	}
	else if(strcmp(name, "stdout") == 0) {
		fileIndex[1] = new OpenFile(fileDescriptor);
		currentSize++;
		recentId = 1;
		return fileIndex[1];
	}
	else {
	  	for (int i = 0; i < 10; i++) {
			if(fileIndex[i] == NULL) {
				emptySlot = i;
				break;
			}	
		}
		fileIndex[emptySlot] = new OpenFile(fileDescriptor);
	  	currentSize++; // mo file thanh cong tang bien diem trong bang mo ta file
	  	recentId = emptySlot;
		
		
	  	return fileIndex[emptySlot];
	  }
      }



    OpenFile* Open(char* name, int type) {

	  int fileDescriptor = OpenForReadWrite(name, FALSE);
	  int emptySlot;
	  printf("Calling OpenFile with type\n");
	  printf("FileName : %s\n", name);
	  if (fileDescriptor == -1) return NULL;
		
	  if(strcmp(name, "stdin") == 0) {
		fileIndex[0] = new OpenFile(fileDescriptor, type);
		currentSize++;
		recentId = 0;
		return fileIndex[0];
	  }
	  else if(strcmp(name, "stdout") == 0) {
		fileIndex[1] = new OpenFile(fileDescriptor, type);
		currentSize++;
		recentId = 1;
		return fileIndex[1];
	  }
	  else {
	  	for (int i = 0; i < 10; i++) {
			if(fileIndex[i] == NULL) {
				emptySlot = i;
				break;
			}
		}
		fileIndex[emptySlot] = new OpenFile(fileDescriptor, type);
	  	currentSize++; // mo file thanh cong tang bien diem trong bang mo ta file
	  	recentId = emptySlot;

	  	return fileIndex[emptySlot];
	  }
	  
    }

    void CloseFileId(int OpenFileId) {

	if(OpenFileId == 0 || OpenFileId == 1) {
		return;
        }
	
	
	delete fileIndex[OpenFileId];
	fileIndex[OpenFileId] = NULL;

	
	currentSize--;
	return;

    }
    bool Remove(char *name) { return Unlink(name) == 0; }

};

#else // FILESYS
class FileSystem {
  public:

    OpenFile** fileIndex; // bang mo ta file trong khoi tao cua fileSystem
    int currentSize; // max = 10	
    int recentId;

    FileSystem(bool format);		// Initialize the file system.
					// Must be called *after* "synchDisk" 
					// has been initialized.
    					// If "format", there is nothing on
					// the disk, so initialize the directory
    					// and the bitmap of free blocks.


    
    bool Create(char *name, int initialSize);  	
					// Create a file (UNIX creat)

    OpenFile* Open(char *name); 	// Open a file (UNIX open)
    OpenFile* Open(char* name, int type); // mo file co doi so mode


    void CloseFileId(int OpenFileId);

    bool Remove(char *name);  		// Delete a file (UNIX unlink)

    void List();			// List all the files in the file system

    void Print();			// List all the files and their contents

  private:
   OpenFile* freeMapFile;		// Bit map of free disk blocks,
					// represented as a file
   OpenFile* directoryFile;		// "Root" directory -- list of 
					// file names, represented as a file
};

#endif // FILESYS

#endif // FS_H

