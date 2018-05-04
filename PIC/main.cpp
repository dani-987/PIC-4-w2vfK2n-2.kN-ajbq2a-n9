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

int main(int argc, char *argv[]) {
	// use '//*' in folowing line for testing the backend, use '/*' for running programm with GUI
	char path[257];
	int progNum = 1;
	Backend* b = new Backend(new GUI());
	while(1){
		for(progNum = 1; progNum < 13; progNum++){
			sprintf_s(path, 256, "..\\Debug\\Testprogramme\\TPicSim%d.LST", progNum);
			b->LoadProgramm(path);
		}
	}
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
		sprintf_s(path, 256, "..\\Debug\\Testprogramme\\TPicSim%d.LST", progNum);
		printf("\nOpening '%s'...\n", path);
		if(b->LoadProgramm(path)){
			printProg(hConsole, b);
			printf("\n\nopened!\n");
			do{
				printf("Input command (b:breakpoint,s:start,o:one-step,w:setW,r:setRam,e:exitAndLoadNewProgramm,p:printProgram,t:toggleWDT):\n");
				fseek(stdin,0,SEEK_END);
				scanf("%c", &input);
				switch(input){
				case 'e': case'E':
					printf("exiting actuel program!...\n");
					input = 'e';
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
	return true;/* /
	Fl::lock();

	GUI* gui = new GUI();
	return gui->run();
	//*/
}