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

#pragma once

class Module;

/* 
 * This timer class will run timers that had been added through the
 * TickManager class. In order to create a timer, you must
 * inherit a Timer class. 
 */
 
class ExportAPI Timer
{ 
    private:
		
	/* Time at which a given timer is scheduled to be executed. */
		
	time_t schedule;

	/* Secs between intervals. */
	
	unsigned int secs;

	/* Set to true when a timer is repeating. */
		
	bool repeat;

   public:

	/* Constructs a timer. */
		
	Timer(unsigned int add_secs, bool repeating = false);

	/* Destructor. */
	
	virtual ~Timer();

        /* 
         * Get current schedule.
	 * 
         * @return:
 	 *
         *         · time_t: Next execution.
         */    
         
	time_t GetSchedule() const
	{
		return this->schedule;
	}

        /* 
         * Sets a new schedule for a timer.
         * 
         * @parameters:
	 *
	 *         · time_t	: Next schedule.
         */    
         
	void SetSchedule(time_t nextschedule)
	{
		schedule = nextschedule;
	}

	void SetNextTick(unsigned int tick);

	/* 
	 * Run() ticks a timer. This function must return
	 * a false or true depending whether you want it to be run again.
	 */

	 virtual bool Run(time_t TIME) = 0;

	 /* Set to true when a timer is meant to repeat. */
	 
	 bool GetRepeat() const
	 {
		return repeat;
	 }

        /* 
         * Returns seconds between intervals.
         * 
         * @return:
 	 *
         *         · uint	: Seconds between this Run()
         *                 	  and the next one.
         */    		
         
 	 unsigned int as_interval() const
 	 {
		return this->secs;
	 }

	/* Stops repeating a timer. */
	
	void StopRepeating()
	{
		repeat = false;
	}
};

class ExportAPI TickManager : public safecast<TickManager>
{
  private:
  
  	/* Timer map structure */
  	
	typedef std::multimap<time_t, Timer*> TimerMap;

	/* Pending timers. */
	
	TimerMap Timers;

  public:
	
	/* Flushes all pending timers. */
	
	void Flush(time_t TIME);

	/* 
	 * Adds a new timer to TimerMap.
         * 
         * @parameters:
	 *
	 *         · Timer	: Class inherting timer that will be added
	 *            	          to pending timers.
         */    
	
	void Add(Timer* timer);

        /* 
         * Removes a timer.
         * 
         * @parameters:
	 *
	 *         · Timer	: Timer to remove.
         */    	
         
	void Remove(Timer* timer);

        /* 
         * Counts pending timers.
         * 
         * @return:
 	 *
         *         · uint	: Timers' size.
         */    
         	
	unsigned int Count()
	{
		return this->Timers.size();
	}
	
	/* Reset al timers. */
	
	void Reset()
	{
		this->Timers.clear();
	}
};
