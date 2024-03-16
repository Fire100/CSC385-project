#ifndef MAINQUEUE
#define MAINQUEUE

#include "mbed.h"
#include "USBAudio.h"

extern EventQueue mainQueue;
extern USBAudio* audio;
const bool CLIENT = true;

#endif // MAINQUEUE
