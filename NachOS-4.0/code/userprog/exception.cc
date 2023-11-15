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
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#define MaxFileLength 32
#define READ_ONLY 1
#define READ_WRITE 0
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
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

void IncreasePC()
{
	// set previous program counter to current programcounter
	kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	/* set next programm counter for brach execution */
	kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
}
char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; // need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	// printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		// printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}
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
int findEmptySlot(FileSystem *temp)
{

	for (int i = 2; i < 20; i++)
	{
		if (kernel->fileSystem->fileDescriptorTable[i].openFile == NULL)
		{
			return i;
		}
	}
	return -1;
}
bool checkValid(FileSystem *temp, int fileDescriptor); // check if the file descritor exist or in range
bool checkValid(FileSystem *temp, int fileDescriptor)
{
	if (fileDescriptor < 2 || fileDescriptor >= 20 || temp->fileDescriptorTable[fileDescriptor].openFile == NULL)
	{
		return false;
	}
	return true;
}
int close_syscall(FileSystem *temp, int fileDescriptor)
{
	if (!checkValid(temp, fileDescriptor))
	{
		return -1;
	}

	delete temp->fileDescriptorTable[fileDescriptor].openFile;
	temp->fileDescriptorTable[fileDescriptor].openFile = NULL;

	return 0;
}
void ExceptionHandler(ExceptionType which)
{

	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which)
	{
	case SyscallException:
		switch (type)
		{

		case SC_Create:
		{
			int virtAddr;
			char *filename = new char[MaxFileLength + 1];
			DEBUG('a', "\n SC_Create call ...");
			DEBUG('a', "\n Reading virtual address of filename");
			virtAddr = kernel->machine->ReadRegister(4);
			DEBUG('a', "\n Reading filename.");

			// MaxFileLength lÃ  = 32
			filename = User2System(virtAddr, MaxFileLength + 1);
			if (filename == NULL)
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				IncreasePC();
				return;
			}

			DEBUG('a', "\n Finish reading filename.");
			if (!kernel->fileSystem->Create(filename))
			{
				printf("\n Error create file '%s'", filename);
				kernel->machine->WriteRegister(2, -1);
				delete filename;
				IncreasePC();
				return;
			}
			kernel->machine->WriteRegister(2, 0);
			delete filename;
			// delete fileSystem;
			IncreasePC();

			break;
		}
		case SC_Open:
		{
			int bufAddr = kernel->machine->ReadRegister(4); // read string pointer from user
			int type = kernel->machine->ReadRegister(5);	// vua sua lai
			char *buf = User2System(bufAddr, MaxFileLength);
			DEBUG(dbgSys, "Here\n");
			DEBUG(dbgSys, "Stop\n");

			int temp_index = findEmptySlot(kernel->fileSystem);
			OpenFile *temp = kernel->fileSystem->Open(buf);
			if (temp_index == -1 || type < 0 || type > 1 || temp == NULL)
			{
				DEBUG('f', "Cannot open file");
				DEBUG(dbgSys, "Can not open file");
				kernel->machine->WriteRegister(2, -1);
			}
			else
			{

				DEBUG(dbgSys, "open file successfully");
				DEBUG(dbgSys, buf << " " <<temp_index) ;
				
				kernel->machine->WriteRegister(2, temp_index);
				kernel->fileSystem->fileDescriptorTable[temp_index].openFile = temp;
				kernel->fileSystem->fileDescriptorTable[temp_index].mode = type;
			}
			delete[] buf;
			IncreasePC();
			break;
		}
		case SC_Close:
		{
			int index_close = kernel->machine->ReadRegister(4);
			int result = close_syscall(kernel->fileSystem, index_close);
			kernel->machine->WriteRegister(2, result);
			DEBUG(dbgSys, "index " << index_close);
			if (result == -1)
			{
				DEBUG(dbgSys, "Fail to close");
			}
			else
			{
				DEBUG(dbgSys, "Close file");
			}
			IncreasePC();
		}
		case SC_Read:
		{

			int bufAddr = kernel->machine->ReadRegister(4);
			int NumBuf = kernel->machine->ReadRegister(5);
			int fileID = kernel->machine->ReadRegister(6);
			int OldPos;
			int NewPos;

			char *buf = new char[NumBuf];
			int bytesRead = 0;

			if (fileID >= 20 || fileID == 1) 
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				break;
			}
		
			if (fileID > 1 && (kernel->fileSystem->fileDescriptorTable[fileID].openFile == NULL || kernel->fileSystem->fileDescriptorTable[fileID].mode == 1 ))  
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				break;
			}

			OldPos = kernel->fileSystem->fileDescriptorTable[fileID].openFile->GetCurrentPos();
			buf = User2System(bufAddr, NumBuf);

			if (fileID == 0) // Console input
			{
				int i = 0;
				//kernel->synchConsoleIn = new SynchConsoleInput(buf); 
				while (i < NumBuf - 1) // Read up to NumBuf - 1 characters
				{
					char ch = kernel->synchConsoleIn->GetChar();
					buf[i] = ch;

					if (ch == '\n' || ch == '\0')
						break;

					i++;
				}

				buf[i] = '\0'; // Null-terminate the buffer
				System2User(bufAddr, i, buf);
				kernel->machine->WriteRegister(2, i);
			}
			else
			{
				bytesRead = kernel->fileSystem->fileDescriptorTable[fileID].openFile->Read(buf, NumBuf);
			}

			if (bytesRead > 0)
			{
				// Copy data from kernel to user space
				NewPos = kernel->fileSystem->fileDescriptorTable[fileID].openFile->GetCurrentPos();
				System2User(bufAddr, bytesRead, buf);
				kernel->machine->WriteRegister(2, bytesRead);
				DEBUG(dbgSys, "Reading successfully \n" << buf);

			}
			else
			{
				kernel->machine->WriteRegister(2, -1);
				delete[] buf;
			}

			delete[] buf;
			IncreasePC();
			break;
		}
		case SC_Write:
		{
			int bufAddr = kernel->machine->ReadRegister(4);
			int NumBuf = kernel->machine->ReadRegister(5);
			int fileID = kernel->machine->ReadRegister(6);
			int OldPos;
			int NewPos;
			
			// mode = 1 means that openFile is just for reading only
			DEBUG(dbgSys, "Here: " << fileID);
			if (fileID >= 20 || fileID == 0) 
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				break;
			}
		
			if (fileID > 1 && (kernel->fileSystem->fileDescriptorTable[fileID].openFile == NULL || kernel->fileSystem->fileDescriptorTable[fileID].mode == 1 ))  
			{
				kernel->machine->WriteRegister(2, -1);
				IncreasePC();
				break;
			}
			
			DEBUG(dbgSys, "Here\n");
			// WRITE TO CONSOLE BY SYMCHCONSOLEOUT
			char *buf  = User2System(bufAddr, NumBuf);
			if (fileID == 1)
			{
				int i = 0;
				DEBUG(dbgSys, "fileID = 1\n");
				//kernel->synchConsoleOut = new SynchConsoleOutput(buf);
				while (buf[i] != '\0' && buf[i] != '\n')
				{
					kernel->synchConsoleOut->PutChar(buf[i]);
					i++;
				}
				buf[i] = '\n';
				kernel->synchConsoleOut->PutChar(buf[i]); // write last character

				kernel->machine->WriteRegister(2, i -1 );
				delete[] buf;
				IncreasePC();
				return;
				break;
			}

			// WRITE TO FILE
			int before_write = kernel->fileSystem->fileDescriptorTable[fileID].openFile->GetCurrentPos();
			if ((kernel->fileSystem->fileDescriptorTable[fileID].openFile->Write(buf, NumBuf)) != 0)
			{
				int after_write = kernel->fileSystem->fileDescriptorTable[fileID].openFile->GetCurrentPos();
				System2User(bufAddr, after_write - before_write, buf);
				kernel->machine->WriteRegister(2, after_write - before_write);
				delete[] buf;
				IncreasePC();
				return;
				break;
			}

			IncreasePC();
			return;
			break;
		}
		case SC_Halt:
		{
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;
		}

		case SC_Add:
		{
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */ (int)kernel->machine->ReadRegister(4),
							/* int op2 */ (int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			IncreasePC();

			return;

			ASSERTNOTREACHED();

			break;
		}
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		ASSERTNOTREACHED();
		break;
	}
}
