#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int maptable[67];
int freelist[134];
int readytable[134];


typedef struct issue_queue {
	int pc;
	
	int op;

	int inp1;
	int inp1_ready;

	int inp2;
	int inp2_ready;

	int dst;
	int bday;		
}issue_queue;
int now_issue = 0;

int** DE;
int** RN;
int** DI;
int** RR;
int** execute_list;
int* WB;


// flag if use ?? Regitsers
int DE_flag = 0;
int RN_flag = 0;
int DI_flag = 0;
int RR_flag = 0;
int EX_flag = 0;
int WB_flag = 0;

int now_cycle = 0;

int ROB_SIZE = 0;
int WIDTH = 0;
int IQ_SIZE = 0;

int** ROB;
int now_ROB = 0;
int end_file = 0;

issue_queue* IQ;

void init(int input_ROB_SIZE, int input_IQ_SIZE, int input_WIDTH){	
	ROB_SIZE = input_ROB_SIZE;
	IQ_SIZE = input_IQ_SIZE;
	WIDTH = input_WIDTH;

	
	DE = (int**)malloc(sizeof(int*) * WIDTH);
	RN = (int**)malloc(sizeof(int*) * WIDTH);
	DI = (int**)malloc(sizeof(int*) * WIDTH);
	RR = (int**)malloc(sizeof(int*) * WIDTH);
	WB = (int*)malloc(sizeof(int) * WIDTH * 5);
	for(int i = 0; i < WIDTH; i++){
		DE[i] = (int*)malloc(sizeof(int) * 5); // 32 is buffer size
		RN[i] = (int*)malloc(sizeof(int) * 5);
		DI[i] = (int*)malloc(sizeof(int) * 6); // + original dst register
		RR[i] = (int*)malloc(sizeof(int) * 5);
	}	

	execute_list = (int**)malloc(sizeof(int*) * WIDTH * 5);
	for(int i = 0; i < WIDTH * 5; i++){
		execute_list[i] = (int*)malloc(sizeof(int) * 4); // [0] = "program counter", [1] = "strart clock", [2] = execution_time, [3] = destination_register
	
		execute_list[i][0] = -1;
	}

	//memset(DE, -1, sizeof(DE));
	memset(WB, -1, sizeof(WB));
	printf("Pipeline registers are created!\n");	

	ROB = (int**)malloc(sizeof(int*) * ROB_SIZE);	

	for(int i = 0;i < ROB_SIZE; i++){
		ROB[i] = (int*)malloc(sizeof(int) * 5); // [0] = "program counter", [1] = "free reg", [2] = "done"
		ROB[i][1] = -1;
	}
	printf("ROB is created!\n");


	memset(freelist, -1, sizeof(freelist));
	memset(readytable, -1, sizeof(readytable));
	for(int i = 0; i < sizeof(maptable) / 4; i++){
		maptable[i] = i;
		readytable[i] = 1;
		freelist[i] = i + 67;
	}

	IQ = (issue_queue*)malloc(sizeof(issue_queue) * IQ_SIZE);

	for(int i = 0; i < IQ_SIZE; i++){
		IQ[i].pc = -1;
		IQ[i].bday = -1;
	}

	return;
}

int flag_bday = 0;

void push_issue(int pc, int op, int inp1, int inp2, int dst){

	IQ[now_issue].pc = pc;
	IQ[now_issue].op = op;

	IQ[now_issue].inp1 = inp1;
	if(inp1 == -1) IQ[now_issue].inp1_ready = 1;
	else IQ[now_issue].inp1_ready = readytable[inp1];
	
	IQ[now_issue].inp2 = inp2;
	if(inp2 == -1) IQ[now_issue].inp2_ready = 1;
	else IQ[now_issue].inp2_ready = readytable[inp2];

	IQ[now_issue].dst = dst;	
	IQ[now_issue].bday = flag_bday;

	flag_bday++;
	now_issue++;

}

void pop_issue(int bday, issue_queue* pop){
	issue_queue temp;

	for(int i = 0; i < IQ_SIZE; i++){
		if(IQ[i].pc != -1 && IQ[i].bday == bday){
			temp.pc = IQ[i].pc;
			temp.op = IQ[i].op;
			temp.inp1 = IQ[i].inp1;
			temp.inp2 = IQ[i].inp2;
			temp.dst = IQ[i].dst;

			memcpy(pop, &temp, sizeof(temp));

			for(int j = i; j < IQ_SIZE - 1; j++){
				IQ[j].pc = IQ[j+1].pc;
				IQ[j].op = IQ[j+1].op;
				IQ[j].inp1 = IQ[j+1].inp1;
				IQ[j].inp1_ready = IQ[j+1].inp1_ready;
				IQ[j].inp2 = IQ[j+1].inp2;
				IQ[j].inp2_ready = IQ[j+1].inp2_ready;
				IQ[j].dst = IQ[j+1].dst;
				IQ[j].bday = IQ[j+1].bday;
				IQ[j+1].pc = -1;
			}
		}
	}
	now_issue--;
}

int num_free = 67;
void push_freelist(int phy_rg){
	freelist[num_free++] = phy_rg;
}

int pop_freelist(){
	int temp = freelist[0];
	for(int i = 0; i < (sizeof(freelist) / 4) - 1; i++){
		freelist[i] = freelist[i+1];
		freelist[i+1] = -1;
		num_free--;
	}

	return temp;
}



void push_ROB(int pc){
	ROB[now_ROB][0] = pc;
	ROB[now_ROB][2] = 0;
 	now_ROB++;	
}

void push_free_reg(int pc, int free_reg){
	for(int i = 0; i < ROB_SIZE; i++){
		if(ROB[i][0] == pc){
			ROB[i][1] = free_reg;
		} 
	}
}

void pop_ROB(int how_many){	
	// Reorder ROB
	for(int i = 0; i < ROB_SIZE - 1; i++){
		ROB[i] = ROB[i+how_many]; // move ROB
		push_freelist(ROB[i][1]);
		//phy_reg[ROB[i][1]] = 0; // free reg
		for(int i = 0; i < 3; i++){ // clean the ROB
			ROB[i+how_many][i] = -1;
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
	if(WB_flag == 0){
		return;	
	}
	else {
		for(int wb_loop = 0; wb_loop < WIDTH * 5; wb_loop++){
			if(WB[wb_loop] != -1){	
				for(int rob_loop = 0; rob_loop < ROB_SIZE; rob_loop++){
					if(ROB[rob_loop][0] == WB[wb_loop]){
						ROB[rob_loop][2] = 1;
						WB[wb_loop] = -1;
						WB_flag--;
						break;
					}
				}
			}
		}	
	}
}

void execute(){

	if(EX_flag == 0){
		return;
	}
	else{
		for(int ex_loop = 0; ex_loop < WIDTH * 5; ex_loop++){
			int check = now_cycle - execute_list[ex_loop][1];
			if(execute_list[ex_loop][0] != -1 && check >= execute_list[ex_loop][2]){
				for(int wb_loop = 0; wb_loop < WIDTH * 5; wb_loop++){
					if(WB[wb_loop] == -1){
						WB[wb_loop] = execute_list[ex_loop][0];
						readytable[execute_list[ex_loop][3]] = 1;
	
	
						execute_list[ex_loop][0] = -1;
						execute_list[ex_loop][1] = -1;
						execute_list[ex_loop][2] = -1;
						EX_flag--;
						WB_flag++;
						break;
					}
	
				}
			}
		}

	}
		
}


void regRead(){
	int remain_ex_list = WIDTH * 5 - EX_flag;

	if(remain_ex_list == 0 || RR_flag == 0){
		return;
	}
	else{

		int num_regRead = 0;
		for(int rr_loop = 0; rr_loop < WIDTH; rr_loop++){
			if(RR[rr_loop][0] != -1){
				num_regRead++;
				RR_flag--;
				for(int ex_loop = 0; ex_loop < WIDTH * 5; ex_loop++){
					if(execute_list[ex_loop][0] == -1){
						execute_list[ex_loop][0] = RR[rr_loop][0];
						execute_list[ex_loop][1] = now_cycle;
						if(RR[rr_loop][1] == 0) execute_list[ex_loop][2] = 1;
						else if(RR[rr_loop][1] == 1) execute_list[ex_loop][2] = 2;
						else execute_list[ex_loop][2] = 5;
						execute_list[ex_loop][3] = RR[rr_loop][4];
						EX_flag++;
						break;
					}
				}
			}
		}
	}
}

void issue(){
	issue_queue* pop = (issue_queue*)malloc(sizeof(issue_queue));
	int* list = (int*)malloc(sizeof(int) * WIDTH);
	int num_issue = 0;
	if(now_issue == 0 || RR_flag != 0){
		return;
	}
	else{	
		for(int i = 0; i < IQ_SIZE; i++){
			if(num_issue > WIDTH) break;
			if(IQ[i].pc != -1 && readytable[IQ[i].inp1] == 1 && readytable[IQ[i].inp2] == 1){
				list[num_issue++] = IQ[i].bday;
			}

		}
		for(int i = 0; i < num_issue; i++){
			pop_issue(list[i], pop);

			RR[i][0] = pop[0].pc;
			RR[i][1] = pop[0].op;
			RR[i][2] = pop[0].inp1;
			RR[i][3] = pop[0].inp2;
			RR[i][4] = pop[0].dst;
			RR_flag++;
		}

	}
}

void dispatch(){
	int num_dispatch = DI_flag;
	int remain_iq = IQ_SIZE - now_issue;

	if(num_dispatch > remain_iq || remain_iq == 0 || num_dispatch == 0){
		return;
	}
	else{
		if(remain_iq < num_dispatch) num_dispatch = remain_iq;
	
		for(int di_loop = 0; di_loop < num_dispatch; di_loop++){
			readytable[DI[di_loop][2]] = 0;
			push_issue(DI[di_loop][0], DI[di_loop][1], DI[di_loop][3], DI[di_loop][4], DI[di_loop][2]);
			DI_flag--;
	
		}
	}
}


void renaming(){
	int num_rename = RN_flag;

	if(DI_flag > 0 || num_rename == 0){
		return;
	}
	else{
		for(int rn_loop = 0; rn_loop < num_rename; rn_loop++){
			DI[rn_loop][0] = RN[rn_loop][0];
			DI[rn_loop][1] = RN[rn_loop][1];
				if(RN[rn_loop][2] == -1){ // dst
					DI[rn_loop][2] = -1;	
				}
				else{
					int temp = pop_freelist();
					push_free_reg(RN[rn_loop][0], RN[rn_loop][2]);
					maptable[RN[rn_loop][2]] = temp;
					DI[rn_loop][2] = temp;
				}

				if (RN[rn_loop][3] == -1) DI[rn_loop][3] = -1;
				else DI[rn_loop][3] = maptable[RN[rn_loop][3]];
				
				if (RN[rn_loop][4] == -1) DI[rn_loop][4] = -1;
				else DI[rn_loop][4] = maptable[RN[rn_loop][4]];
			DI_flag++;
			RN_flag--;
		}
	} 
}


void decode(){
	int num_decode = DE_flag;
	
	if(RN_flag > 0 || num_decode == 0){
		return;	
	}
	else{
		for(int de_loop = 0; de_loop < num_decode; de_loop++){
			if(DE[de_loop][0] != -1){
				for(int rn_loop = 0; rn_loop < 5; rn_loop++){
					RN[de_loop][rn_loop] = DE[de_loop][rn_loop];
				}	
			}	
			RN_flag++;
			DE_flag--;
		}
	}

}

void fetch(FILE* fp){

	if(DE_flag > 0 || now_ROB == ROB_SIZE || feof(fp) != 0 ){
		return;
	}
	else{
 		int remain_ROB = ROB_SIZE - now_ROB;
		int num_fetch = 0;
		
		if(remain_ROB >= WIDTH){ // WIDTH 만큼 fetch
			num_fetch = WIDTH;
		}	
		
		else if(remain_ROB < WIDTH){ // 남아있는 ROB 만큼 fetch
			num_fetch = remain_ROB;
		}
		for(int de_loop = 0; de_loop < num_fetch; de_loop++){
			for(int in = 0; in < 5; in++){
				char temp[32];
				fscanf(fp, "%s", temp);
				if(in == 0) {
					DE[de_loop][in] = strtol(temp, NULL, 16);
				}
				else {
					DE[de_loop][in] = atoi(temp);	
				}

			}

			push_ROB(DE[de_loop][0]);	
			if(feof(fp) != 0) { // 다음 읽을 inst이 eof 인지 확인
				end_file = 1;
				break;
			}
	
			DE_flag++;
		}
	}
}


//int search_ROB(int pc){
//	int index = 0;
//
//	for(int i = 0; i < ROB_SIZE; i++){
//		if(	
//	}
//
//
//	return index;
//}

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

	FILE* fp;
	fp = fopen(argv[4], "r");

	if(fp == NULL) printf("Can not read input file\n");
	//while(!feof(fp)) {
	//	fetch(fp);
	//}
	int i = 0;
	char temp[32];

	while(end_file == 0){
		commit();
		writeback();
		execute();
		regRead();

		issue();

		dispatch();

		renaming();
	
		decode();

		fetch(fp);
		now_cycle++;

		printf("now_ROB: %d\n WB_flag: %d\n EX_flag: %d\n RR_flag: %d\n now_issue: %d\n DI_flag: %d\n RN_flag: %d\n DE_flag: %d\n", now_ROB, WB_flag, EX_flag, RR_flag, now_issue, DI_flag, RN_flag, DE_flag);
	}

commit();
writeback();
execute();
regRead();
issue();
dispatch();
renaming();
decode();
fetch(fp);
now_cycle++;
	//printf("%d %d %d\n", DE_flag, RN_flag, DI_flag);

//	for(int i=0;i<IQ_SIZE;i++) {
//		printf("%d %d %d %d %d %d %d %d\n", IQ[i].pc, IQ[i].op, IQ[i].inp1, IQ[i].inp1_ready
//							,IQ[i].inp2, IQ[i].inp2_ready, IQ[i].dst, IQ[i].bday);
//	}
//
//	for(int i = 0; i < RR_flag; i++){
//		for(int j = 0; j < 5; j++){
//			printf("%d ", RR[i][j]);
//		}
//		printf("\n");
//
}


//	for(int i = 0; i < now_issue; i++){
//		for(int j =0; j < 8;j++) {
//		//	printf("%d ", DE[i][j]);
//		}
//		printf("%d %d %d %d %d %d %d %d",IQ[i].pc, IQ[i].op, IQ[i].inp1, IQ[i].inp1_ready, IQ[i].inp2, IQ[i].inp2_ready, IQ[i].dst, IQ[i].bday); 
//		printf("\n");
//	}
	//for(int i = 0 ; i < sizeof(readytable) / 4; i++) printf("%d\n", readytable[i]);
	//printf("%d\n", ROB[2][1]);




