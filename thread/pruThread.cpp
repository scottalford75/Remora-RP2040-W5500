#include <cstdio>

#include "pico/stdlib.h"

#include "pruThread.h"
#include "../modules/module.h"
#include "../modules/stepgen/stepgen.h"
#include "../modules/digitalPin/digitalPin.h"
#include "../modules/blink/blink.h"

constexpr int SLICE0_PIN = 6;
constexpr int SLICE1_PIN = 27;

// Put pins high when slice0 and slice1 are running
#define USE_TIMER_PINS

using namespace std;

// Thread constructor
pruThread::pruThread(uint8_t slice, uint32_t frequency) :
	slice(slice),
	frequency(frequency)
{
	printf("Creating thread %d\n", this->frequency);
	
#ifdef USE_TIMER_PINS
	if (this->slice == 0){
		gpio_init(SLICE0_PIN);
		gpio_set_dir(SLICE0_PIN, 1);
	}

	if (this->slice == 1){
		gpio_init(SLICE1_PIN);
		gpio_set_dir(SLICE1_PIN, 1);
	}	
#endif

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
	
#ifdef USE_TIMER_PINS
	if (this->slice == 0){
		gpio_put(SLICE0_PIN, 1);
	}

	if (this->slice == 1){
		gpio_put(SLICE1_PIN, 1);
	}
#endif

	if(slice == 1 || false) {
		DigitalPin::singleton().update();
		Blink::singleton().update();
		// iterate over the Thread pointer vector to run all instances of Module::runModule()
		for (iter = vThread.begin(); iter != vThread.end(); ++iter) (*iter)->runModule();
		/*
		// iterate over the second vector that contains module pointers to run after (post) the main vector
		if (hasThreadPost)
		{
			for (iter = vThreadPost.begin(); iter != vThreadPost.end(); ++iter) (*iter)->runModulePost();
		}
		*/
	} else {
		Stepgen::consolidated_stepgen();
	}
	
#ifdef USE_TIMER_PINS
	if (this->slice == 0){
		gpio_put(SLICE0_PIN, 0);
	} 

	if (this->slice == 1){
		gpio_put(SLICE1_PIN, 0);
	}
#endif

	this->execute = false;
	this->semaphore = false;
}

