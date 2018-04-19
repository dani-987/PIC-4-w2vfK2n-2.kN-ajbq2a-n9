#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	/*
	Backend* b = new Backend(new GUI());
	b->LoadProgramm("..\\Debug\\Testprogramme\\TPicSim1.LST");
	while (1) {
		b->Step();
		b->Wait_For_End();
		system("pause");
	}
	return true;/*/
	GUI* gui = new GUI();
	return gui->run();
	//*/
}