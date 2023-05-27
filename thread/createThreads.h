#include "../extern.h"

void createThreads(void)
{
    baseThread = new pruThread(BASE_SLICE, base_freq);

    servoThread = new pruThread(SERVO_SLICE , servo_freq);
}
