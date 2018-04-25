#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	// use '//*' in folowing line for testing the backend, use '/*' for running programm with GUI
	//*
	Backend* b = new Backend(new GUI());
	b->set_DEBUG_ONLY_TESTING(0);
	b->setCommandSpeed(10);
	b->LoadProgramm("..\\Debug\\Testprogramme\\TPicSim7.LST");
	printf("Set Breakpoint returns: %04d\n",b->ToggleBreakpoint(40)+1);
	printf("Set Breakpoint returns: %04d\n",b->ToggleBreakpoint(55)+1);
	system("pause");
	b->get_ASM_ONLY_TESTING()->code[0x04].param1 = 0;
	b->Start();
	b->Wait_For_End();
	system("pause");
	b->Start();
	b->Wait_For_End();
	system("pause");
	b->set_DEBUG_ONLY_TESTING(4);
	while (1) {
		b->SetBit(6,0,0,1);
		b->Step();
		b->Wait_For_End();
		system("pause");
		b->SetBit(6,0,0,0);
		b->Step();
		b->Wait_For_End();
		system("pause");
	}
	return true;/*/
	Fl::lock();

	GUI* gui = new GUI();
	return gui->run();
	//*/
}