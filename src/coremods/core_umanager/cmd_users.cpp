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
#include "core_umanager.h"

CommandAddUser::CommandAddUser(Module* parent) : Command(parent, "ADDUSER", 2, 2)
{
        check_root      =       true;
        check_login     =       0;
        flags  		= 	'r';
        syntax 		= 	"<login> <password>";
}

COMMAND_RESULT CommandAddUser::Handle(User* user, const Params& parameters)
{
        const std::string& login	=	parameters[0];
        const std::string& pass 	= 	parameters[1];
        
        if (pass.length() < 3 || pass.length() > 30)
        {
                user->SendProtocol(ERR_INPUT, INVALID_FORMAT);
                return FAILED; 
        }
        
        const std::string& exists = UserHelper::Find(login, "created");
        
        if (!exists.empty())
        {
                user->SendProtocol(ERR_INPUT, ENTRY_DEFINED);
                return FAILED;
        }
        
        /* We can now insert the user. */
        
        if (UserHelper::Add(login, pass))
        {
                user->SendProtocol(BRLD_OK, PROCESS_OK);
        }

        return SUCCESS;
}

CommandDelUser::CommandDelUser(Module* parent) : Command(parent, "DELUSER", 1, 1)
{
        check_root      =       true;
        check_exists    =       true;
        check_login     =       0;
        flags  		= 	'r';
        syntax 		= 	"<login>";
}

COMMAND_RESULT CommandDelUser::Handle(User* user, const Params& parameters)
{
        if (UserHelper::Remove(parameters[0]))
        {
                user->SendProtocol(BRLD_OK, PROCESS_OK);
                ClientManager::DisconnectAll(parameters[0], PROCESS_OK);
        }

        return SUCCESS;
}

CommandSetStatus::CommandSetStatus(Module* parent) : Command(parent, "SETSTATUS", 2, 2)
{
        check_root	=	true;
        check_exists	=	true;
        check_login	=	0;
        flags  		= 	'r';
        syntax 		= 	"<login> <bool>";
}

COMMAND_RESULT CommandSetStatus::Handle(User* user, const Params& parameters)
{
        if (Helpers::as_bool(parameters[1], false) == false)
        {
                ClientManager::DisconnectAll(parameters[0], PROCESS_OK);
                Kernel->Logins->Remove(parameters[0]);
        }
        
        CMapsHelper::Set(parameters[0], "status", convto_string(Helpers::as_bool(parameters[1], false)));
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        return SUCCESS;
}

CommandListUsers::CommandListUsers(Module* parent) : Command(parent, "LISTUSERS", 0)
{
         flags = 'r';
}

COMMAND_RESULT CommandListUsers::Handle(User* user, const Params& parameters)
{
        const StringVector& users = STHelper::HList("userlist");
        
        Dispatcher::JustAPI(user, BRLD_START_LIST);
        Dispatcher::Simple(user, "User", "Created");

        for (StringVector::const_iterator i = users.begin(); i != users.end(); i++)
        {
                const std::string item = *i;

                std::string created = UserHelper::Find(item, "created");

                if (created.empty())
                {
                      continue;
                }
                
                Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-16s| %-29s|", item.c_str(), Daemon::HumanEpochTime(convto_num<time_t>(created)).c_str()), Daemon::Format("%s %s", item.c_str(), created.c_str()));
        }        
        
        Dispatcher::JustAPI(user, BRLD_END_LIST);
        Dispatcher::SimpleEnd(user);
        
        return SUCCESS;
}

CommandGetStatus::CommandGetStatus(Module* parent) : Command(parent, "GETSTATUS", 1, 1)
{
         syntax = "<login>";
         flags = 'r';
}

COMMAND_RESULT CommandGetStatus::Handle(User* user, const Params& parameters)
{
        const std::string& status = UserHelper::Find(parameters[0], "status");

        if (status.empty())
        {
             user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
             return FAILED;
        }

        user->SendProtocol(BRLD_OK, status);
        return SUCCESS;
}

CommandListStatus::CommandListStatus(Module* parent) : Command(parent, "LISTSTATUS", 0, 1)
{
        syntax = "<*status>";
        flags = 'r';
}

COMMAND_RESULT CommandListStatus::Handle(User* user, const Params& parameters)
{
        bool ReqStatus = false;

        if (parameters.size())
        {
              ReqStatus = Helpers::as_bool(parameters[0], false);
        }
        
        const StringVector& users = STHelper::HList("userlist");

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", "Admin", "Status"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));

        for (StringVector::const_iterator i = users.begin(); i != users.end(); i++)
        {
                const std::string item = *i;
                std::string status = CMapsHelper::Get(item, "status").response;

                if (status.empty())
                {
                       continue;
                }
                
                if (parameters.size())
                {
                      if (Helpers::as_bool(status, false) != ReqStatus)
                      {
                           continue;
                      }
                }
                
                Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("|%-16s | %-29s|", item.c_str(), convto_string(Helpers::as_bool(status)).c_str()), Daemon::Format("%s %s", item.c_str(), convto_string(Helpers::as_bool(status)).c_str()));
        }

        Dispatcher::JustAPI(user, BRLD_END_LIST);
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        
        return SUCCESS;
}

CommandListAdmins::CommandListAdmins(Module* parent) : Command(parent, "LISTADMINS", 0)
{
        flags = 'r';
}

COMMAND_RESULT CommandListAdmins::Handle(User* user, const Params& parameters)
{
        const StringVector& users = STHelper::HList("userlist");

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", "Admin", "Flag"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));

        for (StringVector::const_iterator i = users.begin(); i != users.end(); i++)
        {
                const std::string item = *i;
                std::string userflags = UserHelper::CheckFlags(item);
                
                if (userflags.empty())
                {
                      continue;
                }
                
                Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-16s| %-29s|", item.c_str(), userflags.c_str()), Daemon::Format("%s %s", item.c_str(), userflags.c_str()));
        }

        Dispatcher::JustAPI(user, BRLD_END_LIST);
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        
        return SUCCESS;
}

CommandPasswd::CommandPasswd(Module* parent) : Command(parent, "PASSWD", 1, 2)
{
        syntax = "<password>";
}

COMMAND_RESULT CommandPasswd::Handle(User* user, const Params& parameters)
{
        if (parameters.size() < 2)
        {
                const std::string& pass = parameters[0];

                if (UserHelper::ChangePass(user->login, pass))
                {
                        user->SendProtocol(BRLD_OK, PROCESS_OK);
                }
                else
                {
                        user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
                        return FAILED;
                }

                return SUCCESS;
        }
        
        /* Check if user has root access. */
        
        if (!user->IsAdmin())
        {
                user->SendProtocol(ERR_INPUT, ACCESS_DENIED);
                return FAILED;        
        }
                
        const std::string& newlogin 	= parameters[0];
        const std::string& pass 	= parameters[1];

        if (newlogin.length() < 3 || newlogin.length() > 15)
        {
                user->SendProtocol(ERR_INPUT, USER_MIMMAX_LENGTH);
                return FAILED;
        }

        if (!Kernel->Engine->ValidLogin(newlogin))
        {
                user->SendProtocol(ERR_INPUT, INVALID_UNAME);
                return FAILED;
        }
        
        if (pass.length() < 3 || pass.length() > 30)
        {
                user->SendProtocol(ERR_INPUT, PASS_MIMMAX_LENGTH);
                return FAILED; 
        }
        
        const std::string& exists 	= UserHelper::Find(newlogin, "created");
        
        if (exists.empty())
        {
                user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
                return FAILED;
        }
        
        if (UserHelper::ChangePass(newlogin, pass))
        {
                user->SendProtocol(BRLD_OK, PROCESS_OK);
        }
        else
        {
                user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
                return FAILED;
        }
        
        return SUCCESS;
}

CommandResetCache::CommandResetCache(Module* parent) : Command(parent, "RESETCACHE", 0)
{
        flags 		= 	'm';
}

COMMAND_RESULT CommandResetCache::Handle(User* user, const Params& parameters)
{
        Kernel->Logins->Reset();
        
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        return SUCCESS;
}

CommandInCache::CommandInCache(Module* parent) : Command(parent, "INCACHE", 1, 1)
{
        check_exists    =       true;
        check_login     =       0;
        flags  		= 	'm';
        syntax 		= 	"<login>";
}

COMMAND_RESULT CommandInCache::Handle(User* user, const Params& parameters)
{
        user->SendProtocol(BRLD_OK, convto_string(Kernel->Logins->InCache(parameters[0])));
        return SUCCESS;
}
