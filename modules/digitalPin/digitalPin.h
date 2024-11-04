#ifndef DIGITALPIN_H
#define DIGITALPIN_H

#include <cstdint>

#include "../extern.h"
#include "../module.h"
#include "../../drivers/pin/pin.h"



void createDigitalPin(void);
void loadStaticIO(void);

class DigitalPin : public Module
{
	private:
		static constexpr int MAXPINS = 32;
		struct Item {
			int bitNumber;				// location in the data source
			bool invert;
			int mask;
			int mode;
			int modifier;
			Pin *pin;
		};
		Item items[MAXPINS];
		int npins = 0;


	public:
        void addPin(int, std::string, int, bool, int);
		virtual void update(void);
		virtual void slowUpdate(void);
		static DigitalPin& singleton();
};

#endif
