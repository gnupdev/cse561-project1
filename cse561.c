#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int now_cycle = 0;

int tracer[10000][20];

int maptable[67];
int freelist[200];
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

	int trace;		
}issue_queue;
int now_issue = 0;

int** DE;
int** RN;
int** DI;
int** RR;
int** execute_list;
int** WB;


// flag if use ?? Regitsers
int DE_flag = 0;
int RN_flag = 0;
int DI_flag = 0;
int RR_flag = 0;
int EX_flag = 0;
int WB_flag = 0;


int ROB_SIZE = 0;
int WIDTH = 0;
int IQ_SIZE = 0;

int** ROB;
int now_ROB = 0;
int end_file = 0;
int num_commit = 0;

issue_queue* IQ;

void init(int input_ROB_SIZE, int input_IQ_SIZE, int input_WIDTH){	
	ROB_SIZE = input_ROB_SIZE;
	IQ_SIZE = input_IQ_SIZE;
	WIDTH = input_WIDTH;

	return;
}

int flag_bday = 0;

void push_issue(int pc, int op, int inp1, int inp2, int dst, int trace){

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

	IQ[now_issue].trace = trace;
	

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
			temp.trace = IQ[i].trace;

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
				IQ[j].trace = IQ[j+1].trace;
				
				IQ[j+1].pc = -1;
				IQ[j+1].op = -1;
				IQ[j+1].inp1 = -1;
				IQ[j+1].inp1_ready = -1;
				IQ[j+1].inp2 = -1;
				IQ[j+1].inp2_ready = -1;
				IQ[j+1].dst = -1;
				IQ[j+1].bday = -1;
				IQ[j+1].trace = -1;



			}
		}
	}
	now_issue--;
}

int num_free = 67;
void push_freelist(int phy_rg){
	freelist[num_free] = phy_rg;
	num_free++;
}

int pop_freelist(){
	int temp = freelist[0];
	for(int i = 0; i < (134) - 1; i++){
		freelist[i] = freelist[i+1];
		freelist[i+1] = -1;
	}

	num_free--;
	return temp;
}



void push_ROB(int pc,int fetch_count){
	ROB[now_ROB][0] = pc;
	ROB[now_ROB][2] = -1;
	ROB[now_ROB][3] = fetch_count;
	now_ROB++;
}

void push_free_reg(int pc, int free_reg){
	for(int i = 0; i < ROB_SIZE; i++){
		if(ROB[i][0] == pc){
			ROB[i][1] = free_reg;
			break;
		} 
	}
}

void pop_ROB(int how_many){
	// Reorder ROB
	//printf("how %d ", how_many);
	for(int i = 0; i < how_many; i++){

		if(ROB[i][1] >= 0) push_freelist(ROB[i][1]);
		
		tracer[num_commit][10] = now_cycle + 1;

		for(int j = 0; j <ROB_SIZE -1; j++){
			//if(ROB[j+1][0] == NULL ) printf("%d sex\n", j);
			ROB[j][0] = ROB[j+1][0];
			ROB[j][1] = ROB[j+1][1];
			ROB[j][2] = ROB[j+1][2];
			ROB[j][3] = ROB[j+1][3];
			ROB[j+1][0] = -1;
			ROB[j+1][1] = -1;
			ROB[j+1][2] = -1;
			ROB[j+1][3] = -1;
	}
		num_commit++;


	}

	


	now_ROB = now_ROB - how_many;	
}

int CM_flag = 0;
void commit(){

	int how_many = 0;


	if(ROB[0][2] == 0) how_many = 0;
	else if(ROB[0][2] == 1 && ROB[1][2] == -1) how_many = 1; 
	else if(ROB[0][2] == 1 && ROB[1][2] == 1 && ROB[2][2] == -1) how_many = 2;
	else if(ROB[0][2] == 1 && ROB[1][2] == 1 && ROB[2][2] == 1) how_many = 3;

	if(how_many != 0) {

		pop_ROB(how_many);
	}
	for(int i = 0; i <ROB_SIZE; i++) if(ROB[i][0] != -1 && ROB[i][2] == 1) CM_flag++;





}

void writeback(){

	

	if(WB_flag <= 0) return;
	else{
		for(int i = 0; i < WB_flag; i++){
			for(int j = 0; j < ROB_SIZE; j++){
				if(ROB[j][3] == WB[i][1]){
					ROB[j][2] = 1;
					tracer[WB[i][1]][9] = now_cycle + 1;
					break;
				}

			}
		}
		for(int i = 0 ; i < WIDTH*5;i++) WB[i][0] = -1;
		WB_flag = 0;
	}	
}

void execute(){
	int m = 0;
	if(EX_flag == 0 || WB_flag == WIDTH*5){
		return;
	}
	else{
		for(int i = 0; i < WIDTH * 5; i++){
			if(execute_list[i][0] != -1) {execute_list[i][1]++;}
		}


		for(int ex_loop = 0; ex_loop < WIDTH * 5; ex_loop++){
			if(execute_list[ex_loop][0] != -1){
				if(execute_list[ex_loop][1] >= execute_list[ex_loop][2]){

					int ready_reg = execute_list[ex_loop][3];
					WB[m][0] = execute_list[ex_loop][0];
					execute_list[ex_loop][0] = -1;
					execute_list[ex_loop][1] = -1;
					execute_list[ex_loop][2] = -1;
					execute_list[ex_loop][3] = -1;
				
					tracer[execute_list[ex_loop][6]][8] = now_cycle + 1;	
					WB[m][1] = execute_list[ex_loop][6];				

					EX_flag--;
					WB_flag++;				 		

					if(ready_reg == -1) {
					}

					else{
						readytable[ready_reg] = 1;
						for(int i = 0; i < IQ_SIZE; i++){
							if(IQ[i].inp1 == ready_reg) {
								IQ[i].inp1_ready = 1;
							}
						}
						for(int i = 0; i < IQ_SIZE; i++){
							if(IQ[i].inp2 == ready_reg) {
								IQ[i].inp2_ready = 1;
							}
						}




					}



					m++;
				}

			}
		}

	}	
}


void regRead(){

	int remain_ex_list = WIDTH * 5 - EX_flag;
	if(RR_flag == 0 || remain_ex_list == 0){
		return;
	}
	else{

		for(int rr_loop = 0; rr_loop < RR_flag; rr_loop++){
			if(RR[rr_loop][0] != -1){
				for(int ex_loop = 0; ex_loop < WIDTH * 5; ex_loop++){
					if(execute_list[ex_loop][0] == -1){
						execute_list[ex_loop][0] = RR[rr_loop][0];
						execute_list[ex_loop][1] = 0;
						if(RR[rr_loop][1] == 0) execute_list[ex_loop][2] = 1;
						else if(RR[rr_loop][1] == 1) execute_list[ex_loop][2] = 2;
						else if(RR[rr_loop][1] == 2) execute_list[ex_loop][2] = 5;
						execute_list[ex_loop][3] = RR[rr_loop][4];
						EX_flag++;


						execute_list[ex_loop][6] = RR[rr_loop][6];
						tracer[RR[rr_loop][6]][7] = now_cycle + 1;
							
						RR[rr_loop][0] = -1;
						break;
					}
				}
	
		//	for(int i = 0; i < 6;i++) RR[rr_loop][i] = -1;
			}
		}
		RR_flag = 0;
	}
}
void issue(){

	issue_queue* pop = (issue_queue*)malloc(sizeof(issue_queue));
	int* list = (int*)malloc(sizeof(int) * WIDTH);
	if(now_issue == 0){
		return;
	}
	else{

		int num_issue = 0;
		for(int i = 0; i < IQ_SIZE; i++){
			if(IQ[i].pc != -1 && IQ[i].inp1_ready == 1 && IQ[i].inp2_ready == 1){

				list[num_issue++] = IQ[i].bday;
			}

			if(num_issue == WIDTH) break;

		}
		if(num_issue == 0) return;

		for(int i = 0; i < num_issue; i++){
			pop_issue(list[i], pop);
			RR[i][0] = pop[0].pc;
			RR[i][1] = pop[0].op;
			RR[i][2] = pop[0].inp1;
			RR[i][3] = pop[0].inp2;
			RR[i][4] = pop[0].dst;
			RR[i][6] = pop[0].trace;

			tracer[RR[i][6]][6] = now_cycle+1;
			RR_flag++;
		}
	}
	free(pop);
	free(list);
}
void dispatch(){
	int remain_iq = IQ_SIZE - now_issue;

	if(remain_iq < DI_flag || DI_flag == 0){

		return;
	}
	else{

		for(int di_loop = 0; di_loop < DI_flag; di_loop++){
			readytable[DI[di_loop][2]] = -1;
			push_issue(DI[di_loop][0], DI[di_loop][1], DI[di_loop][3], DI[di_loop][4], DI[di_loop][2], DI[di_loop][6]);
			tracer[DI[di_loop][6]][5] = now_cycle+1;

		}
		DI_flag = 0;
	}

}


void renaming(){
	int num_rename = RN_flag;

	if(DI_flag > 0 || num_rename == 0 || num_free <= 0){
		return;
	}
	else{
		for(int rn_loop = 0; rn_loop < num_rename; rn_loop++){
			DI[rn_loop][0] = RN[rn_loop][0];
			DI[rn_loop][1] = RN[rn_loop][1];
			DI[rn_loop][6] = RN[rn_loop][6];

			
			tracer[RN[rn_loop][6]][4] = now_cycle+1;
			
			if (RN[rn_loop][3] == -1) DI[rn_loop][3] = -1;
			else {
				DI[rn_loop][3] = maptable[RN[rn_loop][3]];

			}
			if (RN[rn_loop][4] == -1) DI[rn_loop][4] = -1;
			else {
				DI[rn_loop][4] = maptable[RN[rn_loop][4]];

			}

			if(RN[rn_loop][2] == -1){ // dst
				DI[rn_loop][2] = -1;	
			}
			else{

				int old = maptable[RN[rn_loop][2]];
				int temp = pop_freelist();
				push_free_reg(RN[rn_loop][0], old);
				maptable[RN[rn_loop][2]] = temp;
				DI[rn_loop][2] = temp;
			}


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
				for(int rn_loop = 0; rn_loop < 7; rn_loop++){
					RN[de_loop][rn_loop] = DE[de_loop][rn_loop];
				}	
				tracer[DE[de_loop][6]][3] = now_cycle+1;// tracer RN
			}
				
	
			RN_flag++;
			DE_flag--;
		}
	}

}
int fu = 0;

int fetch_count = 0;
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
					push_ROB(DE[de_loop][0], fetch_count);	
				}
				else {
					DE[de_loop][in] = atoi(temp);	
				}

			}
			if(feof(fp) != 0) { // 다음 읽을 inst이 eof 인지 확인
				end_file = 1;
				break;
			}
			DE[de_loop][6] = fetch_count;
			tracer[fetch_count][0] = fetch_count;
			tracer[fetch_count][1] = now_cycle;	
			tracer[fetch_count][2] = now_cycle+1;
	
			tracer[fetch_count][11] = DE[de_loop][1];
			tracer[fetch_count][12] = DE[de_loop][3];
			tracer[fetch_count][13] = DE[de_loop][4];
			tracer[fetch_count][14] = DE[de_loop][2];	
			


			fetch_count++;
			DE_flag++;
			

		}
	}
}
int check(FILE* fp){
	int pip_reg = DE_flag + RN_flag + DI_flag + now_issue + RR_flag + EX_flag + WB_flag + (now_ROB-1);
	now_cycle++;
	if(pip_reg == 0 && feof(fp) != 0) return 0;
	else return 1;


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

	FILE* fp;
	fp = fopen(argv[4], "r");

	if(fp == NULL) printf("Can not read input file\n");
	int pip_reg = 0;


	DE = (int**)calloc(WIDTH, sizeof(int*));
	RN = (int**)calloc(WIDTH, sizeof(int*));
	DI = (int**)calloc(WIDTH, sizeof(int*));
	RR = (int**)calloc(WIDTH, sizeof(int*));
	WB = (int**)calloc(WIDTH*5, sizeof(int*));
	for(int i = 0; i < WIDTH; i++){
		DE[i] = (int*)calloc(6, sizeof(int)); // 32 is buffer size
		RN[i] = (int*)calloc(6, sizeof(int));
		DI[i] = (int*)calloc(6, sizeof(int)); // + original dst register
		RR[i] = (int*)calloc(7, sizeof(int));
		


	}	

	execute_list = (int**)malloc(sizeof(int*) * WIDTH * 6);
	for(int i = 0; i < WIDTH * 5; i++){
		execute_list[i] = (int*)malloc(sizeof(int) * 7); // [0] = "program counter", [1] = "strart clock", [2] = execution_time, [3] = destination_register
		WB[i] = (int*)malloc(sizeof(int) * 2);

		execute_list[i][0] = -1;
		execute_list[i][1] = -1;
		execute_list[i][2] = -1;
		execute_list[i][3] = -1;
	}


	ROB = (int**)malloc(sizeof(int*) * (ROB_SIZE + 1));	

	for(int i = 0;i < ROB_SIZE+1; i++){
		ROB[i] = (int*)malloc(sizeof(int) * 4); // [0] = "program counter", [1] = "free reg", [2] = "done"
		ROB[i][1] = -1;
		ROB[i][1] = -1;
		ROB[i][2] = -1;
	}


	memset(freelist, -1, sizeof(freelist));
	for(int i = 0; i < sizeof(maptable) / 4; i++){
		maptable[i] = i;
		freelist[i] = i + 67;
	}

	IQ = (issue_queue*)malloc(sizeof(issue_queue) * IQ_SIZE);

	for(int i = 0; i < IQ_SIZE; i++){
		IQ[i].pc = -1;
		IQ[i].bday = -1;
	}

	for(int i = 0 ; i < WIDTH * 5; i++) WB[i][0] = -1;

	for(int i = 0; i < 67; i++) {readytable[i] = 1; readytable[i+67] = -1;}


	for(int i = 0 ; i < 10000; i ++ ) for(int j = 0 ; j < 100; j ++) tracer[i][j] = -1;


	do{
		commit();
		writeback();
		execute();
		regRead();
		issue();
		dispatch();
		renaming();
		decode();
		fetch(fp);

		//for (int i = 0; i < 134; i++) printf(" %d \n" , readytable[i]);
	
		for(int i = 0 ; i<ROB_SIZE;i++){printf("%d %x %d %d %d\n", ROB[i][3],ROB[i][0], ROB[i][1], ROB[i][2], now_ROB);}
		printf("%d %d %d %d %d %d %d %d %d\n", DE_flag, RN_flag, DE_flag, now_issue, DI_flag ,RR_flag, EX_flag, WB_flag, CM_flag);
		printf("\n");
		//printf("%d %d\n", now_cycle, num_commit);

	}while(check(fp));

	for(int i = 0 ; i < num_commit; i++) {
		printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} DI{%d,%d} IS{%d,%d} RR{%d,%d} EX{%d,%d} WB{%d,%d} CM{%d,%d}\n", 
tracer[i][0], 
tracer[i][11], 
tracer[i][12], tracer[i][13], 
tracer[i][14],
tracer[i][1], tracer[i][2] - tracer[i][1], 
tracer[i][2], tracer[i][3]-tracer[i][2],
 tracer[i][3], tracer[i][4]-tracer[i][3], 
tracer[i][4], tracer[i][5]-tracer[i][4], 
tracer[i][5], tracer[i][6]-tracer[i][5], 
tracer[i][6], tracer[i][7]-tracer[i][6],
 tracer[i][7], tracer[i][8]-tracer[i][7], 
tracer[i][8], tracer[i][9]-tracer[i][8], 
tracer[i][9], tracer[i][10]-tracer[i][9]);
	}
	printf("# === Simulator Command =========#\n");
	printf("# %s %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
	printf("# === Processor Configuration ===#\n");
	printf("# ROB_SIZE = %d			  \n", ROB_SIZE);
	printf("# IQ_SIZE  = %d			  \n", IQ_SIZE);
	printf("# WIDTH    = %d			  \n", WIDTH);
	printf("# === Simulation Results ========#\n");
	printf("# Dynamic Instruction Count = %d  \n", num_commit);
	printf("# Cycles		    = %d  \n", --now_cycle);
	printf("# Instructions Per Cycle    = %.2f  \n", ((float)num_commit / (float)now_cycle));

	fclose(fp);

}


