#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int arch_reg[67];
int phy_reg[134];

int* DE;
int* RN;
int* DI;
int* RR;
int** execute_list;
int* WB;


int ROB_SIZE = 0;
int WIDTH = 0;
int IQ_SIZE = 0;

int** ROB;
int now_ROB = -1;


void init(int input_ROB_SIZE, int input_WIDTH, int input_IQ_SIZE){	
	ROB_SIZE = input_ROB_SIZE;
	WIDTH = input_WIDTH;
	IQ_SIZE = input_IQ_SIZE;

	memset(arch_reg, 0, sizeof(arch_reg));
	memset(phy_reg, 0, sizeof(phy_reg));
	printf("Architecture and Physical Registers set 0\n");
	
	DE = (int*)malloc(sizeof(int) * WIDTH);
	RN = (int*)malloc(sizeof(int) * WIDTH);
	DI = (int*)malloc(sizeof(int) * WIDTH);
	RR = (int*)malloc(sizeof(int) * WIDTH);
	execute_list = (int**)malloc(sizeof(int*) * WIDTH * 5);
	for(int i = 0; i < WIDTH * 5; i++){
		execute_list[i] = (int*)malloc(sizeof(int) * 3); // [0] = "program counter", [1] = "strart clock", [2] = execution_time
	}

	WB = (int*)malloc(sizeof(int) * WIDTH * 5);
	memset(WB, -1, sizeof(WB));
	printf("Pipeline registers are created!\n");	

	ROB = (int**)malloc(sizeof(int) * ROB_SIZE);	

	for(int i = 0;i < ROB_SIZE; i++){
		ROB[i] = (int *)malloc(sizeof(int) * 3); // [0] = "program counter", [1] = "free reg", [2] = "done"
	}
	printf("ROB is created!\n");
	return;
}

void push_ROB(int type, int free_reg){
	ROB[++now_ROB][0] = type;
	ROB[now_ROB][1] = free_reg;
	ROB[now_ROB][2] = 0;
}
void pop_ROB(int how_many){	
	// Reorder ROB
	for(int i = 0; i < ROB_SIZE - 1; i++){
		ROB[i] = ROB[i+how_many]; // move ROB
		phy_reg[ROB[i][1]] = 0; // free reg
		for(int i = 0; i < 3; i++){ // clean the ROB
			ROB[i+how_many][i] = 0;
		}
	}
	now_ROB = now_ROB - how_many;	
}



void commit(){
	int how_many = 0;

	for(int i = 0; i < WIDTH; i++){
		if(ROB[i][2] == 1){
			how_many++;
		}
		else if(ROB[i][2] == 0){
			if(how_many != 0) pop_ROB(how_many);
			break;
		}
	}
}

void writeback(){
	for(int wb_loop = 0; wb_loop < WIDTH * 5; wb_loop++){
		if(WB[wb_loop] != -1){	
			for(int rob_loop = 0; rob_loop < ROB_SIZE; rob_loop++){
				if(ROB[rob_loop][0] == WB[wb_loop]){
					ROB[rob_loop][2] = 1;
					break;
				}
			}
			WB[wb_loop] = -1;
		}
	}	



}

void execute(){
	
	





}

int main(int argc, char *argv[]){





	if(argv[1] == NULL){
		printf("Please enter a <ROB_SIZE> \n");
	}

        else if(argv[2] == NULL){
		printf("Please enter a <IQ_SIZE> \n");
	}
	else if(argv[3] == NULL){
		printf("Please enter a <WIDTH> \n");
	}
	else if(argv[4] == NULL){
		printf("Input file is NULL \n");
	}



	init(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));

	ROB[0][0] = 100;
	ROB[0][1] = 1;
	ROB[0][2] = 3;

	ROB[1] = ROB[0];
	printf("%d %d %d\n", ROB[1][0], ROB[1][1], ROB[1][2]);	

}


