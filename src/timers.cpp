/*
 * BerylDB - A lightweight database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2021 - Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include "beryl.h"

void Timer::SetNextTick(unsigned int tick)
{
	Kernel->Tickers->Remove(this);
	secs = tick;
	SetSchedule(Kernel->Now() + tick);
	Kernel->Tickers->Add(this);
}

Timer::Timer(unsigned int add_secs, bool repeating) : schedule(Kernel->Now() + add_secs)
						    , secs(add_secs)
						    , repeat(repeating)
{

}

Timer::~Timer()
{
	Kernel->Tickers->Remove(this);
}

void TickManager::Flush(time_t TIME)
{
	for (TimerMap::iterator i = Timers.begin(); i != Timers.end(); )
	{
		Timer* timer = i->second;

		/* We must break if no pending timers are present. */
		
		if (timer->GetSchedule() > TIME)
		{
			break;
		}

		/* About to tick a timer, we must remove it. */
		
		Timers.erase(i++);

		if (!timer->Run(TIME))
		{
			continue;
		}

		/* We add the timer again if Run() returned true. */
		
		if (timer->GetRepeat())
		{
			timer->SetSchedule(TIME + timer->as_interval());
			this->Add(timer);
		}
	}
}

void TickManager::Remove(Timer* timer)
{
	std::pair<TimerMap::iterator, TimerMap::iterator> itpair = Timers.equal_range(timer->GetSchedule());

	for (TimerMap::iterator i = itpair.first; i != itpair.second; ++i)
	{
		if (i->second == timer)
		{
			this->Timers.erase(i);
			break;
		}
	}
}

void TickManager::Add(Timer* timer)
{
	this->Timers.insert(std::make_pair(timer->GetSchedule(), timer));
}

