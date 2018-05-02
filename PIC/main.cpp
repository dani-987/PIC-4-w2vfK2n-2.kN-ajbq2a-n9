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
	/*
	int progNum = 0;
	char input = 0;
	char path[257];
	int val = 0;
	size_t tmp;
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	Backend* b = new Backend(new GUI());
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
				printf("Input command (b:breakpoint,s:start,o:one-step,w:setW,r:setRam,e:exitAndLoadNewProgramm,p:printProgram):\n");
				fseek(stdin,0,SEEK_END);
				scanf("%c", &input);
				switch(input){
				case 'e': case'E':
					printf("exiting actuel program!...\n");
					input = 'e';
					break;
				case 'b': case'B':
					printf("Set/Unset Breakpoint! Insert line (int):\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &progNum);
					progNum = b->ToggleBreakpoint(progNum);
					printf("\nBreakpoint settet to : %d (>= 0:linenumber, -1:error, -2:breakpoint unsettet, -3 breakpoint unchanged)", (progNum >= 0)?progNum+1:progNum);
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
				case 'o' :case 'O':
					printf("Doing Step in program...");
					if(!b->Step()){
						printf("Cannot Step!\n");
						b->Stop();
						b->Wait_For_End();
						printf("Error: %s\n", b->GetErrorMSG());
					}
					b->Wait_For_End();
					break;
				case 'r': case'R':
					printf("Select Ram Position (int):\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &progNum);
					if(progNum < 0 || progNum > UC_SIZE_RAM){
						printf("Position is not in ram!");
						break;
					}
					printf("\nInsert Value:\n");
					fseek(stdin,0,SEEK_END);
					scanf("%d", &val);
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