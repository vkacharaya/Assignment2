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

void
ExceptionHandler(ExceptionType which)
{
	int type = kernel->machine->ReadRegister(2);

	DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

	switch (which) {
	case SyscallException:
		switch (type) {
		case SC_Halt:
			DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

			SysHalt();

			ASSERTNOTREACHED();
			break;

		case SC_Add:
			DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

			/* Process SysAdd Systemcall*/
			int result;
			result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
				/* int op2 */(int)kernel->machine->ReadRegister(5));

			DEBUG(dbgSys, "Add returning with " << result << "\n");
			/* Prepare Result */
			kernel->machine->WriteRegister(2, (int)result);

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;

			ASSERTNOTREACHED();

			break;
		case SC_Read:
			DEBUG(dbgSys, "Read " << kernel->machine->ReadRegister(4) << "\n");
			/* Process SysAdd Systemcall*/
			{
				int buffer = (int)kernel->machine->ReadRegister(4);
				int code = ReadConsole(buffer);
				kernel->machine->WriteRegister(2, (int)code);
			}
			DEBUG(dbgSys, "Read done" << "\n");

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			return;
			ASSERTNOTREACHED();

			break;
		case SC_Write:
			DEBUG(dbgSys, "Write " << kernel->machine->ReadRegister(4) << "\n");
			{
				int buffer = (int)kernel->machine->ReadRegister(4);
				int code = WriteConsole(buffer);
				kernel->machine->WriteRegister(2, (int)code);
			}

			DEBUG(dbgSys, "Write done" << "\n");

			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}

			return;
			ASSERTNOTREACHED();
			break;
		case SC_Exit:
			std::cout << "Thread Exited " << endl;
			{
				int buffer = (int)kernel->machine->ReadRegister(4);
				cout << "Exit Status Code from the Program: " << buffer << endl;
			}
			for (int i = 0; i < NumPhysPages; i++)
				if (kernel->currentThread == kernel->machine->pageTable[i].threadId)
				{
					kernel->machine->memoryStatus->Clear(i);
					kernel->machine->pageTable[i].valid = FALSE;
					kernel->machine->pageTable[i].threadId = NULL;
					kernel->machine->pageTable[i].virtualPage = -1;
					kernel->machine->pageTable[i].physicalPage = -1;
					bzero(&(kernel->machine->mainMemory[i*PageSize]), PageSize);
				}
			
			std::cout << "Thread Finshed" << endl;
			kernel->currentThread->Finish();
			return;
			ASSERTNOTREACHED();
			break;
		case SC_ThreadYield:
			std::cout << "Thread Yielded - " << kernel->currentThread->getName() << endl;
			/* Modify return point */
			{
				/* set previous programm counter (debugging only)*/
				kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

				/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
				kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

				/* set next programm counter for brach execution */
				kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
			}
			kernel->currentThread->Yield();
			return;
			ASSERTNOTREACHED();
			break;
		case SC_ThreadFork:

			/* Modify return point */
		{
			/* set previous programm counter (debugging only)*/
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

			/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

			/* set next programm counter for brach execution */
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
		}
		ForkThread();

		return;
		ASSERTNOTREACHED();
		break;
		default:
			cerr << "Unexpected system call " << type << "\n";
			break;
		}
		break;
	case PageFaultException:
	{
		unsigned virtualPage = (unsigned)(kernel->machine->ReadRegister(BadVAddrReg) / PageSize);

		//Contents of the Needed Page
		char* vmContents = new char[PageSize];		
		
		//Current Process Location in Page Table
		int newPageLocation = kernel->threadLocations[kernel->currentThread];

		//Read function for getting the Needed Page
		kernel->swapFile->ReadAt(vmContents, PageSize, newPageLocation + virtualPage * PageSize);

		//Virtual Address
		int virtualAddress = kernel->threadLocations[kernel->currentThread] + (virtualPage * PageSize);

		//Empty space finder in the RAM
		int availableFrame = kernel->machine->memoryStatus->FindAndSet();

		if (availableFrame == -1)
		{
			int randomPage = rand() % NumPhysPages;
			int previousVirtualPage = kernel->machine->pageTable[randomPage].virtualPage;
			Thread* prevThreadid = kernel->machine->pageTable[randomPage].threadId;

			char* pageContents = new char[PageSize];
			bcopy(&(kernel->machine->mainMemory[kernel->machine->pageTable[randomPage].physicalPage * PageSize]), pageContents, PageSize);
			kernel->swapFile->WriteAt(pageContents, PageSize, kernel->threadLocations[prevThreadid] + (previousVirtualPage * PageSize));

			kernel->machine->pageTable[randomPage].virtualPage = virtualPage;
			kernel->machine->pageTable[randomPage].valid = TRUE;
			kernel->machine->pageTable[randomPage].threadId = kernel->currentThread;

			bzero(&(kernel->machine->mainMemory[randomPage * PageSize]), PageSize);
			bcopy(vmContents, &(kernel->machine->mainMemory[randomPage * PageSize]), PageSize);

			delete pageContents;
		}
		else
		{
			kernel->machine->pageTable[availableFrame].threadId = kernel->currentThread;
			kernel->machine->pageTable[availableFrame].virtualPage = virtualPage;
			kernel->machine->pageTable[availableFrame].valid = TRUE;
			kernel->machine->pageTable[availableFrame].physicalPage = availableFrame;
			bzero(&(kernel->machine->mainMemory[availableFrame * PageSize]), PageSize);
			bcopy(vmContents, &(kernel->machine->mainMemory[availableFrame * PageSize]), PageSize);
		}
		delete vmContents;
		return;
		ASSERTNOTREACHED();
		break;
	}
	default:
		cerr << "Unexpected user mode exception" << (int)which << "\n";
		break;
	}
	ASSERTNOTREACHED();
}

