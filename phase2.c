#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

#define OPCODE(instr)  (instr >> 26 & 0x0000003f) 
#define DSTREG(instr)  (instr >> 21 & 0x0000001f) //first reg
#define SRCREG(instr)  (instr >> 16 & 0x0000001f) //second
#define SRCREG2(instr)  (instr >> 11 & 0x0000001f)//third
#define SHAMT(instr)  (instr >> 6 & 0x0000003f)
#define FUNCT(instr)  (instr & 0x0000003f)
// 
#define IMMVAL(instr)  (instr  & 0x0000ffff) 
//
#define ADDRESS(instr) (instr & 0x03ffffff)

int32_t memory[65536];
int32_t regs[32];
int32_t pc, ir;

typedef union{  // reversed the order of struct fields for mobaxterm
	unsigned int inst: 32;
	struct {
		unsigned int funct:6;
		unsigned int shamt:5; // shifter
		unsigned int rd:5;
		unsigned int rt:5;
		unsigned int rs:5;
		unsigned int opcode:6;
	} r;   // register instruction

	struct {
		int imm:16;
		unsigned int rt:5;
		unsigned int rs:5;
		unsigned int opcode:6;
	} i;   // immediate instruction

	struct {
		int adr:26;
		unsigned int opcode:6;
	} j;   // jump instruction
} Instruction;


// take in a pointer (string)
// compare string to each of 32 registers
// return the register num
int regCmp(const char *regPtr){ // 32 reg possibilities
	int myReg;
  
	if(strcmp("$zero",regPtr) == 0){
		myReg = 0;
	}else if(strcmp("$at",regPtr) == 0){
		myReg = 1;
	}else if(strcmp("$v0",regPtr) == 0){
		myReg = 2;
	}else if(strcmp("$v1",regPtr) == 0){
		myReg = 3;
	}else if(strcmp("$a0",regPtr) == 0){
		myReg = 4;
	}else if(strcmp("$a1",regPtr) == 0){
		myReg = 5;
	}else if(strcmp("$a2",regPtr) == 0){
		myReg = 6;
	}else if(strcmp("$a3",regPtr) == 0){
		myReg = 7;
	}else if(strcmp("$t0",regPtr) == 0){
		myReg = 8;
	}else if(strcmp("$t1",regPtr) == 0){
		myReg = 9;
	}else if(strcmp("$t2",regPtr) == 0){
		myReg = 10;
	}else if(strcmp("$t3",regPtr) == 0){
		myReg = 11;
	}else if(strcmp("$t4",regPtr) == 0){
		myReg = 12;
	}else if(strcmp("$t5",regPtr) == 0){
		myReg = 13;
	}else if(strcmp("$t6",regPtr) == 0){
		myReg = 14;
	}else if(strcmp("$t7",regPtr) == 0){
		myReg = 15;
	}else if(strcmp("$s0",regPtr) == 0){
		myReg = 16;
	}else if(strcmp("$s1",regPtr) == 0){
		myReg = 17;
	}else if(strcmp("$s2",regPtr) == 0){
		myReg = 18;
	}else if(strcmp("$s3",regPtr) == 0){
		myReg = 19;
	}else if(strcmp("$s4",regPtr) == 0){
		myReg = 20;
	}else if(strcmp("$s5",regPtr) == 0){
		myReg = 21;
	}else if(strcmp("$s6",regPtr) == 0){
		myReg = 22;
	}else if(strcmp("$s7",regPtr) == 0){
		myReg = 23;
	}else if(strcmp("$t8",regPtr) == 0){
		myReg = 24;
	}else if(strcmp("$t9",regPtr) == 0){
		myReg = 25;
	}else if(strcmp("$k0",regPtr) == 0){
		myReg = 26;
	}else if(strcmp("$k1",regPtr) == 0){
		myReg = 27;
	}else if(strcmp("$gp",regPtr) == 0){
		myReg = 28;
	}else if(strcmp("$sp",regPtr) == 0){
		myReg = 29;
	}else if(strcmp("$fp",regPtr) == 0){
		myReg = 30;
	}else if(strcmp("$ra",regPtr) == 0){
		myReg = 31;
	}else{
		printf("what reg is this: %s\n", regPtr);
	}
	return myReg;
}

//converts type i instruction imm fields
//from string to actual int
int32_t numCnv(const char *regPtr){
  int32_t num = 0;
  int i = 0;
  while(*regPtr != '\0'){
    num *= 10;
    num += *regPtr - '0';
    regPtr++; 
  }
  //printf("%d\n", num);
return num;
}



Instruction parseLine(char *theLine){ 
	char *asmInst[4];
	int i = 0;
	Instruction inst;
	char CR = 13; 
	
	//parses the line into its tokens and stores pointers to each of them
	while(*theLine != '\n' && *theLine != '\0'){ 
		while(*theLine == ' ' || *theLine == '('){ //eliminates any spaces before instruction token
			theLine++;
		}
		asmInst[i] = theLine; // marks the beginning of token
		while((*theLine != ',') && (*theLine != ' ') && 
			  (*theLine != '\0') && (*theLine != CR) &&
			  (*theLine != ')')){  // finds the end of token
			theLine++;
		}
		*theLine = '\0'; // makes sure "end of line" char (\0) are being placed at the end
		theLine++;
    i++;          // sets up next slot in array to place a pointer.
	}

// once an array is filled w/ tokens, we can start building instructions
// based upon the opcode		
	if(strcmp("add",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 20;
	}else if(strcmp("addi",asmInst[0]) == 0){
		inst.i.opcode = 8;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[2]);
		inst.i.imm = numCnv(asmInst[3]);
	}else if(strcmp("addu",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 21;
	}else if(strcmp("addiu",asmInst[0]) == 0){
		inst.i.opcode = 9;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[2]);
		inst.i.imm = numCnv(asmInst[3]);
	}else if(strcmp("sub",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 22;
	}else if(strcmp("subu",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 23;
	}else if(strcmp("and",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 24;
	}else if(strcmp("andi",asmInst[0]) == 0){
		inst.i.opcode = 12;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[2]);
		inst.i.imm = numCnv(asmInst[3]);
	}else if(strcmp("nor",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 27;
	}else if(strcmp("or",asmInst[0]) == 0){
		inst.r.opcode = 0;
		inst.r.rs = regCmp(asmInst[1]);
		inst.r.rt = regCmp(asmInst[2]);
		inst.r.rd = regCmp(asmInst[3]);
		inst.r.shamt = 0;
		inst.r.funct = 25;
	}else if(strcmp("ori",asmInst[0]) == 0){
		inst.i.opcode = 13;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[2]);
		inst.i.imm = numCnv(asmInst[3]);
	}else if(strcmp("lw",asmInst[0]) == 0){
		inst.i.opcode = 35;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[3]); // fixed for mips format
		inst.i.imm = numCnv(asmInst[2]);
	}else if(strcmp("sw",asmInst[0]) == 0){
		inst.i.opcode = 43;
		inst.i.rs = regCmp(asmInst[1]);
		inst.i.rt = regCmp(asmInst[3]); // fixed for mips format
		inst.i.imm = numCnv(asmInst[2]);
	}else{
		printf("there's no opcode added for [%s]", asmInst[0]);
	}
	return inst;
}

void execute(int theInstNum){
	pc = 0;
	while(pc < theInstNum){
		//printf("executing instruction %d\n", pc);
		ir = memory[pc]; //fetched the instruction
		pc++; 
    
		uint32_t opcode = OPCODE(ir);
		int32_t dest_reg, bse_reg, src_reg, src_reg2, imm_val, function, shamt, address; // pcoffset9, pcoffset6, branch, imm_bit
		switch(opcode) {
		
			case 0: // opcode is an r-instruction and needs the function code
				function = FUNCT(ir);
				switch(function) {
					case 20:  // add instruction
						dest_reg = DSTREG(ir);
						//printf("dest reg = %hu\n", dest_reg);
						src_reg = SRCREG(ir);
						//printf("src reg = %hu\n", src_reg);  
						src_reg2 = SRCREG2(ir);
						//printf("src reg 2 = %hd\n", src_reg2);
						regs[dest_reg] = regs[src_reg] + regs[src_reg2];
						break;
					
					case 21:  // addu instruction
						break;
					
					case 22:  // sub instruction
						dest_reg = DSTREG(ir);
						//printf("dest reg = %hu\n", dest_reg);
						src_reg = SRCREG(ir);
						//printf("src reg = %hu\n", src_reg);  
						src_reg2 = SRCREG2(ir);
						//printf("src reg 2 = %hd\n", src_reg2);
						regs[dest_reg] = regs[src_reg] - regs[src_reg2];
						break;
					
					case 23:  // subu instruction
						break;
					
					case 24:  // and instruction
						dest_reg = DSTREG(ir);
						//printf("dest reg = %hu\n", dest_reg);
						src_reg = SRCREG(ir);
						//printf("src reg = %hu\n", src_reg);  
						src_reg2 = SRCREG2(ir);
						//printf("src reg 2 = %hd\n", src_reg2);
						regs[dest_reg] = regs[src_reg] & regs[src_reg2];
						break;
					
					case 25:  // or instruction
						dest_reg = DSTREG(ir);
						//printf("dest reg = %hu\n", dest_reg);
						src_reg = SRCREG(ir);
						//printf("src reg = %hu\n", src_reg);  
						src_reg2 = SRCREG2(ir);
						//printf("src reg 2 = %hd\n", src_reg2);
						regs[dest_reg] = regs[src_reg] | regs[src_reg2];
						break;
					
					case 27:  // nor instruction
						break;
					
					default:
						printf("\n\nDEFAULT CASE: does not support function: %d form %d\n", function, opcode);
						break;
				}
				break;
        
			case 8:  // addi instruction
				//printf("\n\nAdd Instruction\n");
				dest_reg = DSTREG(ir);
				//printf("dest reg = %hu\n", dest_reg);
				src_reg = SRCREG(ir);
				//printf("src reg = %hu\n", src_reg);
				imm_val = IMMVAL(ir);
				//printf("imm val = %hd\n", imm_val);
				regs[dest_reg] = regs[src_reg] + imm_val;
				break;
			
			case 9:  // addui instruction
				break;
			
			case 12:  // andi instruction
				//printf("\n\nAnd Instruction\n");
                dest_reg = DSTREG(ir);
                //printf("dest reg = %hu\n", dest_reg);
                src_reg = SRCREG(ir);
				imm_val = IMMVAL(ir);
				//printf("imm val = %hd\n", imm_val);
			    regs[dest_reg] = regs[src_reg] & imm_val; 
				break;
			
			case 13:  // ori instruction
				//printf("\n\nOr Instruction\n");
                dest_reg = DSTREG(ir);
                //printf("dest reg = %hu\n", dest_reg);
                src_reg = SRCREG(ir);
				imm_val = IMMVAL(ir);
				//printf("imm val = %hd\n", imm_val);
			    regs[dest_reg] = regs[src_reg] | imm_val; 
				break;
			
			case 35:  // lw instruction
				//printf("\n\nLoad Instruction\n");
				dest_reg = DSTREG(ir);
				src_reg = SRCREG(ir);
				imm_val = IMMVAL(ir);
				regs[dest_reg] = memory[regs[src_reg]+imm_val];
				break;
			
			case 43:  //sw instruction
					//printf("\n\nstore Instruction\n");
					src_reg = DSTREG(ir); // DSTREG is actually pulling Src Regs when storing
					dest_reg = SRCREG(ir);
					imm_val = IMMVAL(ir);                                     
					memory[regs[dest_reg]+imm_val] = regs[src_reg];
					//printf("src = %d, dest = %d, imm = %d\n",src_reg,dest_reg,imm_val);
					//printf("memloc = %d\n", regs[dest_reg]+imm_val);
					break;
			default :
				printf("\n\nDEFAULT CASE: does not support opcode: %d\n", opcode);
				break;			
        
        } // switch ends        
    }//end of while
	return;
}


int main(int argc, char *argv[]) {
  
	FILE *inFile;
	char *line;
	line = (char *) malloc(32);
	Instruction inst;
	uint32_t inst32b;
	int i;
	uint32_t bit;
	int memCtr;
  
  
	if(argc > 1){ // read in file
		printf("invalid num of args\n");
	} else { // for the sake of phase 1
		inFile = fopen("test1.asm", "rb"); 
 	}
	memCtr = 0;
	while(fgets(line,32,inFile) != NULL){ // we should think about using bits if longer than 32 char
		//printf("%s", line); // the string already has '\n' char
		inst = parseLine(line);
		memory[memCtr] = inst.inst;
		inst32b = inst.inst;
		printf("Opcode = %d\n", inst32b >> 26);
		i = 31;
		while(i >= 0){
			bit = inst32b >> i;
			bit = bit & 1;
			printf("%d",bit);
			i--;
		}
		printf("\n\n");
		memCtr++;
		}
		free(line); // free up memory from malloc
		printf("number of intructions = %d\n", memCtr);
		execute(memCtr); // memory and regs are global
		//printf("finished executing program\n");
		i = 0;
		while(i < 32){
			printf("reg %d = %d\n", i, regs[i]);
			i++;
		}
	printf("sw at mem location 1030 = %d\n", memory[1030]);
	printf("test complete\n");
	return 0;
}