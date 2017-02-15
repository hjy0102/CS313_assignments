#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include "printInternalReg.h"

#define ERROR_RETURN -1
#define SUCCESS 0

#define TRUE 1
#define FALSE 0

uint64_t fileLen; // length of file (number of bytes)
unsigned char instrBuffer[10]; // instruction buffer 
unsigned char icode_ifun; // char containing icode and ifun
nibble* icode_ifun_arr; // array containing icode, ifun
unsigned char rA_rB; // char containing rA and rB
nibble* rA_rB_Array; // array contains rA, rB

struct fetchRegisters fReg;

int bytesToRead; // bytes to be read from mem for particular instruction
int numBytesCanRead; // num of bytes able to read from mem !!! is this necessary?
int notEnoughBytes = FALSE; // flag for too few bytes

char* fileContents;  // char array of file contents;
unsigned int numConsecutiveHalts = 0; 

int resetInstructBuffer() {
  for (int i = 0; i < 10; i++) {
    instrBuffer[i] = 0;
  }
}

// returns number of bytes needed to read given the icode
int getBytesToRead(nibble icode){
  switch (icode) {
    case 0:
      return 1;
    case 1:
      return 1;
    case 2:
			return 2;
		case 3:
			return 10;
		case 4:
			return 10;
		case 5:
			return 10;
		case 6:
			return 2;
		case 7:
			return 9;
		case 8:
			return 9;
		case 9:
			return 1;
		case 0xA:
			return 2;
		case 0xB:
			return 2;
		default:
			// icode not recognized. Print message and exit.
			printf("Invalid opcode %.2x at 0x%016llX\n", icode_ifun, fReg.PC);
			exit(0);
			break;				
	}
	return 0;
}

// dec to hex in an array 
// little endian -- most sig at array[0]
nibble* decToHex(unsigned char* c) {
  static nibble arr[2];
  arr[0] = *c / 16;
  arr[1] = *c % 16;
  return arr;
}

// reads 8 bytes from instrBuffer starting from offsetFromPC 
// and returns them into one int
uint64_t getValC(int offsetFromPC) {
  uint64_t valC_ByteArray[8];
  for (int i = offsetFromPC; i <= offsetFromPC + 8; i++) {
    valC_ByteArray[i - offsetFromPC] = instrBuffer[i];
  }

  uint64_t new_valC = 0;

  for (int i = 0; i < 8; i++ ) {
    valC_ByteArray[i] = valC_ByteArray[i] << (i*8);
    new_valC = new_valC | valC_ByteArray[i];
  }
  return new_valC;
}

// If ifun is non-zero, prints out where the function code is invalid and exits. This is called if the instruction expects ifun to be zero.
void assertZeroiFunCode(nibble ifun, unsigned char icode_ifun) {
	if (ifun != 0) {
		printf("Invalid function code %.2x at 0x%016llX\n",icode_ifun,fReg.PC);
		exit(0);
	}
}

// Sets rA and rB for printReg function
void set_rA_rB() {
	fReg.regsValid = 1;
	rA_rB = instrBuffer[1];
	rA_rB_Array = decToHex(&rA_rB);
  // casting each to nibble
	fReg.rA = (nibble) rA_rB_Array[0];
	fReg.rB = (nibble) rA_rB_Array[1];
}

// Sets valC and byte0 to byte7 for printReg. Only used by 9 or 10 byte instructions.
void setValC_and_bytes(int numBytesInInstr) {
	int PC_offset = numBytesInInstr - 8;
	
	fReg.valCValid = 1;
	fReg.valC = getValC(PC_offset);
	
	fReg.byte0 = instrBuffer[PC_offset];
	fReg.byte1 = instrBuffer[PC_offset + 1];
	fReg.byte2 = instrBuffer[PC_offset + 2];
	fReg.byte3 = instrBuffer[PC_offset + 3];
	fReg.byte4 = instrBuffer[PC_offset + 4];
	fReg.byte5 = instrBuffer[PC_offset + 5];
	fReg.byte6 = instrBuffer[PC_offset + 6];
	fReg.byte7 = instrBuffer[PC_offset + 7];	
}

// Sets parameters for printReg for 10 byte instructions except instr.
void setParams_10ByteInstruct() {
	set_rA_rB();	
	setValC_and_bytes(10);	
	fReg.valP = fReg.PC + 10;
}

void setParams_9ByteInstruct() {
  fReg.regsValid = 0;
	setValC_and_bytes(9);		
	fReg.valP = fReg.PC + 9;
}

// Sets parameters for printReg for 2 byte instructions except instr.
void setParams_2ByteInstruct() {
	set_rA_rB();
	fReg.valCValid = 0;			
	fReg.valP = fReg.PC + 2;	
}

// Sets parameters for printReg for 1 byte instructions except instr.
void setParams_1ByteInstruct() {
	fReg.regsValid = 0;
	fReg.valCValid = 0;
	fReg.valP = fReg.PC + 1;
}

// process the instruction and call printRegS

void processInstr() {
	if (fReg.icode != 0) {
		numConsecutiveHalts = 0;
	}
	switch (fReg.icode) {
		case 0: 
			fReg.instr = "halt";
			// if invalid ifun, exit
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			// set params
			setParams_1ByteInstruct();
			// increment number of consecutive halts
			numConsecutiveHalts++;
			break;

		case 1:
			fReg.instr = "nop";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_1ByteInstruct();
			break;

		case 2: 
			// get instr based on ifun
			switch (fReg.ifun) {
				case 0:
					fReg.instr = "rrmovq";
					break;
				case 1:
					fReg.instr = "cmovle";
					break;
				case 2:
					fReg.instr = "cmovl";	
					break;
				case 3:
					fReg.instr = "cmove";
					break;
				case 4:
					fReg.instr = "cmovne";
					break;
				case 5:
					fReg.instr = "cmovge";
					break;
				case 6:
					fReg.instr = "cmovg";
					break;
				default:
					// Invalid function code
					printf("Invalid function code %.2x at 0x%016llX\n",icode_ifun,fReg.PC);
					exit(0);
			}			
			setParams_2ByteInstruct();	
			
			break;
		case 3:
			fReg.instr = "irmovq";			
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_10ByteInstruct();		
			
			break;
		case 4: 
			fReg.instr = "rmmovq";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_10ByteInstruct();
		
			break;
		case 5: 
			fReg.instr = "mrmovq";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_10ByteInstruct();

			break;
		case 6: 
			// get instr based on ifun
			switch (fReg.ifun) {
				case 0:
					fReg.instr = "addq";
					break;
				case 1:
					fReg.instr = "subq";
					break;
				case 2:
					fReg.instr = "andq";	
					break;
				case 3:
					fReg.instr = "xorq";
					break;
				case 4:
					fReg.instr = "mulq";
					break;
				case 5:
					fReg.instr = "divq";
					break;
				case 6:
					fReg.instr = "modq";
					break;
				default:
					// Invalid function code
					printf("Invalid function code %.2x at 0x%016llX\n",icode_ifun,fReg.PC);
					exit(0);
			}
			setParams_2ByteInstruct();
			break;

		case 7:
			switch (fReg.ifun) {
				case 0:
					fReg.instr = "jmp";
					break;
				case 1:
					fReg.instr = "jle";
					break;
				case 2:
					fReg.instr = "jl";
					break;
				case 3:
					fReg.instr = "je";
					break;
				case 4:
					fReg.instr = "jne";
					break;
				case 5:
					fReg.instr = "jge";
					break;
				case 6:
					fReg.instr = "jg";
					break;
				default:
					// Invalid function code
					printf("Invalid function code %.2x at 0x%016llX\n",icode_ifun,fReg.PC);
					exit(0);
			}
			setParams_9ByteInstruct();
			break;

		case 8: 
			fReg.instr = "call";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_9ByteInstruct();
			break;
		case 9:
			fReg.instr = "ret";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_1ByteInstruct();
			break;
		case 0xA: 
			fReg.instr = "pushq";
			assertZeroiFunCode(fReg.ifun, icode_ifun);
			setParams_2ByteInstruct();
			break;
		case 0xB: 
			fReg.instr = "popq";
			assertZeroiFunCode(fReg.ifun, icode_ifun);	
			setParams_2ByteInstruct();
			break;
		default:
			// icode not recognized. Print message and exit.
			printf("Invalid opcode %.2x at 0x%016llX", icode_ifun, fReg.PC);
			exit(0);
			break;
	}
	// handling case where insufficient bytes to complete instruction fetch
	if (notEnoughBytes) {
		printf("Memory access error at %.X, required %d bytes, read %d bytes.\n", fReg.PC, bytesToRead, numBytesCanRead);
		exit(0);
	}	
	// for halt instructions, do not print out more than 5 consecutive.
	if (fReg.icode != 0 || numConsecutiveHalts <= 5) {
    printRegS(&fReg);
	}
	// set new fReg.PC
	fReg.PC = fReg.valP;
}


int main(int argc, char **argv) {
  
  int machineCodeFD = -1;       // File descriptor of file with object code
  uint64_t PC = 0;              // The program counter
  // struct fetchRegisters fReg;
  
  // Verify that the command line has an appropriate number
  // of arguments

  if (argc < 2 || argc > 3) {
    printf("Usage: %s InputFilename [startingOffset]\n", argv[0]);
    return ERROR_RETURN;
  }

  // First argument is the file to open, attempt to open it 
  // for reading and verify that the open did occur.
  machineCodeFD = open(argv[1], O_RDONLY);

	if (machineCodeFD < 0) {
    printf("Failed to open: %s\n", argv[1]);
    return ERROR_RETURN;
  }

  // If there is a 2nd argument present, it is an offset so
  // convert it to a value. This offset is the initial value the 
  // program counter is to have. The program will seek to that location
  // in the object file and begin fetching instructions from there.  
  if (3 == argc) {
    // See man page for strtol() as to why
    // we check for errors by examining errno
    errno = 0;
    PC = strtol(argv[2], NULL, 0);
    if (errno != 0) {
      perror("Invalid offset on command line");
      return ERROR_RETURN;
    }
  }

  printf("Opened %s, starting offset 0x%016llX\n", argv[1], PC);

  // added code from here

  fReg.PC = PC;

  FILE *fileStream;
  fileStream = fopen(argv[1], "rb"); // open file in binary read mode
  fseek(fileStream, 0, SEEK_END);
  fileLen = ftell(fileStream); // determine file length
  fseek(fileStream, 0, SEEK_SET);
  
  fileContents = malloc(fileLen * sizeof(char)); // initialize char array of file contents
  fgets(fileContents, fileLen, fileStream);

  while (fReg.PC < fileLen) {
    resetInstructBuffer(); // set instruction buffer to zeros
	
	  fseek(fileStream, fReg.PC, SEEK_SET); // set read offset to fReg.PC
	  int test = fread(instrBuffer, 1, 1, fileStream); // read one byte from fileStream
	
	  icode_ifun = instrBuffer[0]; // set icode_ifun var
	  icode_ifun_arr = decToHex(&icode_ifun); // array containing icode and ifun
	  fReg.icode = icode_ifun_arr[0]; // extract icode
	  fReg.ifun = icode_ifun_arr[1]; // extract ifun
	
	  fseek(fileStream, fReg.PC, SEEK_SET);
	  bytesToRead = getBytesToRead(fReg.icode); // get num bytes to read based on icode. Will terminate if invalid icode
	
	  numBytesCanRead = fread(instrBuffer, 1, bytesToRead, fileStream); // read bytesToRead into instrBuffer
	
	  // if not enough bytes, set exit condition
	  if (numBytesCanRead < bytesToRead) {
		  notEnoughBytes = TRUE;
	  }
	  processInstr();
  }


// given examples 

// #define EXAMPLESON 1
#ifdef  EXAMPLESON

  // The next few lines are examples of various types of output. In the comments is 
  // an instruction, the address it is at and the associated binary code that would
  // be found in the object code file at that address (offset). Your program 
  // will read that binary data and then pull it appart just like the fetch stage. 
  // Once everything has been pulled apart then a call to printRegS is made to 
  // have the output printed. Read the comments in printInternalReg.h for what 
  // the various fields of the structure represent. Note: Do not fill in fields
  // int the structure that aren't used by that instruction. 
  
  
  /************************************************* 
     irmovq $1, %rsi   0x008: 30f60100000000000000
  ***************************************************/

  fReg.PC = 8; fReg.icode = 3; fReg.ifun = 0;
  fReg.regsValid = 1; fReg.rA = 15;  fReg.rB = 6;
  fReg.valCValid = 1; fReg.valC = 1;
  fReg.byte0 = 1;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
  fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
  fReg.valP = 8 + 10;  fReg.instr = "irmovq";

  printRegS(&fReg);


    /************************************************* 
     je target   x034: 733f00000000000000     Note target is a label

     ***************************************************/

  
  fReg.PC = 0x34; fReg.icode = 7; fReg.ifun = 3;
  fReg.regsValid = 0;
  fReg.valCValid = 1; fReg.valC = 0x3f;
  fReg.byte0 = 0x3f;  fReg.byte1 = 0; fReg.byte2 = 0; fReg.byte3 = 0;
  fReg.byte4 = 0;  fReg.byte5 = 0; fReg.byte6 = 0; fReg.byte7 = 0;
  fReg.valP = 0x34 + 9;  fReg.instr = "je";

  printRegS(&fReg);
    /************************************************* 
     nop  x03d: 10    

     ***************************************************/

  fReg.PC = 0x3d; fReg.icode = 1; fReg.ifun = 0;
  fReg.regsValid = 0;
  fReg.valCValid = 0;
  fReg.valP = 0x3d + 1;  fReg.instr = "nop";

  printRegS(&fReg);

    /************************************************* 
     addq %rsi,%rdx  0x03f: 6062 

     ***************************************************/

  fReg.PC = 0x3f; fReg.icode = 6; fReg.ifun = 0;
  fReg.regsValid = 1; fReg.rA = 6; fReg.rB = 2;
  fReg.valCValid = 0;
  fReg.valP = 0x3f + 2;  fReg.instr = "add";

  printRegS(&fReg);
#endif
  // requirement of the assignment
  printf("Normal termination\n");
  exit(0);


  return SUCCESS;

}

