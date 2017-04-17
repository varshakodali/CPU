#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define datasegment  0x100  // 100 bytes memory for bootstrap
#define codesegment  0x150  // 50 bytes for datamemory
#define stacksegment 0xffff

//opcodes for instructions
#define ARITHMETIC 0x0
#define LOADI      0x1        
#define STOREI 	   0x2
#define LOAD       0x3
#define STORE      0x4
#define MOVE       0x5
#define JUMP       0x6
#define LABEL      0x7
#define LEA        0x8
#define PUSH       0x9

//Arithmetic function opcodes
#define ADD 0x0
#define SUB 0x1
#define MUL 0x2
#define DIV 0x3 
#define MOD 0x4

//JUMP Sub function opcodes
#define JE  0x01 
#define JG  0x02
#define JMP 0x03
#define JL  0x04
#define JLE 0x05
#define JGE 0x06

          
//Register Ids
#define Ra_ID 0x0
#define Rb_ID 0x1
#define Rc_ID 0x2
#define Rd_ID 0x3
#define Re_ID 0x4
#define Rf_ID 0x5
#define Rg_ID 0x6
#define Rh_ID 0x7
#define Rz_ID 0x8
#define Ru_ID 0x9
#define V0_ID 10

//Labels
#define L1 0x1
#define L2 0x2
#define L3 0x3
#define L4 0x4
#define L5 0x5
#define L6 0x6

typedef struct Label {

	unsigned char label_Name;
	int nextPC;
} labelKeyVal;

labelKeyVal labelAddr[6];

unsigned char memory[1024 * 1024]; //physical memory(1MB)
unsigned int Ra = 0, Rb = 0,Rc = 0,Rd = 0,Re = 0,Rf = 0,Rg = 0,Rh = 0,Rz = 0,Ru = 1,V0 = 0; //General purpose registers
unsigned int MAR = 0,MDR = 0,PC = codesegment,IR = 0,SP = stacksegment; //Special purpose registers
unsigned short int overflowFlag,CF;
int i = 0; //num of instrucions
int l = 0; //jump labels

//functions to convert assembly to binary
unsigned char get_binary_opcode(unsigned char*);
unsigned char get_binary_registerID(unsigned char*);
unsigned char get_binary_arithmetic_opcode(unsigned char*);
unsigned char get_binary_labelID(unsigned char *);
unsigned char get_binary_jump_opcode(unsigned char *);

void fetch_instruction(); 
void decode_instruction();

//functions to perform required operation
void load_immediate();
void store_immediate();
void load();
void store();
void perform_arithmetic_operation();
void move();
void perform_jump_operation();
void jump(unsigned char);
void load_effective_address();
void push();
void goto_next_instruction();

//functions to fetch/store data from/to memory
void get_data_from_memory(unsigned char, unsigned int);
void store_data_to_memory(unsigned char, unsigned int);

//functions to fetch/store data from/to registers
void assign_data_to_register(unsigned int, unsigned char);
unsigned int get_register_content(unsigned char);


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
        printf("Overflow Flag set to %hu\n", overflowFlag);
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

		unsigned char *instruction[6];
		unsigned char opcode,reg1,reg2,reg3,operation,label_ID,jump_opcode;
		unsigned char input[50]; 
		int file_num;
		unsigned int immediate_value,temp;
    		FILE *fp;
        	memory[0x16] = 0x20; //data to load from address 0x16 for load instruction
		//Array to perform Binary search
		memory[datasegment]     = 0x11;
		memory[datasegment + 1] = 0x12;
		memory[datasegment + 2] = 0x13;
		memory[datasegment + 3] = 0x14;
		memory[datasegment + 4] = 0x15;
		memory[datasegment + 5] = 0x16;

    		printf("================================================\n");
    		printf("Files to be executed\n");
		printf("1. BinarySearch.txt\n");
		printf("2. ForLoop.txt\n");
		printf("3. DoWhile.txt\n");
		printf("4. WhileLoop.txt\n");
		printf("5. LEA.txt\n");
		printf("Enter the file number to be executed\n");
		scanf("%d",&file_num);
    		//print_contents();

		switch(file_num) {
			
			case 1:
			fp = fopen("BinarySearch.txt", "r");
			printf("Given Array Elements:\n");
			printf("Array[0]: 0x%x\n",memory[datasegment]);
        		printf("Array[1]: 0x%x\n",memory[datasegment + 1]);
			printf("Array[2]: 0x%x\n",memory[datasegment + 2]);
			printf("Array[3]: 0x%x\n",memory[datasegment + 3]);
			printf("Array[4]: 0x%x\n",memory[datasegment + 4]);
			printf("Array[5]: 0x%x\n",memory[datasegment + 5]);
			printf("Element to be searched in the given array: 0x13\n");
			break;
			case 2:
			printf("For Loop is executed\n");
        		fp = fopen("ForLoop.txt", "r");
			break;
			case 3:
			printf("Do-While loop is executed 5 times and counter value is pushed to stack\n");
        		fp = fopen("DoWhile.txt", "r");
			break;
			case 4:
			printf("While loop is executed 5 times and counter value is pushed to stack\n");
        		fp = fopen("WhileLoop.txt", "r");
			break;
			case 5:
			printf("LEA Instruction is executed\n");
			fp = fopen("LEA.txt", "r");
			break;
			default:
			printf("Error: Invalid File number\n");
			exit(1);
		}

        	if (fp == NULL)
        	exit(EXIT_FAILURE);
	
		while(fgets(input, sizeof(input), fp) != NULL)
        	{   
			instruction[0] = strtok(input, " ");
			opcode = get_binary_opcode(instruction[0]); //get binary value for opcode

			//Storing arithmetic instructions into memory
			if(opcode == 0)
			{
				instruction[1] = strtok(NULL, ",");  
				instruction[2] = strtok(NULL, ",");
				instruction[3] = strtok(NULL, "\r\n");
				reg1 = get_binary_registerID(instruction[1]);
				operation = get_binary_arithmetic_opcode(instruction[0]);
			    	reg2 = get_binary_registerID(instruction[2]);
			    	reg3 = get_binary_registerID(instruction[3]);
			    	memory[codesegment + 4*i] = (opcode << 4) | (reg1);
			    	memory[codesegment + 4*i + 1] = reg2 << 4 | reg3;
		 	    	memory[codesegment + 4*i + 2] = operation << 4;
			    	memory[codesegment + 4*i + 3] = 0x00;
			}
            		//Storing load,store instructions into memory
		 	else if (opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4) 
		 	{					
				instruction[1] = strtok(NULL, ",");  
				instruction[2] = strtok(NULL, ",");
				instruction[3] = strtok(NULL, "\r\n");    			
				reg1 = get_binary_registerID(instruction[1]); //get binary value for register ID                      
				if(instruction[3] == NULL)
				reg2 = 0;
				else
				reg2 = get_binary_registerID(instruction[3]); 
				immediate_value = strtoul(instruction[2], NULL, 16);
				memory[codesegment + 4*i] = (opcode << 4) | (reg1);
				memory[codesegment + 4*i + 1] = (unsigned char)(immediate_value >> 12);
				memory[codesegment + 4*i + 2] = (unsigned char)(immediate_value >> 4);
				memory[codesegment + 4*i + 3] = (unsigned char)(immediate_value << 4) | reg2;
		 	}
            		//Storing move instruction into memory
            		else if (opcode == 5) 
            		{ 
	            		instruction[1] = strtok(NULL, ",");  
				instruction[2] = strtok(NULL, ",");
				instruction[3] = strtok(NULL, "\r\n");
				reg1 = get_binary_registerID(instruction[1]);
				reg2 = get_binary_registerID(strtok(instruction[2],"\r\n"));
				memory[codesegment + 4*i] = (opcode << 4) | (reg1);
			    	memory[codesegment + 4*i + 1] = reg2 << 4 | 0x0;
				memory[codesegment + 4*i + 2] = 0x00;
				memory[codesegment + 4*i + 3] = 0x00;
		 	}
		 	//Storing Jump instruction into memory
            		else if (opcode == 6) 
            		{	            		
				//Check if ins[1] is there in array of labels
	            		jump_opcode=get_binary_jump_opcode(instruction[0]);
	            		if(jump_opcode == 3)     		
	            		{
	            			instruction[1] = strtok(NULL, "\r\n ");
	            			label_ID = get_binary_labelID(instruction[1]);
					memory[codesegment + 4*i] = opcode<<4;
					memory[codesegment + 4*i + 1] = label_ID;
					memory[codesegment + 4*i + 2] = 0x00;
					memory[codesegment + 4*i + 3] = jump_opcode;
		    		}  
				else
				{
					instruction[1] = strtok(NULL, ",");  
					instruction[2] = strtok(NULL, ",");
					instruction[3] = strtok(NULL, "\r\n ");
					reg1 = get_binary_registerID(instruction[1]);
					reg2 = get_binary_registerID(instruction[2]);
			    		label_ID = get_binary_labelID(instruction[3]);
			       		memory[codesegment + 4*i] = (opcode << 4) | (reg1);
					memory[codesegment + 4*i + 1] = reg2;
				    	memory[codesegment + 4*i + 2] = label_ID;
				    	memory[codesegment + 4*i + 3] = jump_opcode;
				}	
   			}
		 	else if (opcode == 7) 
		 	{
			 	//Create a struct store PC+4 as nextPC and instruction[1] as label_Name
			 	//Push the struct to arry of labels

			 	instruction[1] = strtok(NULL, "\r\n");  
			 	label_ID = get_binary_labelID(instruction[1]);
			 	memory[codesegment + 4*i] = opcode<<4;
			 	memory[codesegment + 4*i + 1] = label_ID;
			 	memory[codesegment + 4*i + 2] = 0x00;
				memory[codesegment + 4*i + 3] = 0x00;
				labelAddr[l].label_Name=label_ID;
				labelAddr[l].nextPC = codesegment + 4*i + 4;
				l++;
		 	}
		 	else if (opcode == 8) 
		 	{ 
		 		// LEA Ra,(Rb,Rc,2) 
				instruction[1] = strtok(NULL, ",");  
				instruction[2] = strtok(NULL, "(,");
				instruction[3] = strtok(NULL, ",");
				instruction[4] = strtok(NULL, ",)\r\n");
				reg1 = get_binary_registerID(instruction[1]);
			    	reg2 = get_binary_registerID(instruction[2]);
			    	reg3 = get_binary_registerID(instruction[3]);
				immediate_value = strtoul(instruction[4], NULL, 16);
			    	memory[codesegment + 4*i] = (opcode << 4) | (reg1);
			    	memory[codesegment + 4*i + 1] = reg2 << 4 | reg3;
		 	    	memory[codesegment + 4*i + 2] = immediate_value << 4;
			    	memory[codesegment + 4*i + 3] = 0x00;
		 	}
			else if (opcode == 9) { //push V0

				instruction[1] = strtok(NULL, " \r\n");
				reg1 = get_binary_registerID(instruction[1]);
				memory[codesegment + 4*i] = opcode<<4 | reg1;
				memory[codesegment + 4*i + 1] = 0x00;
				memory[codesegment + 4*i + 2] = 0x00;
				memory[codesegment + 4*i + 3] = 0x00;
                        }
		 	i++;
		 }
		fclose(fp);
		printf("Before Execution\n");
		print_contents();
		goto_next_instruction();	
	  
}

void goto_next_instruction() {
                    
		if(PC < codesegment + 4*i) {

			fetch_instruction();
			//printf("After executing instruction\n");
			decode_instruction();
			//print_contents();
		}
		else {
		exit(1);
		}
}


//fetching instruction  from memory into register
void fetch_instruction() {

IR = ((unsigned int)memory[PC] << 24) | ((unsigned int)memory[PC+1] << 16) | ((unsigned int)memory[PC+2] << 8) | (unsigned int)memory[PC+3];

}

//decoding the binary instruction
void decode_instruction() {

	unsigned char opcode = IR >> 28;
	//printf("Decoded opcode:%u\n",opcode);
	switch(opcode) 
	{
		case LOADI:
			load_immediate(); //load function with direct addressing mode
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
			break;
		case STOREI:
			store_immediate(); //store function with direct addressing mode
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
			break;
        	case LOAD:
            		load(); //load function with indirect addressing mode
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
		        goto_next_instruction();
			break;
        	case STORE:
            		store(); //store function with indirect addressing mode
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
			break;
        	case ARITHMETIC:
            		perform_arithmetic_operation(); //perform arithmetic operations
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
            		break;
		case MOVE:
			move();
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
			break;
		case JUMP:
	        	perform_jump_operation();
			printf("After Execution\n");
			print_contents();
			goto_next_instruction();
			break;
		case LEA:
			load_effective_address();
			printf("After Execution\n");
			print_contents();
			PC = PC + 4;
			goto_next_instruction();
			break;
		case LABEL:
			PC = PC + 4;
			goto_next_instruction();
			break;
		case PUSH:
			push();
			break;
       		default:
			printf("Invalid opcode\n");
			exit(1);	
	}

}

void push() {

unsigned int data = get_register_content((IR >> 24) & 0x0000000F);
SP = SP - 4;
printf("After Execution\n");
print_contents();
printf("Top of the Stack: 0x%x\n",data);


}


void load_effective_address() 
{
  unsigned int operand1,operand2,immediate_value,result;
  operand1 = get_register_content((IR >> 20) & 0x0000000F);
  operand2 = get_register_content((IR >> 16) & 0x0000000F);
  result = operand1 + operand2 * ((IR >> 12) & 0x0000000F);
  assign_data_to_register(result, (IR >> 24) & 0x0000000F);
}

void perform_jump_operation() 
{
     unsigned char jump_opn = (IR) & 0x000000FF;
     unsigned char label;
     unsigned int r1,r2;
     if(jump_opn == 1)
     {
     	r1 = get_register_content((IR >> 24) & 0x0000000F);
     	r2 = get_register_content((IR >> 16) & 0x0000000F);
     	if(r1==r2)
     	{	
     		label = (IR >> 8) & 0x000000FF;
     		jump(label);
     	}
	else {
	PC = PC +4;
	}

     }
     else if(jump_opn == 2)
     {
     	r1 = get_register_content((IR >> 24) & 0x0000000F);
     	r2 = get_register_content((IR >> 16) & 0x0000000F);
     	if(r1>r2)
     	{	
     		label = (IR >> 8) & 0x000000FF;
     		jump(label);
     	}
	else {
	PC = PC +4;
	}
     }
     else if(jump_opn == 3)
     {
     	label = (IR >> 16) & 0x000000FF;
      	jump(label);
     }
     else if(jump_opn == 4)
     {
     	r1 = get_register_content((IR >> 24) & 0x0000000F);
     	r2 = get_register_content((IR >> 16) & 0x0000000F);
     	if(r1<r2)
     	{	
     		label = (IR >> 8) & 0x000000FF;
     		jump(label);
     	}
	else {
	PC = PC +4;
	}
     }
     else if(jump_opn == 5)
     {
     	r1 = get_register_content((IR >> 24) & 0x0000000F);
     	r2 = get_register_content((IR >> 16) & 0x0000000F);
     	if(r1<=r2)
     	{	
     		label = (IR >> 8) & 0x000000FF;
     		jump(label);
     	}
	else {
	PC = PC +4;
	}
     }
     else if(jump_opn == 6)
     {
     	r1 = get_register_content((IR >> 24) & 0x0000000F);
     	r2 = get_register_content((IR >> 16) & 0x0000000F);
     	if(r1>=r2)
     	{	
     		label = (IR >> 8) & 0x000000FF;
     		jump(label);
     	}
	else {
	PC = PC +4;
	}
     }

}

void jump(unsigned char label)
{
	int k=0;
	while(k<l)
	{
		if(labelAddr[k].label_Name==label)
		{
			PC = labelAddr[k].nextPC;
			//printf("Jumping to label:%u\n", label);
		}
		k++;
	}
}

unsigned char get_binary_opcode(unsigned char *instruction) 
{

    if(strcmp("LI",instruction)==0 || strcmp("li",instruction)==0)
		return LOADI;
    else if(strcmp("SI",instruction)==0 || strcmp("si",instruction)==0)
		return STOREI;
    else if(strcmp("LW",instruction)==0 || strcmp("lw",instruction)==0)
		return LOAD;
    else if(strcmp("SW",instruction)==0 || strcmp("sw",instruction)==0)
		return STORE;
    else if(strcmp("ADD",instruction)==0 || strcmp("SUB",instruction)==0 || strcmp("DIV",instruction)==0 || strcmp("MUL",instruction)==0 || strcmp("MOD",instruction)==0)
        	return ARITHMETIC;
    else if(strcmp("MOV",instruction)==0 || strcmp("mov",instruction)==0)
		return MOVE;
    else if(strcmp("JMP",instruction)==0 || strcmp("JG",instruction)==0 || strcmp("JE",instruction)==0 || strcmp("JL",instruction)==0 || strcmp("JLE",instruction)==0 || strcmp("JGE",instruction)==0)
		return JUMP;
    else if(strcmp("LABEL",instruction)==0)
		return LABEL;
    else if(strcmp("LEA",instruction)==0)
		return LEA;
    else if(strcmp("PUSH",instruction)==0)
		return PUSH;
    else 
    {
		printf("Invalid opcode\n");
		exit(1);
    }

}

unsigned char get_binary_registerID(unsigned char *instruction)
{
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
	else if(strcmp("Rz",instruction)==0 || strcmp("rz",instruction)==0)
		return Rz_ID;
	else if(strcmp("Ru",instruction)==0 || strcmp("ru",instruction)==0)
		return Ru_ID;
	else if(strcmp("V0",instruction)==0 || strcmp("V0",instruction)==0)
		return V0_ID;
	else 
	{
		printf("Invalid Register ID\n");
		exit(1);
	}
}

unsigned char get_binary_labelID(unsigned char *instruction)
{
	if(strcmp("L1",instruction)==0)
		return L1;
	else if(strcmp("L2",instruction)==0)
		return L2;
	else if(strcmp("L3",instruction)==0)
		return L3;
	else if(strcmp("L4",instruction)==0)
		return L4;
	else if(strcmp("L5",instruction)==0)
		return L5;
	else if(strcmp("L6",instruction)==0)
		return L6;
	else 
	{
		printf("Invalid Label ID\n");
		exit(1);
	}
}


unsigned char get_binary_arithmetic_opcode(unsigned char *instruction) 
{
    if(strcmp("ADD",instruction)==0 || strcmp("add",instruction)==0)
		return ADD;
    else if(strcmp("SUB",instruction)==0 || strcmp("sub",instruction)==0)
		return SUB;
    else if(strcmp("MUL",instruction)==0 || strcmp("mul",instruction)==0)
		return MUL;
    else if(strcmp("DIV",instruction)==0 || strcmp("div",instruction)==0)
		return DIV;
    else if(strcmp("MOD",instruction)==0 || strcmp("mod",instruction)==0)
		return MOD;
    else 
    {
		printf("Invalid Arithmetic function\n");
        exit(1);
	}
}

unsigned char get_binary_jump_opcode(unsigned char *instruction)
{
	if(strcmp("JE",instruction)==0)
		return JE;
	else if(strcmp("JG",instruction)==0)
		return JG;
	else if(strcmp("JMP",instruction)==0)
		return JMP;
	else if(strcmp("JL",instruction)==0)
		return JL;
	else if(strcmp("JLE",instruction)==0)
		return JLE;
	else if(strcmp("JGE",instruction)==0)
		return JGE;
	else
	{
		printf("Invalid Jump function\n");
                exit(1);
	}
}

void load_immediate() 
{
    //Left Shift 4 times so that last 20 bits represent the immidiate value stroed in the instruction      
    unsigned int immediateValue = (IR >> 4) & 0x000FFFFF;
    //Left Shift 24 times so that last 4 bits represent the destination register
    unsigned char reg_id = (IR >> 24) & 0x0000000F;       
    MDR = immediateValue;
    MAR = 0;
    assign_data_to_register(immediateValue,reg_id);	
}

void store_immediate() 
{        
        unsigned int address = (IR >> 4) & 0x000FFFFF;
        unsigned char reg_id = (IR >> 24) & 0x0000000F;
        store_data_to_memory(reg_id,address);
}

void load() 
{
	unsigned int value = (IR >> 4) & 0x000FFFFF,address;
    	unsigned char reg1_id = (IR >> 24) & 0x0000000F,reg2_id = IR & 0x0000000F;
        
	switch(reg2_id) 
	{
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
			printf("Register ID Invalid\n");
			exit(1);
	}
    get_data_from_memory(reg1_id,address);
}


void store() 
{
    	unsigned int value = (IR >> 4) & 0x000FFFFF,address;
    	unsigned char reg1_id = (IR >> 24) & 0x0000000F,reg2_id = IR & 0x0000000F;
        
	switch(reg2_id) 
	{
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

void move() 
{
	unsigned int data = get_register_content((IR >> 24) & 0x0000000F);
	assign_data_to_register(data,(IR >> 20) & 0x0000000F);
}

void get_data_from_memory(unsigned char reg_id,unsigned int address) 
{     
    MDR = memory[address];
    MAR = address;

    switch(reg_id) 
    {
		case Ra_ID:
			Ra = MDR;
			//printf("Ra = %x\n",Ra);
			break;
		case Rb_ID:
			Rb = MDR;
			//printf("Rb = %x\n",Rb);
			break;
		case Rc_ID:
			Rc = MDR;
			//printf("Rc = %x\n",Rc);
			break;
		case Rd_ID:
			Rd = MDR;
			//printf("Rd = %x\n",Rd);
			break;
		case Re_ID:
			Re = MDR;
			//printf("Re = %x\n",Re);
			break;
		case Rf_ID:
			Rf = MDR;
			//printf("Rf = %x\n",Rf);
			break;
		case Rg_ID:
			Rg = MDR;
			//printf("Rg = %x\n",Rg);
			break;
		case Rh_ID:
			Rh = MDR;
			//printf("Rh = %x\n",Rh);
			break;
		default:
			printf("Register ID invalid\n");
			exit(1);
	}
}

void store_data_to_memory(unsigned char reg_id, unsigned int address) 
{
    MAR = address;	
    
	switch(reg_id) 
	{
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
	printf("Data at destination address 0x%x = 0x%x\n",address,memory[address]);
}

void perform_arithmetic_operation() 
{
	unsigned int result,operand1,operand2;
	operand1 = get_register_content((IR >> 20) & 0x0000000F);
	operand2 = get_register_content((IR >> 16) & 0x0000000F);

	switch(((IR >> 12) & 0x0000000F))
	{	
		case ADD:   
	    	result = (*addition)(operand1, operand2);
			assign_data_to_register(result, (IR >> 24) & 0x0000000F);
	      	break;
		case SUB: 
	      	result = (*substraction)(operand1, operand2);
			assign_data_to_register(result, (IR >> 24) & 0x0000000F);
	      	break;
		case MUL: 
	      	result = (*multiplication)(operand1, operand2);
			assign_data_to_register(result, (IR >> 24) & 0x0000000F);
	      	break;
	  	case DIV: 
	      	result = (*divide)(operand1, operand2);
			assign_data_to_register(result, (IR >> 24) & 0x0000000F);
	      	break;
		case MOD: 
	      	result = (*modulo)(operand1, operand2);
			assign_data_to_register(result, (IR >> 24) & 0x0000000F);
	      	break;
		default:
	      	printf("Invalid Arithmetic Operation\n");
	      	exit(1);
   } 
}

unsigned int get_register_content(unsigned char registerID) 
{
 	switch(registerID) 
 	{
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
		case Rz_ID:
			return Rz;
		case Ru_ID:
			return Ru;
		case V0_ID:
			return V0;
		default:
			printf("Register ID invalid\n");
			exit(1);
	}
}

void assign_data_to_register(unsigned int data, unsigned char reg_id)
{

	switch(reg_id) 
	{
		case Ra_ID:
			Ra = data;
	        	//printf("Ra = %x\n",Ra);
			break;
		case Rb_ID:
			Rb = data;
	        	//printf("Rb = %x\n",Rb);
			break;
		case Rc_ID:
			Rc = data;
	        	//printf("Rc = %x\n",Rc);
			break;
		case Rd_ID:
			Rd = data;
			//printf("Rd = %x\n",Rd);
			break;
		case Re_ID:
			Re = data;
			//printf("Re = %x\n",Re);
			break;
		case Rf_ID:
			Rf = data;
			//printf("Rf = %x\n",Rf);
			break;
		case Rg_ID:
			Rg = data;
			//printf("Rg = %x\n",Rg);
			break;
		case Rh_ID:
			Rh = data;
			//printf("Rh = %x\n",Rh);
			break;
		case V0_ID:
			V0 = data;
			//printf("V0 = %x\n",Rh);
			break;
		default:
			printf("Register Id invalid\n");
			exit(1);
	}
}

void print_contents() 
{  
	printf("=====================================================\n");      
	printf("| Ra: 0x%x | ",Ra);
	printf("Rb: 0x%x | ",Rb);
	printf("Rc: 0x%x | ",Rc);
	printf("Rd: 0x%x | ",Rd);
	printf("Re: 0x%x | ",Re);
	printf("Rf: 0x%x | ",Rf);
	printf("Rg: 0x%x | ",Rg);
	printf("Rh: 0x%x | ",Rh);
	printf("V0: 0x%x | ",V0);
	printf("SP: 0x%x | \n",SP);	
	printf("| PC = 0x%x | ", PC);
	printf("MDR = 0x%x | ", MDR);
	printf("MAR = 0x%x | \n", MAR);
	printf("=====================================================\n");
}













	


