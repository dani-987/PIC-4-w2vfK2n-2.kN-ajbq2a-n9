#include "GUI.h"
#include <Windows.h>

#include "Compiler.h"

void printProg(HANDLE &hConsole, Backend*& b){
	size_t tmp;
	ASM_TEXT* t = b->GetProgrammText(tmp), *u = t;
	int progNum = 0xF0;
	while(u != nullptr){
		SetConsoleTextAttribute(hConsole, progNum);
		printf("%s\t%s\t%s\t%s\n", u->lineOfCode, u->label, u->asmCode, u->comment);
		progNum ^= 0x08;
		u = u->next;
	}
	b->FreeProgrammText(t);
	SetConsoleTextAttribute(hConsole, 0x07);
}

void printProgramRam(Backend* b){
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for(int i = 0; i < 2;i++){
		SetConsoleTextAttribute(hConsole, 0x0A);
		printf("Bank %d:\n",i);
		SetConsoleTextAttribute(hConsole, 0xF0);
		printf("     08  19  2A  3B  4C  5D  6E  7F \n");
		for(int j = 0; j < UC_SIZE_RAM; j+=8){
			SetConsoleTextAttribute(hConsole, 0xF0);
			printf(" %02x ",j);
			SetConsoleTextAttribute(hConsole, 0x70);
			printf(" %02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x \n",
				b->GetByte(j,i),b->GetByte(j+1,i),b->GetByte(j+2,i),b->GetByte(j+3,i),
				b->GetByte(j+4,i),b->GetByte(j+5,i),b->GetByte(j+6,i),b->GetByte(j+7,i));
		}
		SetConsoleTextAttribute(hConsole, 0x07);
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
	//getestet: 1,2,3,4,5,6,7,8,9,10,12
	// use '//*' in folowing line for testing the backend, use '/*' for running programm with GUI
	/*
	int progNum = 0;
	char input = 0;
	char path[257];
	int val = 0;
	size_t tmp;
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	Backend* b = new Backend(new GUI());
	ASM_TEXT*u = nullptr;
	do{
		printf("\nSelect Program (int)... \n");
		scanf("%d", &progNum);
		fseek(stdin,0,SEEK_END);
		sprintf_s(path, 256, "..\\Testprogramme\\TPicSim%d.LST", progNum);
		printf("\nOpening '%s'...\n", path);
		if(b->LoadProgramm(path)){
			printProg(hConsole, b);
			printf("\n\nopened!\n");
			do{
				printf("Input command (b:breakpoint,s:start,o:one-step,w:setW,r:setRam,e:exitAndLoadNewProgramm,p:printProgram,t:toggleWDT,a:stepAndTestRam,d:speed):\n");
				fseek(stdin,0,SEEK_END);
				scanf("%c", &input);
				switch(input){
				case 'e': case'E':
					printf("exiting actuel program!...\n");
					input = 'e';
					break;
				case 'd': case'D':
					printf("Set execution speed in 100ns (int<4-INT_MAX>):\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &progNum);
					if(progNum >= 4){
						b->SetCommandSpeed(progNum);
						printf("Settet\n");
					}
					else printf("Input ignored...\n");
					break;
				case 'a': case'A':
					printf("Doing Step in program...\n");
					SetConsoleTextAttribute(hConsole, 0xF0);
					u = b->get_ASM_ONLY_TESTING()->code[(b->GetByte(0x0A,0) << 8) | b->GetByte(0x02,0)].guiText;
					printf("%s\t%s\t%s\t%s\n", u->lineOfCode, u->label, u->asmCode, u->comment);
					SetConsoleTextAttribute(hConsole, 0x07);
					if(!b->Step()){
						printf("Cannot Step!\n");
						b->Stop();
						b->Wait_For_End();
						printf("Error: %s\n", b->GetErrorMSG());
					}
					b->Wait_For_End();
					printProgramRam(b);
					break;
				case 't': case'T':
					if(b->IsWatchdogEnabled())b->DisableWatchdog(),printf("Toggeling WDT... WDT is now inactive\n");
					else b->EnableWatchdog(),printf("Toggeling WDT... WDT is now active\n");
					break;
				case 'b': case'B':
					printf("Set/Unset Breakpoint! Insert line (int):\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &progNum);
					progNum = b->ToggleBreakpoint(progNum - 1);
					printf("Breakpoint settet to : %d\n\t(>= 0:linenumber, -1:error, -2:breakpoint unsettet, -3 breakpoint unchanged)\n\n", (progNum < 0)?progNum:progNum+ 1);
					break;
				case 's' :case 'S':
					printf("Starting program...");
					if(!b->Start()){
						printf("Cannot Start!\n");
						b->Stop();
						b->Wait_For_End();
						printf("Error: %s\n", b->GetErrorMSG());
					}
					b->Wait_For_End();
					break;
				case 'o' :case 'O': case '\n':
					printf("Doing Step in program...\n");
					SetConsoleTextAttribute(hConsole, 0xF0);
					u = b->get_ASM_ONLY_TESTING()->code[(b->GetByte(0x0A,0) << 8) | b->GetByte(0x02,0)].guiText;
					printf("%s\t%s\t%s\t%s\n", u->lineOfCode, u->label, u->asmCode, u->comment);
					SetConsoleTextAttribute(hConsole, 0x07);
					if(!b->Step()){
						printf("Cannot Step!\n");
						b->Stop();
						b->Wait_For_End();
						printf("Error: %s\n", b->GetErrorMSG());
					}
					b->Wait_For_End();
					break;
				case 'r': case'R':
					printf("Select Ram Position (int [hex]):\n");
					fseek(stdin,0,SEEK_END);
					scanf("%x", &progNum);
					if(progNum < 0 || progNum > UC_SIZE_RAM){
						printf("Position is not in ram!");
						break;
					}
					printf("\nInsert Value (Reg: 0x%02x, Bank: 0x%02x) (byte [hex]):\n", (progNum%82), (progNum/82));
					fseek(stdin,0,SEEK_END);
					scanf("%x", &val);
					printf("Setting...\n");
					b->SetByte((progNum%82), (progNum/82), val&0xFF);
					break;
				case 'w': case'W':
					printf("\nInsert Value:\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &progNum);
					printf("Setting...\n");
					b->SetRegW(progNum & 0xFF);
					break;
				case 'p': case 'P':
					printProg(hConsole, b);
					printf("\n");
					break;
				default:
					printf("'%c' is not supportet!\n", input);
					break;
				}
			}while(input != 'e');
		}
		else{
			printf("Cannot open!\n%s", b->GetErrorMSG());
		}
	}while(1);
	return true;/*/
	Fl::lock();

	GUI* gui = new GUI();
	return gui->run();
	//*/
}