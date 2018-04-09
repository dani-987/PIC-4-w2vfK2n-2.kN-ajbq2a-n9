#pragma once
#include "Backend.h"

class GUI
{
private:
	Backend* backend;
public:
	GUI();
	~GUI();

	int run();
};

