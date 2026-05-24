#ifndef MODULE_H
#define MODULE_H

#include <cstdint>

// Module base class
// All modules are derived from this base class

class Module
{
	public:
		Module();					// constructor to run the module at the thread frequency

		virtual ~Module();
		void runModule();			// the standard interface that the thread runs at the thread frequency, this calls update() at the module frequency
		virtual void update();		// the standard interface for update of the module - use for stepgen, PWM etc
        virtual void configure();   // the standard interface for one off configuration
        virtual void handleInterrupt();

};

#endif

