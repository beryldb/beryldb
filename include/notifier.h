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

enum NOTIFY_LEVEL
{
      NOTIFY_DEBUG     =  10,		/* Notification of detailed events */  
      NOTIFY_VERBOSE   =  20,		/* Verbose notifications */
      NOTIFY_DEFAULT   =  30		/* The most basic level of notifications */
};

struct Event
{
   friend class Notifier;
   
   private:

       /* User to skip while showing this event. */
           
       User* skip;
    
       /* Command that took place. */
       
       std::string command;

       /* Who to nofiy this event. */
       
       NOTIFY_LEVEL level;
       
   public:

       Event(User* usr, const std::string& cmd, NOTIFY_LEVEL lvl) : skip(usr), command(cmd), level(lvl)
       {
       
       }
};

typedef std::map<User*, NOTIFY_LEVEL> NotifyMap;

class ExportAPI Notifier 
{
   private:
         
         /* Pending events to be flushed */
         
         std::deque<Event> events;
         
         NotifyMap NotifyList;
            
   public: 

         /* Constructor */
         
         Notifier();
         
        /* 
         * Adds an user to the notification list.
         * 
         * @parameters:
         *
         *         · NOTIFY_LEVEL   : Notification level.
         *         · User	    : User that will be notified.
         * 
         * @return:
         *
         *         - bool
         *
         *	       · true       : User added successfuly.
         *             · false	    : An error has occured.
         */

         bool Add(NOTIFY_LEVEL lvl, User* user);

        /* 
         * Checks if provided user is in NotifyList.
         * 
         * @parameters:
	 *
	 *         · user	: User to check:
	 * 
         * @return:
 	 *
 	 *         -bool 
 	 *
         *             · true	: User is in NotifyList.
         *             · false	: User is not.
         */    
         
         bool Has(User* user);

        /* 
         * Removes an user from NotifyList.
         * 
         * @parameters:
	 *
	 *         · user    : User to remove.
         */    
         
         void Remove(User* user);

        /* 
         * Adds an user to the 'Skipping event list'.
         * 
         * @parameters:
	 *
	 *         · User	   : User to skip.
	 *         · NOTIFY_LEVEL  : Level of events this event is from.
	 *         · string	   : Message to send.
         */    
         
         void SkipPush(User* skip, NOTIFY_LEVEL level, const std::string& buff);

         void SkipPush(User* skip, NOTIFY_LEVEL level, const char *fmt, ...) BERYL_PRINTF(4, 5);

        /* 
         * Adds an event to the event queue.
         * 
         * @parameters:
         *
         *         · NOTIFY_LEVEL    : Log Level.
         *         · string	     : Message to send.
         */    
         
         void Push(NOTIFY_LEVEL level, const std::string& buff);

         void Push(NOTIFY_LEVEL level, const char *fmt, ...) BERYL_PRINTF(3, 4);

        /* 
         * Flushes all pending events. This function is
         * called inside the mainloop.
         */

         void Flush();

        /* 
         * Resets NotifyList and pending events.
         * 
         * This function should be called before exiting Beryl.
         */    
                   
         void Reset();

        /* 
         * Counts pending events.
         * 
         * @return:
 	 *
         *         · uint	: Events size.
         */          
            
         unsigned int Count()
         {
                return this->events.size();
         }
         
};