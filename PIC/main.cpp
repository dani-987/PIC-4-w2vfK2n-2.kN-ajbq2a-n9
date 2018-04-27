#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	// use '//*' in folowing line for testing the backend, use '/*' for running programm with GUI
	/*
	Backend* b = new Backend(new GUI());
	b->set_DEBUG_ONLY_TESTING(0);
	b->setCommandSpeed(1000);
	b->LoadProgramm("..\\Debug\\Testprogramme\\TPicSim8.LST");
	//prog 7
	//printf("Set Breakpoint returns: %04d, %04d\n",(int)(b->ToggleBreakpoint(40)+1),(int)(b->ToggleBreakpoint(55)+1));
	//prog8
	printf("Set Breakpoint returns: %04d, %04d, %04d, %04d, %04d\n",(int)(b->ToggleBreakpoint(72)+1),(int)(b->ToggleBreakpoint(31)+1),(int)(b->ToggleBreakpoint(39)+1),(int)(b->ToggleBreakpoint(47)+1),(int)(b->ToggleBreakpoint(53)+1));
	system("pause");
	b->set_DEBUG_ONLY_TESTING(5);
	while (1) {
		b->Start();
		b->Wait_For_End();
		system("pause");
	}
	return true;/*/
	Fl::lock();

	GUI* gui = new GUI();
	return gui->run();
	//*/
}