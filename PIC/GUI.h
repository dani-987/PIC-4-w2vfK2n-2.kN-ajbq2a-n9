#pragma once

class GUI;

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

