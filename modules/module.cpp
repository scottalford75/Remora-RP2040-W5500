#include "module.h"

#include <cstdio>

Module::Module()
{
}


Module::~Module(){}


void Module::runModule()
{
	this->update();
}


void Module::update(){}
void Module::configure(){}
void Module::handleInterrupt(){}
