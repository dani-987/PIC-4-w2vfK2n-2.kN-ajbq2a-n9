#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	GUI* gui = new GUI();
	Fl::lock();
	return gui->run();
}