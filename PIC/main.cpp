#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	// use '//*' in folowing line for testing the backend, use '/*' for running programm with GUI
	//*
	Backend* b = new Backend(new GUI());
	b->LoadProgramm("..\\Debug\\Testprogramme\\TPicSim3.LST");
	while (1) {
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