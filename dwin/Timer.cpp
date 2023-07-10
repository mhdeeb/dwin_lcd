// Date: 10/FEB/2022
// Path: Fryer_App/Timer.cpp
// Link: https://github.com/mhdeeb/Fryer_App

#include "Timer.h"

#include <Arduino.h>

Timer::Timer(u32 initialValue) : counter(0, initialValue, initialValue, 1, false), lastUpdateTime(millis())
{
    Set(initialValue);
}

void Timer::Set(u32 value)
{
    counter.SetMax(value);
    counter.SetValue(value);

    isChanged = true;
}

void Timer::Start()
{
    isRunning = true;
    isFinished = false;
}

void Timer::Stop()
{
    isRunning = false;
    isFinished = false;
}

void Timer::Reset()
{
    Set(counter.GetMax());
    isRunning = false;
    isFinished = false;
}

void Timer::SetUpdateDelay(u32 updateDelay) { this->updateDelay = updateDelay; }

void Timer::Update()
{
    if (isRunning && (millis() - lastUpdateTime) >= updateDelay)
    {
        counter.Decrement();

        if (counter.GetValue() == counter.GetMin())
        {
            isRunning = false;
            isFinished = true;
        }

        lastUpdateTime = millis();

        isChanged = true;
    }
}
bool Timer::IsRunning() const { return isRunning; }

bool Timer::IsFinished() const { return isFinished; }

bool Timer::PopChanged()
{
    bool temp = isChanged;
    isChanged = false;
    return temp;
}

u32 Timer::GetTime() const { return counter.GetValue(); }