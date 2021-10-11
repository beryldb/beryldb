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
#include "core_admin.h"

class AutoResume : public Timer
{
   private:
           
           User* user;
           
   public:
   
           AutoResume(User* usr, unsigned int seconds) : Timer(seconds, false), user(usr)
           {
                   Kernel->Tickers->Add(this);
           }
           
           bool Run(time_t current)
           {
                   if (!this->user || this->user->IsQuitting())
                   {
                           return false;
                   }
                   
                   user->Paused = false;
                   return false;
           }
};

CommandFinger::CommandFinger(Module* parent) : Command(parent, "FINGER", 0)
{
        flags = 'e';
}

COMMAND_RESULT CommandFinger::Handle(User* user, const Params& parameters)
{
        const LoginHash& users = Kernel->Clients->GetLogins();

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-20s+%-10s+%-10s+%-25s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 21).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 26).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-25s| %-20s| %-10s| %-10s| %-25s|", "Instance", "IP", "Login", "Agent", "Logged"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-20s+%-10s+%-10s+%-25s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 21).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 26).c_str()));

        for (LoginHash::const_iterator i = users.begin(); i != users.end(); ++i)
        {
            	User* const login = i->second;
            	
            	if (login->registered != REG_OK)
            	{
            	        continue;
            	}
            	
                Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-25s| %-20s| %-10s| %-10s| %-25s|", login->instance.c_str(), login->GetReadableIP().c_str(), login->login.c_str(), login->agent.c_str(), Daemon::HumanEpochTime(login->GetConnected()).c_str()), Daemon::Format("%s %s %s %s %s", login->instance.c_str(), login->GetReadableIP().c_str(), login->login.c_str(), login->agent.c_str(), Daemon::HumanEpochTime(login->GetConnected()).c_str()));
        }

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-25s+%-20s+%-10s+%-10s+%-25s+", Dispatcher::Repeat("-", 26).c_str(), Dispatcher::Repeat("-", 21).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 11).c_str(), Dispatcher::Repeat("-", 26).c_str()));
        Dispatcher::JustAPI(user, BRLD_END_LIST);
	return SUCCESS;
}

CommandPause::CommandPause(Module* parent) : Command(parent, "PAUSE", 1, 2)
{
        flags = 'm';
        syntax = "<instance> <*seconds>";
}

COMMAND_RESULT CommandPause::Handle(User* user, const Params& parameters)
{
        User* const target = Kernel->Clients->FindInstance(parameters[0]);

        if (!target)
        {
                user->SendProtocol(Protocols::NoInstance(parameters[0]));
                return FAILED;
        }

        target->Paused = true;
        
        if (parameters.size() == 2)
        {
             new AutoResume(target, convto_num<unsigned int>(parameters[1]));
        }
        
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        return SUCCESS;
}

CommandResume::CommandResume(Module* parent) : Command(parent, "RESUME", 1, 1)
{
        flags = 'm';
        syntax = "<instance>";
}

COMMAND_RESULT CommandResume::Handle(User* user, const Params& parameters)
{
        User* const target = Kernel->Clients->FindInstance(parameters[0]);

        if (!target)
        {
                user->SendProtocol(Protocols::NoInstance(parameters[0]));
                return FAILED;
        }

        target->Paused = false;
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        return SUCCESS;
}

CommandIdle::CommandIdle(Module* parent) : Command(parent, "IDLE", 1, 1)
{
        flags  = 'm';
        syntax = "<instance>";
}

COMMAND_RESULT CommandIdle::Handle(User* user, const Params& parameters)
{
        User* const target = Kernel->Clients->FindInstance(parameters[0]);

        if (!target)
        {
                user->SendProtocol(Protocols::NoInstance(parameters[0]));
                return FAILED;
        }	

        LocalUser* luser = IS_LOCAL(target);
        
        if (luser)
        {
                time_t idle = Kernel->Now() - luser->touchbase;
                user->SendProtocol(BRLD_OK, idle);
                return SUCCESS;
        }
        
        return FAILED;
}

CommandLogins::CommandLogins(Module* parent) : Command(parent, "LOGINS", 1, 1)
{
        flags = 'e';
        syntax = "<login>";
}

COMMAND_RESULT CommandLogins::Handle(User* user, const Params& parameters)
{
        const UserVector& logins 	= Kernel->Clients->FindLogin(parameters[0]);

        Dispatcher::JustAPI(user, BRLD_START_LIST);
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-16s | %-20s | %-10s", "Login", "IP", "Agent", "Logged"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-16s | %-20s | %-10s", Dispatcher::Repeat("―", 30).c_str(), Dispatcher::Repeat("―", 16).c_str(), Dispatcher::Repeat("―", 20).c_str(), Dispatcher::Repeat("―", 10).c_str()));

        for (UserVector::const_iterator i = logins.begin(); i != logins.end(); ++i)
        {
            	User* const login = *i;

                if (login->registered != REG_OK)
                {
                        continue;
                }
            	
            	Dispatcher::JustEmerald(user, BRLD_ITEM_LIST, Daemon::Format("%-30s | %-16s | %-20s | %-10s", login->instance.c_str(), login->GetReadableIP().c_str(), login->agent.c_str(), Daemon::HumanEpochTime(login->GetConnected()).c_str())), Daemon::Format("%s %s %s %s", login->instance.c_str(), login->GetReadableIP().c_str(), login->agent.c_str(), Daemon::HumanEpochTime(login->GetConnected()).c_str());
        }

        Dispatcher::JustAPI(user, BRLD_END_LIST);
	return SUCCESS;
}

CommandFindFlags::CommandFindFlags(Module* parent) : Command(parent, "FINDFLAGS", 1, 1)
{
        flags  = 'e';
        syntax = "<flag>";
}

COMMAND_RESULT CommandFindFlags::Handle(User* user, const Params& parameters)
{
        const UserVector& logins = ClientManager::FindPrivs(parameters[0]);

        Dispatcher::JustAPI(user, BRLD_START_LIST);
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-16s | %-20s", "Instance", "Login", "Flags"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("%-30s | %-16s | %-20s", Dispatcher::Repeat("―", 30).c_str(), Dispatcher::Repeat("―", 16).c_str(), Dispatcher::Repeat("―", 20).c_str()));

        for (UserVector::const_iterator i = logins.begin(); i != logins.end(); ++i)
        {
                User* const login = *i;
                Dispatcher::JustEmerald(user, BRLD_ITEM_LIST, Daemon::Format("%-30s | %-16s | %-20s", login->instance.c_str(), login->login.c_str(), login->session->GetFlags().c_str())), Daemon::Format("%s %s %s", login->instance.c_str(), login->login.c_str(), login->session->GetFlags().c_str());
        }

        Dispatcher::JustAPI(user, BRLD_END_LIST);
        return SUCCESS;
}
