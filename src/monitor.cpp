/*
 * BerylDB - A modular database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2021 Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include <sstream>

#include "beryl.h"
#include "monitor.h"

MonitorHandler::MonitorHandler()
{

}

bool MonitorHandler::Add(User* user, MONITOR_LEVEL level)
{
       if (level < 5 || level > 10)
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

        CMDBuffer adding(instance, cmd, level, params);
        this->buffer.push_back(adding);
}

void MonitorHandler::Flush()
{
        BufferQueue pending = this->buffer;
        MonitorMap& all = this->MonitorList;

        for (BufferQueue::iterator it = pending.begin(); it != pending.end(); it++)
        {
               CMDBuffer flushing = *it;
               
               for (MonitorMap::iterator uit = all.begin(); uit != all.end(); uit++)
               {
                      User* user = uit->first;
                      MONITOR_LEVEL level = uit->second;
                      
                      if (user->instance == flushing.instance)
                      {
                           continue;
                      }
                      
                      if (level >= flushing.level)
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
                             Dispatcher::SmartDiv(user, BRLD_MONITOR, flushing.instance, Daemon::Format("%s %s", flushing.command.c_str(), sfinal.c_str()).c_str(), ":");
                      }
               }     
               
               this->buffer.pop_front();
        } 
}