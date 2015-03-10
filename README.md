## Synopsis

The project is designed to provide capabilities to achieve RPC data serialization and deserialization on typical UNIX system calls, such as open, close, read, write, lseek, unlink, stat, getdirentries, as well as two self-defined functions, which are getdirtree and freedirtree.

An RPC protocol is designed to marshall and unmarshall data structures to achieve data serialization and deserialization so that RPC is transparent to client.

## Code Example

The tcp-sample directory has a sample code for a simple server and client.

The interpose directory has code for creating a interposition library. and networking code to log the operations to the remote server. 

## Installation

Run "make" in Interpose directory to build the programs.

## Tests

The tools directory has a few programs to test the code. These are binary-only tools that operate on the local filesystem.  You will make them operate across the network by interposing on their C library calls. Run any of these tools without arguments for a brief message on how to use it.  These binaries should work on x86 64-bit Linux systems (e.g., unix.andrew.cmu.edu servers).  

To use the interposing library, try (if using BASH shell):

	LD_PRELOAD=./interpose/mylib.so ./tools/440read README

or (if using CSH, TCSH, ...):

	env LD_PRELOAD=./interpose/mylib.so ./tools/440read README

You should see a message indicating the open function in the interpositon library was called.  

Note that the 440tree tool uses the getdirtree function implemented in libdirtree.so in the lib directory.  Please add the absolute path of this directory to LD_LIBRARY_PATH
to make sure that the system can find the library, e.g. on BASH:

	export LD_LIBRARY_PATH="$LD_LIBRARY_PATH;$PWD/lib"

or if using CSH, TCSH, ...:

	setenv LD_LIBRARY_PATH "$LD_LIBRARY_PATH;$PWD/lib"

## Contributors
Thanks to CMU 15440 staff with sample start code.

Most of the code in Interpose directory is written by Xinkai Wang.

Contact for details: xinkaiw [AT] andrew [DOT] cmu [DOT] edu