#ifndef PRUTHREAD_H
#define PRUTHREAD_H

#include "timer.h"

// Standard Template Library (STL) includes
#include <vector>

using namespace std;

class Module;

class pruThread
{

	private:

		pruTimer* 		    TimerPtr;
	
		uint8_t				slice;
		uint32_t 			frequency;

		bool hasThreadPost;		// run updatePost() vector

		vector<Module*> vThread;		// vector containing pointers to Thread modules
		vector<Module*> vThreadPost;		// vector containing pointers to Thread modules that run after the main vector modules
		vector<Module*>::iterator iter;

	public:

		bool				semaphore;
		bool				execute;

		pruThread(uint8_t slice, uint32_t frequency);

		void registerModule(Module *module);
		void registerModulePost(Module *module);
		void startThread(void);
        void stopThread(void);
		void run(void);
};

#endif

