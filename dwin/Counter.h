// Date: 10/FEB/2022
// Path: Fryer_App/Counter.h
// Link: https://github.com/mhdeeb/Fryer_App

#ifndef COUNTER_H
#define COUNTER_H

#include "def.h"

class Counter
{
private:
    u32 value;
    u32 max;
    u32 min;
    u32 step;
    bool isCycleable;
    bool isLocked = false;

public:
    Counter(u32 min, u32 max, u32 initialValue = 0, u32 step = 1, bool isCycleable = true);

    void SetValue(u32 value);

    u32 GetValue() const;

    void SetMax(u32 max);

    u32 GetMax() const;

    void SetMin(u32 min);

    u32 GetMin() const;

    void SetStep(u32 step);

    u32 GetStep() const;

    void SetCycleable(bool isCycleable);

    bool IsCycleable() const;

    void Increment();

    void Decrement();

    void Reset(u8 value = 0);

    void Lock();

    void Unlock();
};

#endif // COUNTER_H
