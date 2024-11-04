
#include "stepgen.h"
#include "../remora.h"
//#include "../boardconfig.h"


// If undef then we use bresenham
#undef USE_FLOAT_DDS

#define USE_FAST_PIN_SETTING

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

void createStepgen()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    int joint = module["Joint Number"];
    const char* step = module["Step Pin"];
    const char* dir = module["Direction Pin"];

    // configure pointers to data source and feedback location
    //ptrJointFreqCmd[joint] = &rxData.jointFreqCmd[joint];
    //ptrJointFeedback[joint] = &txData.jointFeedback[joint];
    //ptrJointEnable = &rxData.jointEnable;

    // create the step generator, register it in the thread
    Module* stepgen = new Stepgen(base_freq, joint, step, dir, STEPBIT);
    baseThread->registerModule(stepgen);
    baseThread->registerModulePost(stepgen);
}


/***********************************************************************
    MODULE CONFIGURATION AND CREATION FROM STATIC CONFIG - boardconfi.h   
************************************************************************/


// Here is some static data so we can just aggregate all
// the step gen in one loop.
static int32_t staticThreadFreq;
static int32_t all_step_bits = 0;
struct Flat {
	int32_t D;
	int32_t mask;  // this is mask in remora's tx/rx structure
	int32_t direction_mask;
	int32_t step_mask; 
	Pin *directionPin, *stepPin;
	int32_t count;
	int32_t jointNumber;
	int16_t skip;
	int16_t direction;
};

// Array of static data
static Flat flat_data[8];
static int nsteppers = 0;

void Stepgen::consolidated_stepgen()
{ 
	auto* rxData = getCurrentRxBuffer(&rxPingPongBuffer);
	auto* txData = getCurrentTxBuffer(&txPingPongBuffer);

#ifdef USE_FAST_PIN_SETTING
	// This uses gpio_clr_mask and gpio_set_mask 
	uint32_t step_and_dir_mask=0;
	uint32_t dir_reset_mask=0;

	for(int stepper_idx = 0; stepper_idx < nsteppers; stepper_idx++) {
		Flat& data =  flat_data[stepper_idx];
		if((rxData->jointEnable & data.mask) != 0) {
			int32_t freq = rxData->jointFreqCmd[data.jointNumber];

			bool freqIsNegative = freq < 0;
			// Check if the direction is new, normalize frequency
			int16_t newDirection = 1;
			if(freqIsNegative){
				newDirection = -1;
				freq = -freq;
			}
			// If direction is new skip 2 step intervals and update the gpio
			if(newDirection != data.direction) {
				data.direction = newDirection;
				data.skip = 2;
				data.directionPin->set(newDirection > 0);
			}
			// Bresenham
			int32_t dy = freq;
			int32_t dx = staticThreadFreq;
			
			if(data.skip > 0) {
				// skipping this iteration, update skip iteration count
				data.skip--;
			} else {
				if(data.D > 0) {
					// Bresenham state has hit a step threshold
					data.D -= 2 * dx;
					step_and_dir_mask |= data.step_mask;
					// UPdate count
					data.count += data.direction;	
					txData->jointFeedback[data.jointNumber] = data.count;	
				}
				data.D += 2 * dy;
			}
		}
	}
	// TODO: is this enough direction settling time? I think direction
	// needs to have settled before 
	gpio_set_mask(step_and_dir_mask);
	// busy wait
	// seems to yield 2.5us of step pulse
	for(volatile int i = 0 ; i < 32; i++) ;
	
	gpio_clr_mask(all_step_bits);
	//for(int stepper_idx = 0; stepper_idx < nsteppers;stepper_idx++) {
	//	flat_data[stepper_idx].stepPin->set(false);
	//}


#else
	// This just sets pins as fast as possible individually.
	// Probably delete.
	for(int stepper_idx = 0; stepper_idx < nsteppers; stepper_idx++) {
		Flat& data =  flat_data[stepper_idx];
		if((rxData->jointEnable & data.mask) != 0) {
			int32_t freq = rxData->jointFreqCmd[data.jointNumber];
			bool isForward = true;
			if(freq < 0){
				isForward = false;
				freq = -freq;
			}
			int32_t dy = freq;
			int32_t dx = staticThreadFreq;
			if(data.D > 0) {
				// Update bresenham state.
				data.D -= 2 * dx;
				// Set pins
				data.directionPin->set(isForward);
				data.stepPin->set(true);
				// Update count
				data.count += isForward ? 1 : -1;
				txData->jointFeedback[data.jointNumber] = data.count;	
			}
			data.D += 2 * dy;
		}
	}
	
	for(int stepper_idx = 0; stepper_idx < nsteppers;stepper_idx++) {
		flat_data[stepper_idx].stepPin->set(false);
	}

#endif
}

/***********************************************************************
                METHOD DEFINITIONS
************************************************************************/

Stepgen::Stepgen(int32_t threadFreq, int jointNumber, std::string step, std::string direction, int stepBit) :
	jointNumber(jointNumber),
	step(step),
	direction(direction),
	stepBit(stepBit)
{
	staticThreadFreq = threadFreq;
	this->stepPin = new Pin(this->step, OUTPUT);
	this->directionPin = new Pin(this->direction, OUTPUT);
	this->DDSaccumulator = 0;
	this->rawCount = 0;
	this->frequencyScale = (float)(1 << this->stepBit) / (float)threadFreq;
	this->mask = 1 << this->jointNumber;
	this->isEnabled = false;
	this->isForward = false;
	this->threadFreq = threadFreq;
	this->bresenhamD = 0;

	Flat& flat = flat_data[nsteppers];
	flat.D = 0;
	flat.step_mask = 1 << stepPin->getPinNumber();
	flat.direction_mask = 1 << directionPin->getPinNumber();
	flat.mask = mask;
	flat.stepPin = stepPin;
	flat.directionPin = directionPin;
	flat.jointNumber = jointNumber;
	flat.count = 0;
	flat.skip = 0;
	flat.direction = 0;
	all_step_bits |= flat.step_mask;
	nsteppers++;
	printf("adding joint %d nsteppers now %d\n", jointNumber, nsteppers);
}


void Stepgen::update()
{
	// Use the standard Module interface to run makePulses()
	this->makePulses();
}

void Stepgen::updatePost()
{
}

void Stepgen::slowUpdate()
{
	return;
}

void Stepgen::makePulses()
{
	int32_t stepNow = 0;

	rxData = getCurrentRxBuffer(&rxPingPongBuffer);
	txData = getCurrentTxBuffer(&txPingPongBuffer);

	this->isEnabled = ((rxData->jointEnable & this->mask) != 0);

#ifdef USE_FLOAT_DDS
	// Here is the old code
	if (this->isEnabled == true)  												// this Step generator is enables so make the pulses
	{
		this->frequencyCommand = rxData->jointFreqCmd[this->jointNumber];             // Get the latest frequency command via pointer to the data source
		this->DDSaddValue = this->frequencyCommand * this->frequencyScale;		// Scale the frequency command to get the DDS add value
		stepNow = this->DDSaccumulator;                           				// Save the current DDS accumulator value
		this->DDSaccumulator += this->DDSaddValue;           	  				// Update the DDS accumulator with the new add value
		stepNow ^= this->DDSaccumulator;                          				// Test for changes in the low half of the DDS accumulator
		stepNow &= (1L << this->stepBit);                         				// Check for the step bit
		//this->rawCount = this->DDSaccumulator >> this->stepBit;   				// Update the position raw count

		if (this->DDSaddValue > 0)												// The sign of the DDS add value indicates the desired direction
		{
			this->isForward = true;
		}
		else
		{
			this->isForward = false;
		}

		if (stepNow)
		{
			this->directionPin->set(this->isForward);             		    	// Set direction pin
			this->stepPin->set(true);											// Raise step pin
			if (this->isForward)
            {
                ++this->rawCount;
            }
            else
            {
                --this->rawCount;
            }
            txData->jointFeedback[this->jointNumber] = this->rawCount;	
		}
	}
#else
	if(this->isEnabled) {
		int32_t freq = rxData->jointFreqCmd[this->jointNumber];
		if(freq < 0){
			isForward = false;
			freq = -freq;
		} else {
			isForward = true;
		}
		int32_t dy = freq;
		int32_t dx = threadFreq;
		if(bresenhamD > 0) {
			// Update bresenham state.
			bresenhamD -= 2 * dx;
			// Set pins
			directionPin->set(isForward);
			stepPin->set(true);
			// Update count
			rawCount += isForward ? 1 : -1;
			txData->jointFeedback[this->jointNumber] = this->rawCount;	
		}
		bresenhamD += 2 * dy;

	}
#endif



}


void Stepgen::stopPulses()
{
	this->stepPin->set(false);	// Reset step pin
}


void Stepgen::setEnabled(bool state)
{
	this->isEnabled = state;
}
