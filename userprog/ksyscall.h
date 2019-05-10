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

#include "kernel.h"




void SysHalt()
{
	kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
	return op1 + op2;
}


int ReadConsole(int bufferAddress)
{
	char * buffer = "Hi I am Coming from Console\n";
	char c = *(buffer);
	int i = 0;
	while (c != '\0')
	{
		kernel->machine->WriteMem(bufferAddress, 1, c);
		c = *(buffer + ++i);
		bufferAddress++;
	}
	return 1;
}

int WriteConsole(int buffer)
{
	char d = 's';
	while (d != '\0')
	{
		int c = 0;
		kernel->machine->ReadMem(buffer, 1, &c);
		std::cout << (char)c;
		d = (char)c;
		buffer++;
	}
	std::cout << (char)d;
	return 1;
}

void RealThread(int which) {
	std::cout << "Inside Thread Fork" << endl;
	for (int i = 0; i < 1000; i++)
		continue;
	std::cout << "Coming out of Thread Fork" << endl;
}

void ForkThread()
{
	Thread * forkThread = new Thread("Fork");
	forkThread->Fork((VoidFunctionPtr)RealThread, (void *)0);
}







#endif /* ! __USERPROG_KSYSCALL_H__ */
