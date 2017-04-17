/*Instruction format for load and store: 4bits  -  opcode, 
                                         4bits  -  source/destination register ID, 
                                         24bits -  memory address
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define codesegment 10*1024 // 10Kb memory for bootstrap
#define LOAD 0x1            // opcode for Load instruction
#define STORE 0x2           // opcode for Store instruction

#define Ra_ID 0x0
#define Rb_ID 0x1
#define Rc_ID 0x2
#define Rd_ID 0x3
#define Re_ID 0x4
#define Rf_ID 0x5
#define Rg_ID 0x6
#define Rh_ID 0x7

unsigned char memory[16*1024*1024]; //physical memory(16MB)
unsigned int Ra = 0x04, Rb = 0,Rc = 0,Rd = 0,Re = 0,Rf = 0,Rg = 0,Rh = 0; // General purpose registers
unsigned int MAR = 0,MDR = 0,PC = codesegment,IR = 0; // special purpose registers
unsigned short int flags = 0;

void fetch_instruction(); 
void decode_instruction();
void load(unsigned char, unsigned int);
void store(unsigned char, unsigned int);
void print_contents();

int main() {
	unsigned char *instruction[3];
        unsigned char input[40];
	unsigned char opcode,destination_register;
	unsigned int memory_address;
        int i = 0;
	memory[0x16] = 20; // data to load from address 0x16 for load instruction

        print_contents();
        printf("Enter one instruction(load/store) at a time(memory address should be in hexadecimal)\n");
 	
	while(fgets(input, sizeof(input), stdin)) {

	instruction[0] = strtok(input, " ");
	instruction[1] = strtok(NULL, ",");  
	instruction[2] = strtok(NULL, "\r\n");
	memory_address = strtoul(instruction[2], NULL, 16);

	if(strcmp("LW",instruction[0])==0 || strcmp("lw",instruction[0])==0)
	opcode = LOAD;
	else if(strcmp("ST",instruction[0])==0 || strcmp("st",instruction[0])==0)
	opcode = STORE;
	else {
		printf("Invalid opcode\n");
		exit(1);
	}

	if(strcmp("Ra",instruction[1])==0 || strcmp("ra",instruction[1])==0)
	destination_register = Ra_ID;
	else if(strcmp("Rb",instruction[1])==0 || strcmp("rb",instruction[1])==0)
	destination_register = Rb_ID;
	else if(strcmp("Rc",instruction[1])==0 || strcmp("rc",instruction[1])==0)
	destination_register = Rc_ID;
	else if(strcmp("Rd",instruction[1])==0 || strcmp("rd",instruction[1])==0)
	destination_register = Rd_ID;
	else if(strcmp("Re",instruction[1])==0 || strcmp("re",instruction[1])==0)
	destination_register = Re_ID;
	else if(strcmp("Rf",instruction[1])==0 || strcmp("rf",instruction[1])==0)
	destination_register = Rf_ID;
	else if(strcmp("Rg",instruction[1])==0 || strcmp("rg",instruction[1])==0)
	destination_register = Rg_ID;
	else if(strcmp("Rh",instruction[1])==0 || strcmp("rh",instruction[1])==0)
	destination_register = Rh_ID;
	else {
		printf("Invalid Register ID\n");
		exit(1);
	}

	memory[codesegment + 4*i] = (opcode << 4) | (destination_register);
	memory[codesegment + 4*i + 3] = (unsigned char)(memory_address);
	memory[codesegment + 4*i + 2] = (unsigned char)(memory_address >> 8);
	memory[codesegment + 4*i + 1] = (unsigned char)(memory_address >> 8);
	fetch_instruction();
	decode_instruction();
        PC = PC + 4;
        print_contents();
	i++;
	}
	return 0;

}

void fetch_instruction() {

//fetching instruction into register from memory
IR = ((unsigned int)memory[PC] << 24) | ((unsigned int)memory[PC+1] << 16) | ((unsigned int)memory[PC+2] << 8) | (unsigned int)memory[PC+3];

}

void decode_instruction() {

	unsigned int  temp  = IR;
	unsigned char temp1 = temp >> 24,opcode = temp1 >> 4,register_name = (temp << 4) >> 28;
	switch(opcode) {
		case LOAD:
			load(register_name,(temp << 8) >> 8);
			break;
		case STORE:
			store(register_name,(temp << 8) >> 8);
			break;
		default:
			printf("Invalid opcode\n");
			exit(1);
		
	}
}
void load(unsigned char reg,unsigned int address) {

	MDR = memory[address];
        MAR = address;
	switch(reg) {
	case Ra_ID:
		Ra = MDR;
		printf("Ra = %d\n",Ra);
		break;
	case Rb_ID:
		Rb = MDR;
		printf("Rb = %d\n",Rb);
		break;
	case Rc_ID:
		Rc = MDR;
		printf("Rc = %d\n",Rc);
		break;
	case Rd_ID:
		Rd = MDR;
		printf("Rd = %d\n",Rd);
		break;
	case Re_ID:
		Re = MDR;
		printf("Re = %d\n",Re);
		break;
	case Rf_ID:
		Rf = MDR;
		printf("Rf = %d\n",Rf);
		break;
	case Rg_ID:
		Rg = MDR;
		printf("Rg = %d\n",Rg);
		break;
	case Rh_ID:
		Rh = MDR;
		printf("Rh = %d\n",Rh);
		break;
	default:
		printf("Register ID invalid\n");
		exit(1);
	}
}
void store(unsigned char reg,unsigned int address) {

        MAR = address;	
	switch(reg) {
	case Ra_ID:
		MDR = Ra;
		break;
	case Rb_ID:
		MDR = Rb;
		break;
	case Rc_ID:
		MDR = Rc;
		break;
	case Rd_ID:
		MDR = Rd;
		break;
	case Re_ID:
		MDR = Re;
		break;
	case Rf_ID:
		MDR = Rf;
		break;
	case Rg_ID:
		MDR = Rg;
		break;
	case Rh_ID:
		MDR = Rh;
		break;
	default:
		printf("Register Id invalid\n");
		break;
	}

	memory[address] = (unsigned char) MDR;
	printf("Data at destination address 0x%x = %d\n",address,memory[address]);

}
void print_contents() {
	
	printf("Program counter = 0x%x\n", PC);
	printf("MDR = %d\n", MDR);
	printf("MAR = 0x%x\n", MAR);

}


