#include "stepgen.h"

/***********************************************************************
                MODULE CONFIGURATION AND CREATION FROM JSON     
************************************************************************/

Stepgen *createStepgen()
{
    const char* comment = module["Comment"];
    printf("\n%s\n",comment);

    int joint = module["Joint Number"];
    const char* step = module["Step Pin"];
    const char* dir = module["Direction Pin"];

    float steplen = module["steplen"];
    float stepspace = module["stepspace"];
    float dirsetup = module["dirsetup"];
    float dirhold = module["dirhold"];
    float dirdelay = module["dirdelay"];

    // configure pointers to data source and feedback location
    //ptrJointFreqCmd[joint] = &rxData.jointFreqCmd[joint];
    //ptrJointFeedback[joint] = &txData.jointFeedback[joint];
    //ptrJointEnable = &rxData.jointEnable;

    // create the step generator, register it in the thread
    Stepgen* stepgen = new Stepgen(
        base_freq, joint, step, dir, steplen, stepspace,
        dirsetup, dirhold, dirdelay
    );
    baseThread->registerModule(stepgen);
    return stepgen;
}
