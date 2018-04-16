#include "GUI.h"

#include "Compiler.h"

int main(int argc, char *argv[]) {
	GUI* gui = new GUI();
	return gui->run();
}