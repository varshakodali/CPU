#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define codesegment 1024      //1Kb memory for bootstrap

//opcodes for instructions
#define LOADI  0x1        
#define STOREI 0x2
#define LOAD   0x3
#define STORE  0x4
#define ARITHMETIC 0x0
#define ADD 0x0
#define SUB 0x1
#define MUL 0x2
#define DIV 0x3 
#define MOD 0x4
          
//Register Ids
#define Ra_ID 0x0
#define Rb_ID 0x1
#define Rc_ID 0x2
#define Rd_ID 0x3
#define Re_ID 0x4
#define Rf_ID 0x5
#define Rg_ID 0x6
#define Rh_ID 0x7



unsigned char memory[1024 * 1024]; //physical memory(1MB)
unsigned int Ra = 0x04, Rb = 5,Rc = 10,Rd = 0,Re = 0,Rf = 0,Rg = 0,Rh = 0; //General purpose registers
unsigned int MAR = 0,MDR = 0,PC = codesegment,IR = 0; //Special purpose registers
unsigned short int flags = 0, overflowFlag;

//functions to convert assembly to binary
unsigned char get_binary_opcode(unsigned char*);
unsigned char get_binary_registerID(unsigned char*);
unsigned char get_binary_arithmetic_opcode(unsigned char*);

void fetch_instruction(); 
void decode_instruction();

//functions to perform required operation
void load_immediate();
void store_immediate();
void load();
void store();
void perform_arithmetic_operation();

//functions to fetch/store data from/to memory
void get_data_from_memory(unsigned char, unsigned int);
void store_data_to_memory(unsigned char, unsigned int);

//functions to fetch/store data from/to registers
void assign_data_to_register(unsigned int, unsigned char);
unsigned int get_register_content(unsigned char);

//ALU functions
//unsigned int add(unsigned int, unsigned int);
//unsigned int sub(unsigned int, unsigned int);
//unsigned int mul(unsigned int, unsigned int);
//unsigned int division(unsigned int, unsigned int);
//unsigned int int_mod(unsigned int, unsigned int);

//-------------------------------------------------------------------------------------------------------


unsigned int add(unsigned int operand1, unsigned int operand2){

unsigned int op1,op2,carry;
op1= operand1;
op2= operand2;

while(op2 !=0){
	carry = op1 & op2;
	op1 = op1 ^ op2;
	op2 = carry << 1;
}
if((op1<operand1) || (op1<operand2)){
	overflowFlag = 1;
        printf("Overflow Flag set to %hu", overflowFlag);
}
else{
	overflowFlag = 0;
}
	return op1;
}




unsigned int division(unsigned int operand1, unsigned int operand2){

	int flag = 1;
	int quotient = 0;
	while (operand2 <= operand1){

	operand2 <<= 1;
	flag <<= 1;
	}
	while(flag > 1){
		operand2 >>= 1;
		flag >>= 1;
		if(operand1 >= operand2){
			operand1 = operand1-operand2;
			quotient |= flag;

		}
	}
	return quotient;
}
unsigned int mul(unsigned int operand1, unsigned int operand2){

	unsigned int reg = 0;
	int flag =0;
	while(operand2 != 0)
	{
		if(operand2 & 1){
			//reg += operand1;
			reg = add(operand1, reg);
			if(overflowFlag) {
				flag = 1; 
				printf("Overflow Flag set to %hu", overflowFlag);
			}
		}
		operand1 <<= 1;
		operand2 >>= 1;
	}
	overflowFlag = flag;
	 return reg;
}

unsigned int substract(unsigned int operand1, unsigned int operand2){
unsigned int carry=0, neg=0;
if(operand1 < operand2)
	neg = 1;

operand2 = ~operand2;
operand2 = operand2 + 1;
operand1 = add(operand1, operand2);

if(neg == 1){
operand1 = ~operand1;
operand1 = operand1 + 1;
}	
	overflowFlag = 0;
	return operand1;
}

unsigned int (*substraction)(unsigned int, unsigned int) = &substract;
unsigned int (*addition)(unsigned int, unsigned int) = &add;

unsigned int int_mod (unsigned int operand1, unsigned int operand2){

	unsigned int sum=0, tempSum=0, remainder=0;
	sum = operand2;
	while(sum < operand1)
	{
		tempSum = sum;
		sum = (*addition)(sum, operand2);
	}
	remainder = (*substraction) (operand1, tempSum);
	overflowFlag=0;
	return remainder;
}
//==================================Function initialization==============================================================

unsigned int (*modulo)(unsigned int, unsigned int) = &int_mod;
unsigned int (*divide)(unsigned int, unsigned int) = &division;
unsigned int (*multiplication)(unsigned int, unsigned int) = &mul;




void print_contents();

int main() {

	unsigned char *instruction[4];
        unsigned char input[40];  //assembly input form user
	unsigned char opcode,reg1,reg2,reg3,operation;
	unsigned int immediate_value,temp;
	
        int i = 0;
	memory[0x16] = 20; //data to load from address 0x16 for load instruction

        printf("==============================================================================================\n");
        printf("Enter one instruction(load/store) at a time(memory address should be in hexadecimal)\n");
        printf("\n");
        printf("Before executing Instruction\n");
        print_contents();
	while(fgets(input, sizeof(input), stdin))
        {
    
		instruction[0] = strtok(input, " ");
		instruction[1] = strtok(NULL, ",");  
		instruction[2] = strtok(NULL, ",");
		instruction[3] = strtok(NULL, "\r\n");
                	       
		if(instruction[0] == NULL || instruction[1] == NULL || instruction[2] == NULL )
		{
			printf("Unrecognized Instruction\n");
			printf("==============================================================================================\n");
			printf("Enter one instruction(load/store) at a time(memory address should be in hexadecimal)\n");
	    	        printf("\n");
		}

         opcode = get_binary_opcode(instruction[0]); //get binary value for opcode
         reg1 = get_binary_registerID(instruction[1]); //get binary value for register ID

         if(instruction[3] == NULL)
         reg2 = 0;
         else
         reg2 = get_binary_registerID(instruction[3]);

	 //Storing instruction into memory
         if(opcode != 0){
          
            immediate_value = strtoul(instruction[2], NULL, 16);
            temp = immediate_value;
            memory[codesegment + 4*i] = (opcode << 4) | (reg1);
	    memory[codesegment + 4*i + 3] = (unsigned char)(temp << 4) | reg2;
	    memory[codesegment + 4*i + 2] = (unsigned char)(temp >> 4);
	    memory[codesegment + 4*i + 1] = (unsigned char)(temp >> 12);
         }

         else {

            operation = get_binary_arithmetic_opcode(instruction[0]);
            reg2 = get_binary_registerID(instruction[2]);
            reg3 = get_binary_registerID(instruction[3]);
            memory[codesegment + 4*i] = (opcode << 4) | (reg1);
	    memory[codesegment + 4*i + 3] = 0x00;
	    memory[codesegment + 4*i + 2] = operation << 4;
	    memory[codesegment + 4*i + 1] = reg2 << 4 | reg3;

         }

         fetch_instruction();
         printf("After executing instruction\n");
	 decode_instruction();
         PC = PC + 4; //incrementing program counter       
         print_contents();
	 i++;

	 printf("==============================================================================================\n");
	 printf("Enter one instruction(load/store) at a time(memory address should be in hexadecimal)\n");
         printf("\n");
        }

         
}

//fetching instruction  from memory into register
void fetch_instruction() {

IR = ((unsigned int)memory[PC] << 24) | ((unsigned int)memory[PC+1] << 16) | ((unsigned int)memory[PC+2] << 8) | (unsigned int)memory[PC+3];

}

//decoding the binary instruction
void decode_instruction() {

	unsigned char opcode = IR >> 28;

	switch(opcode) {

		case LOADI:
			load_immediate(); //load function with direct addressing mode
			break;
		case STOREI:
			store_immediate(); //store function with direct addressing mode
			break;
                case LOAD:
                        load(); //load function with indirect addressing mode
			break;
                case STORE:
                        store(); //store function with indirect addressing mode
			break;
                case ARITHMETIC:
                        perform_arithmetic_operation(); //perform arithmetic operations
                        break;
       		default:
			printf("Invalid opcode\n");
			exit(1);
		
	}

}

unsigned char get_binary_opcode(unsigned char *instruction) {

        if(strcmp("LI",instruction)==0 || strcmp("li",instruction)==0)
	return LOADI;
	else if(strcmp("SI",instruction)==0 || strcmp("si",instruction)==0)
	return STOREI;
        if(strcmp("LW",instruction)==0 || strcmp("lw",instruction)==0)
	return LOAD;
	else if(strcmp("SW",instruction)==0 || strcmp("sw",instruction)==0)
	return STORE;
        else if(strcmp("ADD",instruction)==0 || strcmp("SUB",instruction)==0 || strcmp("DIV",instruction)==0 || strcmp("MUL",instruction)==0 || strcmp("MOD",instruction)==0)
        return ARITHMETIC;
	else {
		printf("Invalid opcode\n");
		exit(1);
	}

}

unsigned char get_binary_registerID(unsigned char *instruction){

	if(strcmp("Ra",instruction)==0 || strcmp("ra",instruction)==0)
	return Ra_ID;
	else if(strcmp("Rb",instruction)==0 || strcmp("rb",instruction)==0)
	return Rb_ID;
	else if(strcmp("Rc",instruction)==0 || strcmp("rc",instruction)==0)
	return Rc_ID;
	else if(strcmp("Rd",instruction)==0 || strcmp("rd",instruction)==0)
	return Rd_ID;
	else if(strcmp("Re",instruction)==0 || strcmp("re",instruction)==0)
	return Re_ID;
	else if(strcmp("Rf",instruction)==0 || strcmp("rf",instruction)==0)
	return Rf_ID;
	else if(strcmp("Rg",instruction)==0 || strcmp("rg",instruction)==0)
	return Rg_ID;
	else if(strcmp("Rh",instruction)==0 || strcmp("rh",instruction)==0)
	return Rh_ID;
	else {
		printf("Invalid Register ID\n");
                return -1;
		exit(1);
	}

}

unsigned char get_binary_arithmetic_opcode(unsigned char *instruction) {

        if(strcmp("ADD",instruction)==0 || strcmp("add",instruction)==0)
	return ADD;
	else if(strcmp("SUB",instruction)==0 || strcmp("sub",instruction)==0)
	return SUB;
        if(strcmp("MUL",instruction)==0 || strcmp("mul",instruction)==0)
	return MUL;
	else if(strcmp("DIV",instruction)==0 || strcmp("div",instruction)==0)
	return DIV;
	else if(strcmp("MOD",instruction)==0 || strcmp("mod",instruction)==0)
	return MOD;
     	else {
		printf("Invalid Arithmetic function\n");
                exit(1);
	}

}

void load_immediate() {

	unsigned int address = (IR >> 4) & 0xFFFFF;
        unsigned char reg_id = (IR >> 24) & 0xF;
        get_data_from_memory(reg_id,address);
	
}

void store_immediate() {
        
        unsigned int address = (IR >> 4) & 0xFFFFF;
        unsigned char reg_id = (IR >> 24) & 0xF;
        store_data_to_memory(reg_id,address);
}

void load() {

	unsigned int value = (IR >> 4) & 0xFFFFF,address;
        unsigned char reg1_id = (IR >> 24) & 0xF,reg2_id = IR & 0xF;
        
	switch(reg2_id) {

	case Ra_ID:
		address = (unsigned int)(Ra + value);
		break;
	case Rb_ID:
		address = (unsigned int)(Rb + value);
		break;
	case Rc_ID:
		address = (unsigned int)(Rc + value);
		break;
	case Rd_ID:
		address = (unsigned int)(Rd + value);
		break;
	case Re_ID:
		address = (unsigned int)(Re + value);
		break;
	case Rf_ID:
		address = (unsigned int)(Rf + value);
		break;
	case Rg_ID:
		address = (unsigned int)(Rg + value);
		break;
	case Rh_ID:
		address = (unsigned int)(Rh + value);
		break;
	default:
		printf("Register ID invalid\n");
		exit(1);
	}
        get_data_from_memory(reg1_id,address);
}


void store() {

	unsigned int value = (IR >> 4) & 0xFFFFF,address;
        unsigned char reg1_id = (IR >> 24) & 0xF,reg2_id = IR & 0xF;
        
	switch(reg2_id) {

	case Ra_ID:
		address = (unsigned int)(Ra + value);
		break;
	case Rb_ID:
		address = (unsigned int)(Rb + value);
		break;
	case Rc_ID:
		address = (unsigned int)(Rc + value);
		break;
	case Rd_ID:
		address = (unsigned int)(Rd + value);
		break;
	case Re_ID:
		address = (unsigned int)(Re + value);
		break;
	case Rf_ID:
		address = (unsigned int)(Rf + value);
		break;
	case Rg_ID:
		address = (unsigned int)(Rg + value);
		break;
	case Rh_ID:
		address = (unsigned int)(Rh + value);
		break;
	default:
		printf("Register ID invalid\n");
		exit(1);
	}
        store_data_to_memory(reg1_id,address);
}

void get_data_from_memory(unsigned char reg_id,unsigned int address) {
        
        MDR = memory[address];
        MAR = address;

        switch(reg_id) {

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

void store_data_to_memory(unsigned char reg_id, unsigned int address) {

        MAR = address;	

	switch(reg_id) {

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

void perform_arithmetic_operation() {

  unsigned int result,operand1,operand2;
  unsigned short int temp = IR;
  operand1 = get_register_content((IR >> 20) & 0xF);
  operand2 = get_register_content((IR >> 16) & 0xF);

  switch(temp >> 12){
	
	case ADD:    
        result = (*addition)(operand1, operand2);
	assign_data_to_register(result, (IR >> 28) & 0xF);
        break;
 	case SUB: 
        result = (*substraction)(operand1, operand2);
	assign_data_to_register(result, (IR >> 28) & 0xF);
        break;
 	case MUL: 
        result = (*multiplication)(operand1, operand2);
	assign_data_to_register(result, (IR >> 28) & 0xF);
        break;
        case DIV: 
        result = (*divide)(operand1, operand2);
	assign_data_to_register(result, (IR >> 28) & 0xF);
        break;
	case MOD: 
        result = (*modulo)(operand1, operand2);
	assign_data_to_register(result, (IR >> 28) & 0xF);
        break;
	default:
        printf("Invalid Arithmetic Operation\n");
        exit(1);

   }
        
}

unsigned int get_register_content(unsigned char registerID) {

 	switch(registerID) {

	case Ra_ID:
		return Ra;
	case Rb_ID:
		return Rb;
	case Rc_ID:
		return Rc;
	case Rd_ID:
		return Rd;
	case Re_ID:
		return Re;
	case Rf_ID:
		return Rf;
	case Rg_ID:
		return Rg;
	case Rh_ID:
		return Rh;
	default:
		printf("Register ID invalid\n");
		exit(1);
	}
}

void assign_data_to_register(unsigned int data, unsigned char reg_id){

	switch(reg_id) {

	case Ra_ID:
		Ra = data;
                printf("Ra = %d\n",Ra);
		break;
	case Rb_ID:
		Rb = data;
                printf("Rb = %d\n",Rb);
		break;
	case Rc_ID:
		Rc = data;
                printf("Rc = %d\n",Rc);
		break;
	case Rd_ID:
		Rd = data;
		printf("Rd = %d\n",Rd);
		break;
	case Re_ID:
		Re = data;
		printf("Re = %d\n",Re);
		break;
	case Rf_ID:
		Rf = data;
		printf("Rf = %d\n",Rf);
		break;
	case Rg_ID:
		Rg = data;
		printf("Rg = %d\n",Rg);
		break;
	case Rh_ID:
		Rh = data;
		printf("Rh = %d\n",Rh);
		break;
	default:
		printf("Register Id invalid\n");
		exit(1);
	}

}

void print_contents() {
	
	printf("Program counter = 0x%x\n", PC);
	printf("MDR = %d\n", MDR);
	printf("MAR = 0x%x\n", MAR);

}













	


