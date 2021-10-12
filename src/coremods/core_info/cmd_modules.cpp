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
#include "engine.h"

CommandModules::CommandModules(Module* parent) : TargetedCommand(parent, "MODULES")
{
	syntax = "[<server>]";
}

COMMAND_RESULT CommandModules::Handle(User* user, const Params& parameters)
{
	bool for_us = (parameters.empty() || engine::equals(parameters[0], Kernel->Config->ServerName));

	if ((!for_us) || (!IS_LOCAL(user)))
	{
		if (!user->CanPerform('e'))
		{
			user->SendProtocol(ERR_INPUT, ACCESS_DENIED);
			return FAILED;
		}

		if (!for_us)
		{
			return SUCCESS;
		}
	}

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-49s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 50).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-49s|", "Module", "Description"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-49s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 50).c_str()));

        const ModuleHandler::ModuleMap& mods = Kernel->Modules->GetModules();

  	for (ModuleHandler::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
	{
		Module* m = i->second;
		Version V = m->GetDescription();

                bool is_core = false;

		if (IS_LOCAL(user) && user->CanPerform('e'))
		{
			std::string ModFlags("VCO");
			size_t pos = 0;
	
			for (int mult = 1; mult <= VF_OPTCOMMON; mult *= 2, ++pos)
			{
				if (!(V.Flags & mult))
				{
					ModFlags[pos] = '-';
				}
                                else
                                {
                                        if (mult == VF_CORE && is_core == false)
                                        {
                                             is_core = true;
                                        }
                                }
                        }
                        
                        if (is_core)
                        {
                              continue;
                        }

                        Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-16s| %-49s|", m->SourceFile.c_str(), V.description.c_str()), Daemon::Format("%s %s", m->SourceFile.c_str(), V.description.c_str()));
			
		}
		else
		{
			Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-16s| %-49s|", m->SourceFile.c_str(), V.description.c_str()), Daemon::Format("%s %s", m->SourceFile.c_str(), V.description.c_str()));
		}
	}

	Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-49s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 50).c_str()));
        Dispatcher::JustAPI(user, BRLD_END_LIST);
	return SUCCESS;
}

CommandCoreModules::CommandCoreModules(Module* parent) : TargetedCommand(parent, "COREMODULES")
{
	syntax = "[<server>]";
}

COMMAND_RESULT CommandCoreModules::Handle(User* user, const Params& parameters)
{
	bool for_us = (parameters.empty() || engine::equals(parameters[0], Kernel->Config->ServerName));

	if ((!for_us) || (!IS_LOCAL(user)))
	{
                if (!user->CanPerform('e'))
                {
                        user->SendProtocol(ERR_INPUT, ACCESS_DENIED);
                        return FAILED;
                }

		if (!for_us)
		{
			return SUCCESS;
		}
	}

        Dispatcher::JustAPI(user, BRLD_START_LIST);

        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-18s+%-49s+", Dispatcher::Repeat("-", 19).c_str(), Dispatcher::Repeat("-", 50).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-18s| %-49s|", "Module", "Description"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-18s+%-49s+", Dispatcher::Repeat("-", 19).c_str(), Dispatcher::Repeat("-", 50).c_str()));

	const ModuleHandler::ModuleMap& mods = Kernel->Modules->GetModules();

  	for (ModuleHandler::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
	{
		Module* m = i->second;
		Version V = m->GetDescription();
		
		bool is_core = false;
		
		if (IS_LOCAL(user) && user->CanPerform('e'))
		{
			std::string ModFlags("VCO");
			size_t pos = 0;
	
			for (int mult = 1; mult <= VF_OPTCOMMON; mult *= 2, ++pos)
			{
				if (!(V.Flags & mult))
				{
					ModFlags[pos] = '-';
				}
				else
				{
		                        if (mult == VF_CORE && is_core == false)
		                        {
	        	                     is_core = true;
	                	        }
	                        }
			}
			
			if (!is_core)
			{
			      continue;
			}
			
                        Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-18s| %-49s|", m->SourceFile.c_str(), V.description.c_str()), Daemon::Format("%s %s", m->SourceFile.c_str(), V.description.c_str()));
		}
		else
		{
                        Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-18s| %-49s|", m->SourceFile.c_str(), V.description.c_str()), Daemon::Format("%s %s", m->SourceFile.c_str(), V.description.c_str()));
		}
	}

        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-19s+%-49s+", Dispatcher::Repeat("-", 19).c_str(), Dispatcher::Repeat("-", 50).c_str()));
	
        Dispatcher::JustAPI(user, BRLD_END_LIST);
	return SUCCESS;
}
