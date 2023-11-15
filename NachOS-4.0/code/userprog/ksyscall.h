/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__
#define __USERPROG_KSYSCALL_H__

#include "syscall.h"
#include "main.h"
#include "filehdr.h"
#define MaxFileLength 32

/*  Input:  - User space address (int)
			- Limit of buffer (int)
	Output: - Buffer (char*)
	Purpose: Copy buffer from User memory space to System memory space */
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = new char[limit + 1]; // need for terminal string

	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}

	return kernelBuf;
}

/*  Input:  - User space address (int)
			- Limit of buffer (int)
			- Buffer (char[])
	Output: - Number of bytes copied (int)
	Purpose: Copy buffer from System memory space to User memory space */
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;

	if (len == 0)
		return len;

	int i = 0;
	int oneChar = 0;

	do
	{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);

	return i;
}

void SysHalt()
{
	kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
	cout << op1 + op2 << endl;
	return op1 + op2;
}

int SysCreate(char *filename)
{
	if (strlen(filename) == 0)
	{
		return -1;
	}

	if (filename == NULL)
	{
		return -1;
	}

	if (!kernel->fileSystem->Create(filename))
	{
		return -1;
	}
	cout << "create file: " << filename << " success!" << endl;
	return 0;
}


int SysRemove(char *filename)
{
	if (strlen(filename) == 0)
	{
		cout << "Remove failed" << endl;
		return -1;
	}

	if (filename == NULL)
	{
		cout << "Remove failed" << endl;
		return -1;
	}

	if (!kernel->fileSystem->Remove(filename))
	{
		cout << "Remove failed: " << filename << " not exist" << endl;
		return -1;
	}
	cout << "Remove file: " << filename << " success!" << endl;
	return 0;
}

// int Seek(int position, OpenFileId id){

// 			// seek into files: stdin, stdout, `out of domain` fileSystem
// 			if (id < 1 || id > kernel->fileSystem. || kernel->fileSystem->openfile[id] == NULL)
// 			{
// 						printf("\nFile name is not valid");
// 				return -1;
// 			}

// 			int len = kernel->fileSystem->openfile[id]->Length();	// file size

// 			if (position > len)	// try to move file ptr to pos, pos > len --> wrong
// 			{
// 				kernel->machine->WriteRegister(2, -1);
// 				break;
// 			}

// 			if (pos == -1)	// move file ptr to the begining of file
// 				pos = len;

// 			kernel->fileSystem->openfile[id]->Seek(pos);
// 			machine->WriteRegister(2, pos);
// 			break;
// }
#endif /* ! __USERPROG_KSYSCALL_H__ */
