#include <cstdio>

#include "pico/stdlib.h"

#include "pruThread.h"
#include "../modules/module.h"


using namespace std;

// Thread constructor
pruThread::pruThread(uint8_t slice, uint32_t frequency) :
	slice(slice),
	frequency(frequency)
{
	printf("Creating thread %d\n", this->frequency);
	
	if (this->slice == 0){
		gpio_init(6);
		gpio_set_dir(6, 1);
	}

	if (this->slice == 1){
		gpio_init(27);
		gpio_set_dir(27, 1);
	}	

	this->semaphore = false;
	this->execute = false;
}

void pruThread::startThread(void)
{
	TimerPtr = new pruTimer(this->slice, this->frequency, this);
}

void pruThread::stopThread(void)
{
    this->TimerPtr->stopTimer();
}


void pruThread::registerModule(Module* module)
{
	this->vThread.push_back(module);
}


void pruThread::registerModulePost(Module* module)
{
	this->vThreadPost.push_back(module);
	this->hasThreadPost = true;
}


void pruThread::run(void)
{

	if(!this->execute)
		return;	
	
	while (this->semaphore == true);	
		this->semaphore = true;	
	
	if (this->slice == 0){
		gpio_put(6, 1);
	}

	if (this->slice == 1){
		gpio_put(27, 1);
	}

	// iterate over the Thread pointer vector to run all instances of Module::runModule()
	for (iter = vThread.begin(); iter != vThread.end(); ++iter) (*iter)->runModule();

	// iterate over the second vector that contains module pointers to run after (post) the main vector
	if (hasThreadPost)
	{
		for (iter = vThreadPost.begin(); iter != vThreadPost.end(); ++iter) (*iter)->runModulePost();
	}
	
	if (this->slice == 0){
		gpio_put(6, 0);
	} 

	if (this->slice == 1){
		gpio_put(27, 0);
	}

	this->execute = false;
	this->semaphore = false;
}
