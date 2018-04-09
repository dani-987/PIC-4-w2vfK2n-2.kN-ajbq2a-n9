#include "GUI.h"



GUI::GUI()
{
	backend = new Backend();
}


GUI::~GUI()
{
}

int GUI::run()
{
	backend->GetBit(0, 0);
	while (1) {
		char* c = backend->getErrorMSG();
		printf("%s\n", c);
		free(c);
	}
	return 0;
}
