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
#include "core_channel.h"

CommandMyChans::CommandMyChans(Module* parent) : Command(parent, "MYCHANS", 0, 0)
{

}

COMMAND_RESULT CommandMyChans::Handle(User* user, const Params& parameters)
{
          Dispatcher::JustAPI(user, BRLD_START_LIST);

          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-22s|", "Channel"));
          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));

          for (User::SubsList::iterator i = user->chans.begin(); i != user->chans.end(); i++)
          {
                    Channel* const chan = (*i)->chan;
                    Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-22s|", chan->GetName().c_str()), Daemon::Format("%s", chan->GetName().c_str()));
                    
          }

          Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
          Dispatcher::JustAPI(user, BRLD_END_LIST);
          return SUCCESS;
}

CommandChans::CommandChans(Module* parent) : Command(parent, "GETCHANS", 1, 1)
{
          syntax = "<instance>";
          flags = 'e';
}

COMMAND_RESULT CommandChans::Handle(User* user, const Params& parameters)
{
          const User* found = Kernel->Clients->FindInstanceOnly(parameters[0]);
           
          if (!found)
          {
                  user->SendProtocol(ERR_INPUT, PROCESS_NULL);
                  return FAILED;
          }
           
          Dispatcher::JustAPI(user, BRLD_START_LIST);

          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-22s|", found->instance.c_str()));
          Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
           
          for (User::SubsList::iterator i = found->chans.begin(); i != found->chans.end(); i++)
          {
                  Channel* const chan = (*i)->chan;
                  Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-22s|", chan->GetName().c_str()), Daemon::Format("%s", chan->GetName().c_str()));
          }

          Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));

          Dispatcher::JustAPI(user, BRLD_END_LIST);
          return SUCCESS;
}

CommandUsers::CommandUsers(Module* parent) : Command(parent, "USERS", 1)
{
            syntax = "<channel>";
}

COMMAND_RESULT CommandUsers::Handle(User* user, const Params& parameters)
{
            const std::string& channel = parameters[0];

            if (!Kernel->Engine->ValidChannel(channel))
            {
                     user->SendProtocol(ERR_INPUT, INVALID);
                     return FAILED;
            }

            Channel* const chan = Kernel->Channels->Find(channel);

            if (!chan)
            {
                  user->SendProtocol(ERR_INPUT, NOT_FOUND);
                  return FAILED;
            }

            const size_t users = chan->GetClientCounter();

            user->SendProtocol(BRLD_OK, convto_string(users));
            return SUCCESS;
}

CommandListChans::CommandListChans(Module* parent) : Command(parent, "LISTCHANS", 0, 0)
{
        last_empty_ok = false;
}

COMMAND_RESULT CommandListChans::Handle(User* user, const Params& parameters)
{
        size_t minusers = 0;
        size_t maxusers = 0;

        for (Params::const_iterator iter = parameters.begin(); iter != parameters.end(); ++iter)
        {
                const std::string& constraint = *iter;

                if (constraint[0] == '<')
                {
                        maxusers = convto_num<size_t>(constraint.c_str() + 1);
                }
                else if (constraint[0] == '>')
                {
                        minusers = convto_num<size_t>(constraint.c_str() + 1);
                }
       }
       
        const ChanMap& chans = Kernel->Channels->GetSubs();
      
        unsigned int counter = 0;

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-10s", "Name", "Clients"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-10s", Dispatcher::Repeat("―", 30).c_str(), Dispatcher::Repeat("―", 10).c_str()));
       
        for (ChanMap::const_iterator i = chans.begin(); i != chans.end(); ++i)
        {
                     Channel* const chan = i->second;
                     const size_t users = chan->GetClientCounter();
                     
                     if ((minusers && users <= minusers) || (maxusers && users >= maxusers))
                     {
                          continue;
                     }
                     
                     counter++;
                     Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("%-30s | %-10s", chan->GetName().c_str(), convto_string(users).c_str()), Daemon::Format("%s %s", chan->GetName().c_str(), convto_string(users).c_str()));
        }

        Dispatcher::JustAPI(user, BRLD_END_LIST);
        return SUCCESS;
}
