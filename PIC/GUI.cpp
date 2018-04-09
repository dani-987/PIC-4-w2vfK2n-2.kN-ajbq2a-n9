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
		printf(backend->getErrorMSG());
	}
	return 0;
}
