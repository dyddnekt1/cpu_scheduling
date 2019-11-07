#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct process {
	int id;// PID
	int arrivaltime; 
	int cpuburst;
	int io;// 1: has I/O interrupt, 0: NO I/O services
	int iotime; //the time amount between cpu burst start and I/O interrupt
	int ioburst;
	int priority; //low number means high priority
	int waitingtime;
	int turnaroundtime;
	char type; //type for multilevel queue (A,B,C)
} PROCESS;


int i,j,k,temp,swap; //indexes and temporary variable for swap
int num;//number of processes

PROCESS* processarr; 

void createProcess();
void printProcess();
void FCFS();
void SJF();
void Priority();
void Roundrobin();
void preemptiveSJF();
void preemptivePriority();
void SJF_with_aging();
void Priority_with_aging();
void preemptiveSJF_with_aging();
void preemptivePriority_with_aging();
void MultilevelQueue();
void MultilevelFeedbackQueue();
void printGaunt(int record[], int time, float awt, float att);


int main() {
	srand((unsigned)time(NULL));//seed for rand()

	createProcess(); //create random number of processes
	printProcess(); //print processes information

	//schedule processes with each algorithm and print result with Gaunt Chart
	FCFS(); 
	SJF(); 
	Priority();
	Roundrobin();
	preemptiveSJF();
	preemptivePriority();
	SJF_with_aging();
	Priority_with_aging();
	preemptiveSJF_with_aging();
	preemptivePriority_with_aging();
	MultilevelQueue();
	MultilevelFeedbackQueue();
	

	return 0;
}

void createProcess() {
	
	printf("Press any key to create random processes\n");
	getchar();
	
	num = (rand() % 8) + 3; // 3 ~ 10 processes

	processarr = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processarr[i].id = i;
		processarr[i].arrivaltime = rand() % 10; // 0 ~ 9 arrival time
		processarr[i].cpuburst = rand() % 10 + 2; // 2 ~ 11 cpu burst
		processarr[i].io = rand() % 2; // I/O interrupt 0: NO , 1: YES
		if (processarr[i].io) { // if it has I/O interrupt,
			processarr[i].iotime = (rand() % (processarr[i].cpuburst -1)) + 1; // 1 ~ cpuburst-1 i/o time
			processarr[i].ioburst = rand() % 3 + 1; // 1~3 I/O burst
		}
		else
		{
			processarr[i].iotime = 0;
			processarr[i].ioburst = 0;
		}
		processarr[i].priority = rand() % 15 + 1; //low number means high priority
		processarr[i].waitingtime = 0;
		processarr[i].turnaroundtime = 0;
		processarr[i].type = rand() % 3 + 65;// A or B or C
	}
	printf("%d processes are created\n\n",num);

}

void printProcess() { //print processes info
	for (i = 0; i < num; i++) {
		printf("Process id: %d, arrivaltime: %d, cpuburst: %d, type: %c \n", processarr[i].id, processarr[i].arrivaltime, processarr[i].cpuburst, processarr[i].type);
		printf("I/O ? : ");
		if (processarr[i].io)
			printf("YES, I/O time : %d, I/O burst : %d", processarr[i].iotime, processarr[i].ioburst);
		else
			printf("NO                              ");
		printf(", priority : %d\n\n", processarr[i].priority);
	}
}

void FCFS() {

	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) { //copy processarr
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival=0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt=0;// average waiting time
	float att=0;// average turnaround time
	

	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //when process arrives
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		if (running == -1 && (eoq > 0)) { //First come first served (scheduled)
			running = readyqueue[0];
			for (i = 0; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record the running process id for Gaunt Chart, -1: idle

		if (running >= 0) //decrement cpuburst
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //decrement I/O burst
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;// increment time

		for (i = 0; i < eoq; i++) //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//if cpuburst gets zero, the process terminates and set running to -1
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {//if I/O interrupt occurs, set iostate[running] to 1
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;//set running to -1
			}
		}


		for (i = 0; i < num; i++) { //if I/O burst gets zero the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		if ((time > lastarrival) && (running == -1) && (eoq == 0)) //all processes have arrived, no process is running, nothing in ready queue
			end = 1; //set end to 1
		for (i = 0; i < num; i++) { //if there is process doing I/O service, it is not end
			if (iostate[i] == 1)
				end = 0;
		}
		if (end) //if end == 1
			break;//finish scheduling break from while loop
	}

	for (i = 0; i < num; i++) { // calculate AWT and ATT
		awt += processcopy[i].waitingtime; 
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;



	printf("1. FCFS \n");
	printGaunt(record, time, awt, att); //print Gaunt Chart
	
}
void SJF(){

		PROCESS* processcopy;
		processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

		for (i = 0; i < num; i++) {
			processcopy[i].id = processarr[i].id;
			processcopy[i].arrivaltime = processarr[i].arrivaltime;
			processcopy[i].cpuburst = processarr[i].cpuburst;
			processcopy[i].io = processarr[i].io;
			processcopy[i].iotime = processarr[i].iotime;
			processcopy[i].ioburst = processarr[i].ioburst;
			processcopy[i].priority = processarr[i].priority;
			processcopy[i].waitingtime = processarr[i].waitingtime;
			processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
			processcopy[i].type = processarr[i].type;
		}


		int readyqueue[20];
		int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
		int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
		int time = 0;//time
		int running = -1; //index of processes running// -1 is idle
		int lastarrival = 0;
		int end = 0; //all processes are terminated
		int record[500]; //record for Gaunt Chart
		float awt = 0;// average waiting time
		float att = 0;// average turnaround time


		for (i = 0; i < num; i++) {
			if (processcopy[i].arrivaltime > lastarrival)
				lastarrival = processcopy[i].arrivaltime;
		}

		while (1) {
			for (i = 0; i < num; i++) { //processes arrive
				if (processcopy[i].arrivaltime == time) {
					readyqueue[eoq] = processcopy[i].id;
					
				eoq++;
					}
				}
			

			if (running == -1 && (eoq > 0)) { //schedule shortest job first
				temp = 0;
				for (i = 0; i < eoq; i++) {
					if (processcopy[readyqueue[i]].cpuburst < processcopy[readyqueue[temp]].cpuburst)
						temp = i;
				}
				running = readyqueue[temp];
				for (i = temp; i < eoq - 1; i++) {
					readyqueue[i] = readyqueue[i + 1];
				}
				eoq--;

			}
			record[time] = running; //record for Gaunt Chart, -1: idle

			if (running >= 0) //decrement cpuburst
				processcopy[running].cpuburst--;

			for (i = 0; i < num; i++) { //decrement ioburst
				if (iostate[i] == 1)
					processcopy[i].ioburst--;
			}

			time++;//increment time

			for (i = 0; i < eoq; i++) //increment waiting time
				processcopy[readyqueue[i]].waitingtime++;

			if (running >= 0) {//process terminates
				if (processcopy[running].cpuburst == 0) {
					processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
					running = -1;
				}

			}

			if (running >= 0) {//I/O interrupt
				if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
					iostate[running] = 1;
					running = -1;//set running to -1
				}
			}


			for (i = 0; i < num; i++) { //I/O burst finished
				if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
					iostate[i] = 0;
					readyqueue[eoq] = processcopy[i].id;
					

					eoq++;
						}
					}
				

					if ((time > lastarrival) && (running == -1) && (eoq == 0))
						end = 1;
					for (i = 0; i < num; i++) {
						if (iostate[i] == 1)
							end = 0;
					}
					if (end)
						break;
				}
			

				for (i = 0; i < num; i++) {
					awt += processcopy[i].waitingtime;
					att += processcopy[i].turnaroundtime;
				}

				awt /= (float)num;
				att /= (float)num;

	printf("2. SJF \n");
	printGaunt(record, time, awt, att);


			
}
void Priority(){
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;

				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule high priority job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].priority < processcopy[readyqueue[temp]].priority)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart, -1: idle

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //ioburst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) //waiting time increment
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//process terminate
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;//set running to -1
			}
		}


		for (i = 0; i < num; i++) { //I/O burst finished
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;


				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0)) //finish scheduling
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("3. Priority \n");
	printGaunt(record, time, awt, att);



}

void Roundrobin(){
	int quantum = (rand() % 5) + 1;//random time quantum
	int quantumleft = quantum;//time quantum left
	
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //process arrives
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		if (running == -1 && (eoq > 0)) { 
			running = readyqueue[0];
			quantumleft = quantum;
			for (i = 0; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart, -1: idle

		if (running >= 0) { //cpuburst decrement
			processcopy[running].cpuburst--;
			quantumleft--;
		}
		for (i = 0; i < num; i++) { //ioburst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;// time increment



		for (i = 0; i < eoq; i++) // waiting time increment of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//if cpuburst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		

		if (running >= 0) {//I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;//set running to -1
			}
		}

		if ((running>=0) &&(quantumleft == 0))
		{
			readyqueue[eoq] = processcopy[running].id;
			eoq++;
			running = -1;
		}


		for (i = 0; i < num; i++) { //I/O burst finished
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				eoq++;
			}
		}

		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}

	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;




	printf("4. RoundRobin, time quantum = %d \n", quantum);
	printGaunt(record, time, awt, att);
}
void preemptiveSJF(){

	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //process arrives
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].cpuburst < processcopy[running].cpuburst) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule shortest job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].cpuburst < processcopy[readyqueue[temp]].cpuburst)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart, -1: idle

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //ioburst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//if cpu burst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].cpuburst < processcopy[running].cpuburst) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("5. Preemptive SJF \n");
	printGaunt(record, time, awt, att);



}
void preemptivePriority(){
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].priority < processcopy[running].priority) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule high priority job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].priority < processcopy[readyqueue[temp]].priority)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //I/O Burst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		if (running >= 0) {//if cpu burst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].priority < processcopy[running].priority) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}

				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("6.Preemptive Priority \n");
	printGaunt(record, time, awt, att);




}


void SJF_with_aging() {

	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time
	int age[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;

				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule shortest job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].cpuburst - age[readyqueue[i]]/5.0 < processcopy[readyqueue[temp]].cpuburst - age[readyqueue[temp]] /5.0)
					temp = i; //compare cpuburst - age/5 (age == waitingtime in readyqueue)
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //I/O Burst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) { //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;
		}

		for (i = 0; i < num; i++)
			age[i] = processcopy[i].waitingtime; //aging (age == waitingtime)

		if (running >= 0) {//if remaining cpuburst get 0, terminate process and system goes idle
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;


				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("7. SJF with Aging \n");
	printGaunt(record, time, awt, att);

}
void Priority_with_aging() {
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time
	int age[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };//age table

	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;

				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule high priority job first
			temp = 0;
			for (i = 0; i < eoq; i++) { //compare priority - age/5 (age == waitingtime in readyqueue)
				if (processcopy[readyqueue[i]].priority - age[readyqueue[i]] / 5.0 < processcopy[readyqueue[temp]].priority - age[readyqueue[temp]] / 5.0)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //I/O Burst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) { //increment waiting time of process in readyqueue
			processcopy[readyqueue[i]].waitingtime++;
		}

		for (i = 0; i < num; i++)
			age[i] = processcopy[i].waitingtime; //aging (age == waitingtime)

		if (running >= 0) {//if cpu burst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;


				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("8. Priority with Aging \n");
	printGaunt(record, time, awt, att);

}
void preemptiveSJF_with_aging() {


	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time
	int age[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].cpuburst < processcopy[running].cpuburst - age[running] / 5.0) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule shortest job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].cpuburst - age[readyqueue[i]] / 5.0 < processcopy[readyqueue[temp]].cpuburst - age[readyqueue[temp]]/5.0)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //I/O Burst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		for (i = 0; i < num; i++)
			age[i] = processcopy[i].waitingtime; //aging (age == waitingtime)

		if (running >= 0) {//if cpu burst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].cpuburst -age[readyqueue[eoq]]/5.0 < processcopy[running].cpuburst - age[running] / 5.0) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("9. Preemptive SJF with Aging \n");
	printGaunt(record, time, awt, att);




}
void preemptivePriority_with_aging() {
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int readyqueue[20];
	int eoq = 0; //next empty index of ready queue, (index of last process in ready queue) + 1,  (end of queue)
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0;//time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0;//arrival time of last process
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0;// average waiting time
	float att = 0;// average turnaround time
	int age[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //processes arrive
			if (processcopy[i].arrivaltime == time) {
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].priority < processcopy[running].priority - age[running]/5.0) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}
				eoq++;
			}
		}


		if (running == -1 && (eoq > 0)) { //schedule high priority job first
			temp = 0;
			for (i = 0; i < eoq; i++) {
				if (processcopy[readyqueue[i]].priority-age[readyqueue[i]]/5.0 < processcopy[readyqueue[temp]].priority-age[readyqueue[temp]]/5.0)
					temp = i;
			}
			running = readyqueue[temp];
			for (i = temp; i < eoq - 1; i++) {
				readyqueue[i] = readyqueue[i + 1];
			}
			eoq--;

		}
		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //cpuburst decrement
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //I/O Burst decrement
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//time increment

		for (i = 0; i < eoq; i++) //increment waiting time of processes in ready queue
			processcopy[readyqueue[i]].waitingtime++;

		for (i = 0; i < num; i++)
			age[i] = processcopy[i].waitingtime; //aging (age == waitingtime)

		if (running >= 0) {//if cpu burst gets zero, the process terminates
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to ready queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				readyqueue[eoq] = processcopy[i].id;
				if (running >= 0) {
					if (processcopy[readyqueue[eoq]].priority - age[readyqueue[eoq]]/5.0 < processcopy[running].priority - age[running]/5.0) { //preemption occurred
						swap = readyqueue[eoq];
						readyqueue[eoq] = running;
						running = swap;
					}
				}

				eoq++;
			}
		}


		if ((time > lastarrival) && (running == -1) && (eoq == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}


	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;

	printf("10.Preemptive Priority with Aging \n");
	printGaunt(record, time, awt, att);


}

void MultilevelQueue() {
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int Aqueue[20];// priority : Aqueue > Bqueue > Cqueue
	int Bqueue[20];
	int Cqueue[20];
	int eoq_A = 0; //next empty index of ready Aqueue
	int eoq_B = 0; //next empty index of ready Bqueue
	int eoq_C = 0; //next empty index of ready Cqueue
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0; //current time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0; //last arrival time
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0; // average waiting time
	float att = 0; // average turnaround time


	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //when process arrives, it goes into queue of its type (Aqueue, Bqueue, Cqueue)
			if (processcopy[i].arrivaltime == time) {
				switch (processcopy[i].type) {
				
				case 'A':
					Aqueue[eoq_A] = processcopy[i].id;
					eoq_A++; break;

				case 'B':
					Bqueue[eoq_B] = processcopy[i].id;
					eoq_B++; break;
				
				case 'C':
					Cqueue[eoq_C] = processcopy[i].id;
					eoq_C++; break;

				}
			}
		}

		if (running == -1 && (eoq_A > 0)) { //schedule processes in Aqueue first
			running = Aqueue[0];
			for (i = 0; i < eoq_A - 1; i++) {
				Aqueue[i] = Aqueue[i + 1];
			}
			eoq_A--;
		}
		else if ((running == -1) && (eoq_A == 0) && (eoq_B > 0)) {//schedule processes in Bqueue second
			running = Bqueue[0];
			for (i = 0; i < eoq_B - 1; i++) {
				Bqueue[i] = Bqueue[i + 1];
			}
			eoq_B--;
		}
		else if ((running == -1) && (eoq_A == 0) && (eoq_B == 0) &&(eoq_C >0)) {//schedule processes in Cqueue last
			running = Cqueue[0];
			for (i = 0; i < eoq_C - 1; i++) {
				Cqueue[i] = Cqueue[i + 1];
			}
			eoq_C--;
		}



		record[time] = running; //record for Gaunt Chart

		if (running >= 0) //decrement CPU burst
			processcopy[running].cpuburst--;

		for (i = 0; i < num; i++) { //decrement I/O burst
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//increment time

		for (i = 0; i < eoq_A; i++) // increment waiting time in Aqueue
			processcopy[Aqueue[i]].waitingtime++;
		for (i = 0; i < eoq_B; i++) // increment waiting time in Bqueue
			processcopy[Bqueue[i]].waitingtime++;
		for (i = 0; i < eoq_C; i++) // increment waiting time in Cqueue
			processcopy[Cqueue[i]].waitingtime++;

		if (running >= 0) {//if cpuburst gets zero, process gets terminated and system goes idle
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1,
			}
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to A,B,C queue
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				switch (processcopy[i].type) {
				case 'A':
					Aqueue[eoq_A] = processcopy[i].id;
					eoq_A++; break;

				case 'B':
					Bqueue[eoq_B] = processcopy[i].id;
					eoq_B++; break;

				case 'C':
					Cqueue[eoq_C] = processcopy[i].id;
					eoq_C++; break;

				}
			}
		}

		if ((time > lastarrival) && (running == -1) && (eoq_A == 0) && (eoq_B == 0) && (eoq_C == 0))
			end = 1; //all processes arrived && nothing is running && Nothing in 3 queues && No processes doing I/O service
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}

	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;



	printf("11. Multilevel Queue \n");
	printGaunt(record, time, awt, att);
}
void MultilevelFeedbackQueue() { // with 3 queues, whose algorithms are RR(tq=3), RR(tq=5), FCFS
	PROCESS* processcopy;
	processcopy = (PROCESS*)malloc(sizeof(PROCESS)*num);

	for (i = 0; i < num; i++) {
		processcopy[i].id = processarr[i].id;
		processcopy[i].arrivaltime = processarr[i].arrivaltime;
		processcopy[i].cpuburst = processarr[i].cpuburst;
		processcopy[i].io = processarr[i].io;
		processcopy[i].iotime = processarr[i].iotime;
		processcopy[i].ioburst = processarr[i].ioburst;
		processcopy[i].priority = processarr[i].priority;
		processcopy[i].waitingtime = processarr[i].waitingtime;
		processcopy[i].turnaroundtime = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].turnaroundtime;
		processcopy[i].type = processarr[i].type;
	}


	int queue1[20];// RR, timequantum == 3
	int queue2[20];// RR, timequantum == 5
	int queue3[20];// FCFS
	int eoq_1 = 0; //next empty index of ready queue1
	int eoq_2 = 0; //next empty index of ready queue2
	int eoq_3 = 0; //next empty index of ready queue3
	int iostate[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 0: not doing I/O services, 1: doing I/O services
	int time = 0; //current time
	int running = -1; //index of processes running// -1 is idle
	int lastarrival = 0; //last arrival time
	int end = 0; //all processes are terminated
	int record[500]; //record for Gaunt Chart
	float awt = 0; // average waiting time
	float att = 0; // average turnaround time

	int timequantum1 = 3;
	int timequantum2 = 5;
	int timequantum1left = timequantum1;
	int timequantum2left = timequantum2;

	int from; //which queue this process is from

	for (i = 0; i < num; i++) {
		if (processcopy[i].arrivaltime > lastarrival)
			lastarrival = processcopy[i].arrivaltime;
	}

	while (1) {
		for (i = 0; i < num; i++) { //when process arrives, it goes into queue1
			if (processcopy[i].arrivaltime == time) {
					queue1[eoq_1] = processcopy[i].id;
					eoq_1++; 
				}
			}

		if (running == -1 && (eoq_1 > 0)) { //schedule processes in queue1
			running = queue1[0];
			for (i = 0; i < eoq_1 - 1; i++) {
				queue1[i] = queue1[i + 1];
			}
			eoq_1--;
			from = 1;
			timequantum1left = timequantum1;
		}
		else if ((running == -1) && (eoq_1 == 0) && (eoq_2 > 0)) {//schedule processes in queue2
			running = queue2[0];
			for (i = 0; i < eoq_2 - 1; i++) {
				queue2[i] = queue2[i + 1];
			}
			eoq_2--;
			from = 2;
			timequantum2left = timequantum2;
		}
		else if ((running == -1) && (eoq_1 == 0) && (eoq_2 == 0) && (eoq_3 >0)) {//schedule processes in queue3
			running = queue3[0];
			for (i = 0; i < eoq_3 - 1; i++) {
				queue3[i] = queue3[i + 1];
			}
			eoq_3--;
			from = 3;
		}



		record[time] = running; //record for Gaunt Chart

		if (running >= 0) { //decrement CPU burst
			processcopy[running].cpuburst--;
			if (from == 1)
				timequantum1left--;
			else if (from == 2)
				timequantum2left--;
		}

		for (i = 0; i < num; i++) { //decrement I/O burst
			if (iostate[i] == 1)
				processcopy[i].ioburst--;
		}

		time++;//increment time

		for (i = 0; i < eoq_1; i++) // increment waiting time in Aqueue
			processcopy[queue1[i]].waitingtime++;
		for (i = 0; i < eoq_2; i++) // increment waiting time in Bqueue
			processcopy[queue2[i]].waitingtime++;
		for (i = 0; i < eoq_3; i++) // increment waiting time in Cqueue
			processcopy[queue3[i]].waitingtime++;

		if (running >= 0) {//if cpuburst gets zero, process gets terminated and system goes idle
			if (processcopy[running].cpuburst == 0) {
				processcopy[running].turnaroundtime = time - processcopy[running].arrivaltime;
				running = -1;
			}

		}

		if (running >= 0) {// I/O interrupt
			if ((processcopy[running].io) && (processcopy[running].iotime == processarr[running].cpuburst - processcopy[running].cpuburst)) {
				iostate[running] = 1;
				running = -1;// set running to -1,
			}
		}


		if ((running >= 0) && (timequantum1left == 0) && (from==1)) //if process from queue1 spends all timequantum
		{															//it goes to queue2
			queue2[eoq_2] = processcopy[running].id;
			eoq_2++;
			running = -1;
		}

		if ((running >= 0) && (timequantum2left == 0) && (from == 2)) //if process from queue2 spends all timequantum
		{															//it goes to queue3
			queue3[eoq_3] = processcopy[running].id;
			eoq_3++;
			running = -1;
		}


		for (i = 0; i < num; i++) { // if I/O burst gets zero, the process goes back to queue1
			if (iostate[i] == 1 && processcopy[i].ioburst == 0) {
				iostate[i] = 0;
				queue1[eoq_1] = processcopy[i].id;
				eoq_1++;

				}
			
		}

		if ((time > lastarrival) && (running == -1) && (eoq_1 == 0) && (eoq_2 == 0) && (eoq_3 == 0))
			end = 1;
		for (i = 0; i < num; i++) {
			if (iostate[i] == 1)
				end = 0;
		}
		if (end)
			break;
	}

	for (i = 0; i < num; i++) {
		awt += processcopy[i].waitingtime;
		att += processcopy[i].turnaroundtime;
	}

	awt /= (float)num;
	att /= (float)num;



	printf("12. Multilevel Feedback Queue \n");
	printGaunt(record, time, awt, att);



}



void printGaunt(int record[], int time, float awt, float att) {

	if (time <= 25) {
		for (i = 0; i < time; i++)
			printf("----");

		printf("\n");

		for (i = 0; i < time; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 0; i < time; i++)
			printf("----");

		printf("\n");


		for (i = 0; i <= time; i++)
			printf("%-4d", i);
		printf("\n");
		printf("\n");
	}

	else {
		for (i = 0; i < 25; i++)
			printf("----");

		printf("\n");

		for (i = 0; i < 25; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 0; i < 25; i++)
			printf("----");

		printf("\n");

		for (i = 0; i <= 25; i++)
			printf("%-4d", i);
		printf("\n");
		
		for (i = 25; i < time; i++)
			printf("----");
		printf("\n");
		

		for (i = 25; i < time; i++) {
			if (record[i] >= 0)
				printf(" %d |", record[i]);
			else
				printf(" i |");
		}
		printf("\n");

		for (i = 25; i < time; i++)
			printf("----");

		printf("\n");


		for (i = 25; i <= time; i++)
			printf("%-4d", i);
		printf("\n");
		printf("\n");
	}
	printf("awt: %f, att: %f", awt, att);

	printf("\n");
	printf("\n");

}