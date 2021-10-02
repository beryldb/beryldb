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

#include <sstream>

#include "beryl.h"
#include "notifier.h"
#include "monitor.h"
#include "engine.h"

MonitorHandler::MonitorHandler()
{

}

bool MonitorHandler::Add(User* user, MONITOR_LEVEL level)
{
        if (!user || user->IsQuitting())
        {
            return false;
        }

        this->MonitorList.insert(std::make_pair(user, level));
        return true;
}

bool MonitorHandler::Has(User* user)
{
        MonitorMap::iterator it = this->MonitorList.find(user);

        if (it == this->MonitorList.end())
        {
                return false;
        }
        
        return true;
}
        
void MonitorHandler::Remove(User* user)
{
        this->MonitorList.erase(user);
}        

void MonitorHandler::Push(const std::string& instance, const std::string& cmd, MONITOR_LEVEL level, const CommandModel::Params& params)
{
        if (!this->MonitorList.size())
        {
             return;
        }

        /* CMDBuffer, which is later inserted in a buffered list. */
        
        CMDBuffer adding(instance, cmd, level, params);
        this->buffer.push_back(adding);
}

MonitorMap MonitorHandler::GetList(const std::string& arg)
{
      MonitorMap list;
      
      if (arg.empty())
      {
          return this->MonitorList;
      }
      
      MONITOR_LEVEL monitor = MONITOR_DEFAULT;
      
      if (arg == "DEFAULT")
      {
          monitor = MONITOR_DEFAULT;
      }
      else if (arg == "DEBUG")
      {
          monitor = MONITOR_DEBUG;
      }
      
      MonitorMap all = this->MonitorList;
      
      for (MonitorMap::iterator uit = all.begin(); uit != all.end(); uit++)
      {
               User* user = uit->first;
               MONITOR_LEVEL level = uit->second;
               
               if (level != monitor)
               {
                     continue;
               }
               
               list.insert(std::make_pair(user, level));
      }
      
      return list;
}

void MonitorHandler::Flush()
{
        if (!this->buffer.size())
        {
            return;
        }

        if (!this->MonitorList.size())
        {
             this->buffer.clear();
             return;
        }

        const MonitorMap& all = this->MonitorList;
        
        /* Process next item in buffer */
        
        CMDBuffer flushing    = this->buffer.front();
               
        for (MonitorMap::const_iterator i = all.begin(); i != all.end(); i++)
        {
                      User* const user    = i->first;
                      MONITOR_LEVEL level = i->second;
                      
                      if (!user || user->IsQuitting())
                      {
                           continue;
                      }
                      
                      if (user->instance == flushing.instance)
                      {
                           continue;
                      }
                      
                      if (level <= flushing.level)
                      {
                             std::ostringstream fullparams;
                             
                             unsigned int size = flushing.params.size();
                             unsigned int counter = 0;
                             
                             for (std::string item: flushing.params) 
                             {
                                       fullparams << item;
                                       counter++;
                                       
                                       if (size != counter)
                                       {
                                             fullparams << " ";    
                                       }
                             }
    
                             std::string sfinal(fullparams.str());
                             
                             user->SendProtocol(BRLD_MONITOR, Daemon::Format("%s", Daemon::Format("%s %s %s", flushing.instance.c_str(), flushing.command.c_str(), sfinal.c_str()).c_str()));
                      }
        }     
               
        this->buffer.pop_front();
}

void MonitorHandler::Reset()
{
        this->MonitorList.clear();
        this->buffer.clear();
}

Notifier::Notifier()
{

}

bool Notifier::Add(NOTIFY_LEVEL lvl, User* user)
{
        if (!user || user->IsQuitting())
        {
            return false;
        }
        
        this->NotifyList.insert(std::make_pair(user, lvl));
        return true;
}

bool Notifier::Has(User* user)
{
        NotifyMap::iterator it = this->NotifyList.find(user);

        if (it == this->NotifyList.end())
        {
                return false;
        }

        return true;
}

void Notifier::Remove(User* user)
{
        this->NotifyList.erase(user);
}

void Notifier::SkipPush(User* skip, NOTIFY_LEVEL level, const std::string& buff)
{
        if (!this->NotifyList.size() || buff.empty() || !skip || skip->IsQuitting())
        {
             return;
        }

        Event adding(skip, buff, level);
        this->events.push_back(adding);
}

void Notifier::SkipPush(User* skip, NOTIFY_LEVEL level, const char *fmt, ...) 
{
       std::string buff;
       SCHEME(buff, fmt, fmt);
       this->SkipPush(skip, level, buff);
}

void Notifier::Push(NOTIFY_LEVEL level, const std::string& buff)
{
        if (!this->NotifyList.size() || buff.empty())
        {
             return;
        }

        Event adding(NULL, buff, level);
        this->events.push_back(adding);
}

void Notifier::Push(NOTIFY_LEVEL level, const char *fmt, ...) 
{
       std::string buff;
       SCHEME(buff, fmt, fmt);
       this->Push(level, buff);
}


void Notifier::Flush()
{
       if (this->events.empty())
       {
            return;
       }

       if (!this->NotifyList.size())
       {
            this->events.clear();
            return;
       }
      
       Event ready = this->events.front();
       
       const NotifyMap& all = this->NotifyList;

       for (NotifyMap::const_iterator i = all.begin(); i != all.end(); i++)
       {
                      User* const user = i->first;
                      
                      if (!user || user->IsQuitting())
                      {
                           continue;
                      }
                       
                      if (ready.skip && !ready.skip->IsQuitting())
                      {
                            /* Continues on skipping user. */
                            
                            if (ready.skip == user)
                            {
                                 continue;
                            }
                      }
                      
                      NOTIFY_LEVEL level = i->second;
                      
                      if (level <= ready.level)
                      {
                             user->SendProtocol(BRLD_NOTIFICATION, Daemon::Format("%s", ready.command.c_str()).c_str());
                      }
       }

       this->events.pop_front();
}

void Notifier::Reset()
{
       this->events.clear();
       this->NotifyList.clear();
}


