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
#include "core_futures.h"

CommandFuture::CommandFuture(Module* Creator) : Command(Creator, "FUTURE", 3, 3)
{
         check_value    =       true;         
         check_key	=	1;
         group 		= 	'f';
         syntax 	= 	"<seconds> <key> \"value\"";
}

COMMAND_RESULT CommandFuture::Handle(User* user, const Params& parameters) 
{       
          const std::string& seconds 	= 	parameters[0];
          
          if (!CheckValidPos(user, seconds))
          {
              return FAILED;
          }
  
          ExpireHelper::Future(user, parameters[1], convto_num<unsigned int>(seconds), parameters.back());
          return SUCCESS;
}

CommandTTE::CommandTTE(Module* Creator) : Command(Creator, "TTE", 1)
{
        check_key       =       0;
        group 		= 	'f';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandTTE::Handle(User* user, const Params& parameters)
{
         const signed int ttl = FutureManager::GetTIME(user->GetDatabase(), parameters[0], user->select);
         
         if (ttl != -1)
         {
                  user->SendProtocol(BRLD_OK, static_cast<unsigned int>((int)ttl - (int)Kernel->Now()));
         }
         else
         {      
                  /* Unable to remove this 'key' from ExpireManager. */
                  
                  user->SendProtocol(ERR_INPUT, PROCESS_NULL);
                  return FAILED;
         }
         
         return SUCCESS;
}

CommandExec::CommandExec(Module* Creator) : Command(Creator, "EXEC", 1, 1)
{
       check_key       =       0;
       group 	       =       'f';
       syntax	       =       "<key>";
}

COMMAND_RESULT CommandExec::Handle(User* user, const Params& parameters)
{
       GlobalHelper::UserFutureExecute(user, parameters[0]);
       return SUCCESS;
}

CommandFResetAll::CommandFResetAll(Module* Creator) : Command(Creator, "FRESETALL", 0, 0)
{
       flags = 'e';
}

COMMAND_RESULT CommandFResetAll::Handle(User* user, const Params& parameters)
{
       FutureManager::Reset();
       user->SendProtocol(BRLD_OK, PROCESS_OK);
       return SUCCESS;
}

CommandFReset::CommandFReset(Module* Creator) : Command(Creator, "FRESET", 0, 1)
{
        group  = 'f';        
        syntax = "<*select>";
}

COMMAND_RESULT CommandFReset::Handle(User* user, const Params& parameters)
{
        std::string use;

        if (parameters.size())
        { 
            use = parameters[0];

            if (!CheckValid(user, use))
            {
                 return FAILED;
            }

            if (!Daemon::CheckRange(user, use, INVALID_RANGE, 1, 100))
            {
                  return FAILED;
            }
        }
        else
        {
             use = user->select;
        }

        if (user->GetDatabase() && user->GetDatabase()->IsClosing())
        {
              user->SendProtocol(ERR_INPUT, DATABASE_BUSY);
              return FAILED;
        }

        Kernel->Store->Futures->SelectReset(user->GetDatabase()->GetName(), convto_num<unsigned int>(use));
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        return SUCCESS;
}

CommandCancel::CommandCancel(Module* Creator) : Command(Creator, "CANCEL", 1, 1)
{
        check_key       =       0;
        group 		= 	'f';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandCancel::Handle(User* user, const Params& parameters) 
{
        GlobalHelper::FutureCancel(user, parameters[0]);
        return SUCCESS;
}

CommandFutureAT::CommandFutureAT(Module* Creator) : Command(Creator, "FUTSET", 3, 3)
{
        check_value     =       true;         
        check_key       =       1;
        group 		= 	'f';
        syntax 		= 	"<epoch time> <key> \"value\"";
}

COMMAND_RESULT CommandFutureAT::Handle(User* user, const Params& parameters) 
{    
          const std::string& seconds 	= 	parameters[0];
          
          if (!CheckValid(user, seconds))
          {
              return FAILED;
          }

          const unsigned int exp_usig = convto_num<unsigned int>(seconds);

          if ((time_t)exp_usig < Kernel->Now())
          {
                 user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
                 return FAILED;
          }
          
          ExpireHelper::FutureAT(user, parameters[1], exp_usig, parameters.back());
          return SUCCESS;
}

CommandFutureList::CommandFutureList(Module* Creator) : Command(Creator, "FTLIST", 0, 1)
{
         last_empty_ok 	= 	true;
         syntax 	= 	"<*argument>";
}

COMMAND_RESULT CommandFutureList::Handle(User* user, const Params& parameters)
{
         const std::string& arg = parameters[0];
         
         /* 
          * These two arguments are valid.  Arguments valid are:
          *
          * h: Human readable format.
          * r: Raw format date.
          */
         
         if (parameters.size() && arg != "h" && arg != "r")
         {
                  user->SendProtocol(ERR_INPUT, INVALID_TYPE);
                  return FAILED;
         }
         
         /* User has requested all expires with expiration date. */
         
         const FutureMap& futures = Kernel->Store->Futures->GetFutures();
         
         Dispatcher::JustAPI(user, BRLD_START_LIST);

         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-25s| %-25s| %-9s| %-9s|", "Key", "Schedule", "Select", "Database"));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));

         for (FutureMap::const_iterator it = futures.begin(); it != futures.end(); ++it)
         {
               FutureEntry const entry = it->second;
               
               std::string schedule;
               
               if (parameters.size() && arg == "h")
               {
                      schedule = convto_string(entry.schedule);
               }
               else 
               {
                      schedule = Daemon::HumanEpochTime(entry.schedule).c_str();
               }
               
               Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-25s| %-25s| %-9s| %-9s|", entry.key.c_str(), schedule.c_str(), convto_string(entry.select).c_str(), entry.database->GetName().c_str()), Daemon::Format("%s %s %s %s",  entry.key.c_str(), schedule.c_str(), convto_string(entry.select).c_str(), entry.database->GetName().c_str()));
               
         }
         
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));
         Dispatcher::JustAPI(user, BRLD_END_LIST);
         return SUCCESS;
}

CommandSelectCount::CommandSelectCount(Module* Creator) : Command(Creator, "FTSELECT", 0, 1)
{
         syntax = "<select>";
}

COMMAND_RESULT CommandSelectCount::Handle(User* user, const Params& parameters) 
{
         unsigned int select;
         
         /* No argument provided, we simply count expire items. */
         
         if (!parameters.size())
         {
                  select = user->select;
         }
         else
         {
                  select  = convto_num<unsigned int>(parameters[0]);
         }
         
         const FutureMap& expiring = Kernel->Store->Futures->GetFutures();

         Dispatcher::JustAPI(user, BRLD_START_LIST);

         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-25s | %-25s | %-9s | %-10s", "Key", "Schedule", "Select", "Database"));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-25s | %-25s | %-9s | %-10s", Dispatcher::Repeat("―", 25).c_str(), Dispatcher::Repeat("―", 25).c_str(), Dispatcher::Repeat("―", 9).c_str(), Dispatcher::Repeat("―", 10).c_str()));

         for (FutureMap::const_iterator it = expiring.begin(); it != expiring.end(); ++it)
         {
               FutureEntry const entry = it->second;

               if (entry.select != select || entry.database != user->GetDatabase())
               {  
                         continue;
               }
               
               std::string schedule = Daemon::HumanEpochTime(entry.schedule).c_str();
               Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("%-25s | %-25s | %-9s | %-10s", entry.key.c_str(), schedule.c_str(), convto_string(entry.select).c_str(), entry.database->GetName().c_str()), Daemon::Format("%s %s %s %s",  entry.key.c_str(), schedule.c_str(), convto_string(entry.select).c_str(), entry.database->GetName().c_str()));
         }
         
         Dispatcher::JustAPI(user, BRLD_END_LIST);
         return SUCCESS;
}

