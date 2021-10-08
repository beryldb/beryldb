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
#include "core_expires.h"

CommandExpireFIND::CommandExpireFIND(Module* Creator) : Command(Creator, "EXPFIND", 1, 2)
{
         last_empty_ok  	= 	true;
         flags 			= 	'e';
         syntax 		= 	"<%key> <argument>";
}

COMMAND_RESULT CommandExpireFIND::Handle(User* user, const Params& parameters)
{
         const std::string& arg 	= 	parameters[1];

         if (parameters.size() == 2 && arg != "h" && arg != "r")
         {
                  user->SendProtocol(ERR_INPUT, INVALID_TYPE);
                  return FAILED;
         }
         
         const ExpireMap& expiring = Kernel->Store->Expires->GetExpires();

         Dispatcher::JustAPI(user, BRLD_START_LIST);
         
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-25s| %-25s| %-9s| %-9s|", "Key", "Schedule", "Select", "Database"));
         
         
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-30s+%-10s", Dispatcher::Repeat("-", 31).c_str(), Dispatcher::Repeat("-", 11).c_str()));
         
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-30s | %-10s", "Expire", "Schedule"));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-10s", Dispatcher::Repeat("―", 30).c_str(), Dispatcher::Repeat("―", 10).c_str()));

         for (ExpireMap::const_iterator it = expiring.begin(); it != expiring.end(); ++it)
         {
               ExpireEntry const entry = it->second;

               if (entry.database != user->GetDatabase())
               {
                     continue;
               }

               if (entry.select != user->select)
               {
                    continue;
               }
               
               if (!Daemon::Match(entry.key, parameters[0]))
               {
                      continue;
               }
               
               std::string schedule;

               if (parameters.size() == 2 && arg == "h")
               {
                      schedule = convto_string(entry.schedule);
               }
               else 
               {
                      schedule = Daemon::HumanEpochTime(entry.schedule).c_str();
               }
               
               Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("%-30s | %-10s", entry.key.c_str(), schedule.c_str()), Daemon::Format("%s %s", entry.key.c_str(), schedule.c_str()));
         }

         Dispatcher::JustAPI(user, BRLD_END_LIST);
         return SUCCESS;
}

CommandExpireLIST::CommandExpireLIST(Module* Creator) : Command(Creator, "EXPLIST", 0, 1)
{
         last_empty_ok 	= 	true;
         flags 		= 	'e';
         syntax 	= 	"<*argument>";
}

COMMAND_RESULT CommandExpireLIST::Handle(User* user, const Params& parameters)
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
         
         const ExpireMap& expiring = Kernel->Store->Expires->GetExpires();
         
         if (!expiring.size())
         {
              Dispatcher::JustAPI(user, BRLD_START_LIST);
              Dispatcher::JustEmerald(user, BRLD_OK, PROCESS_EMPTY);
              Dispatcher::JustAPI(user, BRLD_END_LIST);
              return SUCCESS;
         }
         
         
         Dispatcher::JustAPI(user, BRLD_START_LIST);

         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-25s| %-25s| %-9s| %-9s|", "Key", "Schedule", "Select", "Database"));
         Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));

         for (ExpireMap::const_iterator it = expiring.begin(); it != expiring.end(); ++it)
         {
               ExpireEntry entry = it->second;
               
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

         Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-25s+%-25s+%-9s+%-10s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 10).c_str(), Dispatcher::Repeat("-", 10).c_str()));
         Dispatcher::JustAPI(user, BRLD_END_LIST);
         return SUCCESS;
}

CommandReset::CommandReset(Module* Creator) : Command(Creator, "RESET", 0)
{
         flags = 'e';
}

COMMAND_RESULT CommandReset::Handle(User* user, const Params& parameters)
{       
         const unsigned int counter = Kernel->Store->Expires->CountAll();

         /* Clears all expires pending. */
         
         ExpireManager::Reset();
         user->SendProtocol(BRLD_OK, PROCESS_OK);
         sfalert(user, NOTIFY_DEFAULT, "All expires have been removed: %u", counter);
         return SUCCESS;
}

CommandSelectReset::CommandSelectReset(Module* Creator) : Command(Creator, "SRESET", 0, 1)
{
        syntax = "<select>";
}

COMMAND_RESULT CommandSelectReset::Handle(User* user, const Params& parameters)
{
        std::string use;
       
        if (parameters.size())
        { 
            use = parameters[0];
        
            if (!CheckValidPos(user, use))
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

        /* Clears all expires pending. */

        const unsigned int counter = ExpireManager::SelectReset(user->GetDatabase()->GetName(), convto_num<unsigned int>(use));
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        
        sfalert(user, NOTIFY_DEFAULT, "Expires from select %s have been removed: %u", use.c_str(), counter);
        return SUCCESS;
}

CommandExpire::CommandExpire(Module* Creator) : Command(Creator, "EXPIRE", 2, 2)
{
        check_key       =       0;
        group  		= 	'e';
        syntax 		= 	"<key> <seconds>";
}

COMMAND_RESULT CommandExpire::Handle(User* user, const Params& parameters) 
{       
          const std::string& seconds 	= 	parameters[1];

          if (!CheckValidPos(user, seconds))
          {
                return FAILED;
          }
                  
          ExpireHelper::Expire(user, parameters[0], convto_num<unsigned int>(seconds));
          return SUCCESS;
}

CommandDBEReset::CommandDBEReset(Module* Creator) : Command(Creator, "ERESET", 1, 1)
{
          group  = 'e';
          syntax = "<database>";
}

COMMAND_RESULT CommandDBEReset::Handle(User* user, const Params& parameters) 
{       
          const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(parameters[0]);

          if (!database)
          {
                user->SendProtocol(ERR_INPUT, PROCESS_NULL);
                return FAILED;
          }
          
          if (database->IsClosing())
          {
                user->SendProtocol(ERR_INPUT, DATABASE_BUSY);
                return FAILED;
          }
        
          Kernel->Store->Expires->DatabaseReset(database->GetName());
          return SUCCESS;
}

CommandSetex::CommandSetex(Module* Creator) : Command(Creator, "SETEX", 3, 3)
{
          check_key   	= 	1;
          check_value 	= 	true;
          group       	= 	'e';
          syntax      	= 	"<seconds> <key> \"value\"";
}

COMMAND_RESULT CommandSetex::Handle(User* user, const Params& parameters) 
{       
          const std::string& seconds 		= 	parameters[0];

          if (!CheckValidPos(user, seconds))
          {
               return FAILED;
          }

          /* We convert expiring time to int. */
          
          ExpireHelper::Setex(user, convto_num<unsigned int>(seconds), parameters[1], parameters.back());
          return SUCCESS;
}

CommandExpireAT::CommandExpireAT(Module* Creator) : Command(Creator, "EXPIREAT", 2, 2)
{
        check_key       =       0;
        group  		= 	'e';
        syntax 		= 	"<key> <epoch time>";
}

COMMAND_RESULT CommandExpireAT::Handle(User* user, const Params& parameters) 
{    
          const std::string& seconds 	= 	parameters[1];

          if (!CheckValidPos(user, seconds))
          {
              return FAILED;
          }
                  
          const unsigned int exp_usig = convto_num<unsigned int>(seconds);

          if ((time_t)exp_usig < Kernel->Now())
          {
                 user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
                 return FAILED;
          }
          
          ExpireHelper::ExpireAT(user, parameters[0], exp_usig);
          return SUCCESS;
}