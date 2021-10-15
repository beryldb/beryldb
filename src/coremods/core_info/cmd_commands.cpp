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
#include "core_info.h"

CommandSyntax::CommandSyntax(Module* parent) : Command(parent, "SYNTAX", 1, 1)
{
	syntax = "<command>";
}

COMMAND_RESULT CommandSyntax::Handle(User* user, const Params& parameters)
{
	const std::string& cmd = parameters[0];
	
	Command* const Found = Kernel->Commander->GetBase(cmd);
	
	if (!Found)
	{
		user->SendProtocol(ERR_INPUT, PROCESS_NULL);
		return FAILED;
	}
	
	user->SendProtocol(BRLD_OK, Daemon::Format("%s %s", Found->name.c_str(), Found->syntax.c_str()));
	return SUCCESS;
}

CommandCommands::CommandCommands(Module* parent) : Command(parent, "COMMANDS", 0, 1)
{
         group = 'w';
         syntax = "<key>";
}

COMMAND_RESULT CommandCommands::Handle(User* user, const Params& parameters)
{
	std::string key;
	
	if (parameters.size() > 0)
	{
		key = parameters[0];
	}
	else
	{
		key = "*";
	}

	std::transform(key.begin(), key.end(), key.begin(), ::toupper);
	
	const CommandHandler::CommandMap& commands = Kernel->Commander.GetCommands();
	
	std::map<std::string, std::string> list;

	for (CommandHandler::CommandMap::const_iterator i = commands.begin(); i != commands.end(); ++i)
	{
		std::string cmd = i->first;
		
	        if (!Daemon::Match(cmd, key))
	        {
	        	continue;
	        }
	        
		/* Requesting user must have required flags. */
		
		if (i->second->flags)
		{
			if (user->CanPerform(i->second->flags))
			{
	                        std::string fsyntax = i->second->syntax;
	                        
	                        if (fsyntax == NO_SYNTAX)
	                        {
	                        	fsyntax = "";
	                        }
	                        
	                        list.insert(std::make_pair(i->second->name.c_str(), fsyntax.c_str()));
			}
			else
			{
				continue;
			}
		}
		else
		{
		        std::string fsyntax = i->second->syntax;
                                
                        if (fsyntax == NO_SYNTAX)
                        {
                              fsyntax = "";
                        }

                        list.insert(std::make_pair(i->second->name.c_str(), fsyntax.c_str()));
		}
	}

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-30s+%-50s+", Dispatcher::Repeat("-", 31).c_str(), Dispatcher::Repeat("-", 51).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-30s| %-50s|", "Command", "Syntax"));
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-30s+%-50s+", Dispatcher::Repeat("-", 31).c_str(), Dispatcher::Repeat("-", 51).c_str()));
	
	for (std::map<std::string, std::string>::iterator i = list.begin(); i != list.end(); ++i)
	{
		Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-30s| %-50s|", i->first.c_str(), i->second.c_str()), Daemon::Format("%s %s", i->first.c_str(), i->second.c_str()));
	}

	Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-30s+%-50s+", Dispatcher::Repeat("-", 31).c_str(), Dispatcher::Repeat("-", 51).c_str()));
        Dispatcher::JustAPI(user, BRLD_END_LIST);
	return SUCCESS;
}

CommandGetStartup::CommandGetStartup(Module* parent) : Command(parent, "STARTUP", 0, 1)
{
        syntax = "<*f>";
}

COMMAND_RESULT CommandGetStartup::Handle(User* user, const Params& parameters)
{
	if (!parameters.size())
	{
		user->SendProtocol(BRLD_OK, Daemon::HumanEpochTime(Kernel->GetStartup()));
		return SUCCESS;
	}
	
        user->SendProtocol(BRLD_OK, Kernel->GetStartup());
        return SUCCESS;
}
