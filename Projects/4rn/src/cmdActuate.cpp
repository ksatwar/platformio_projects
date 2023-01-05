#include "cmdActuate.h"
bool cmdActuate(char cmd){
    bool retval=true;
    switch (cmd)
    {
        case 'r':
        ESP.restart();
        break;
        default:
        retval=false;
    }
    return retval;
}