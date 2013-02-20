Kevin Mangan
kmm2256
Operating Systems
Assignment 1

Documentation of files
---------------------------
makefile

myshell.c: contains all the code for the shell
myshell: exectuable shell program
input.txt: contains random words
input2.txt: contains text (input txt and new.txt)
testrun.txt: contains a test run of the shell

Design specs
--------------------------

1. To keep track of the path names, I used a linked list
2. I parsed the command line and if I found a redirection symbol or pipe symbol, main would call the appropriate function
3. If now parsing symbol was found, then the normal execute() function was called

Problems
--------------------------

There seems to be some trouble chaining commands such as: sort input2.txt | grep input.txt > new.txt

Also, sometimes you must enter exit a couple times to get it to actually exit


WRITTEN PROBLEMS
-------------------------------------------------------------------------

1. Operating systems sit quietly waiting for something to do (or an interrupt). This interrupt could be a process to execute, an I/O device to service, or a user to respond.
	Interrupt is either caused by hardware or software.
	A trap/exeption is caused by software either from an error or a request from the user.

2. 
	a. After setting up buffers, pointers, and counters for the device, the device controller transfers an entire block of data directly to or from its own buffer to memory
		with no intervention by the CPU
	b. Using DMA, an interrupt tells the device driver when the operation is over, so the CPU does not know.
	c. There is no interference with user programs when using DMA

3. Not practical: Priveliged mode provides a means for protecting the system from errant users. This is so there can be certain instructions (privileged instructions) that can only be run
			in priveliged mode.
   Practical: If it is a very small system with only one user or very little functionality (like an embedded system) then it is fine to not have a privileged mode.

4. Some things you may want to take into account when designing an OS for a mobile device:
	- Battery life is limited, so you want your OS to be mindful of power consumtion and efficiency
	- Memory is much more limited on smaller devices (for now) and OS must be more compact
	- Unique I/O like GPS and Accelerometers must be compatible with the OS

5. Advantage: 
	The microkernel structures the OS by removing all non-essential components from the kernel and implementing them as system and user level programs.
	This makes the kernel smaller. They also provide minimal process and memory management, in addition to communication facility. System programs and user programs interact
	indirectly by passing messages to the kernel.

   Disadvantage:
	Performance of the microkernel can suffer due to system-function overhead.

6. Loadable Kernel Modules are efficient because some services can be implemented dynamically at run-time. This is better than putting them directly into the kernel because then you don't have
	to recompile the whole kernel every time you add some sort of implementation. Also, any module can call any other module.
	So basically it is more flexible.

7. Loadable kernel modules can call each other without having to go through the actual kernel, whereas in a microkernel, you need message passing for the system and user programs
	to interact with each other. 
   Loadable kernel modules cannot replace microkernels because they only have a little bit of functionality for the kernel, not the whole kernel. They are like add-ons or
	accessories.

8. First way: Multiprogramming is a way to increase CPU utilization by organizing jobs so the CPU always has one to execute. Applications take turns waiting in a job pool
			and when one process has to wait, the CPU gets to work on another one. This is usually used for CPU, memory, and display.
   Second way: Time Sharing is another way. The CPU executes multiple jobs by switching among them, but the switches occur so frequently that the user can interact with 
			each program while it is running. This is typically used by Disk, Audio-device, and Network-device
