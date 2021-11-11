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
#include "engine.h"
#include "channelmanager.h"

class CommandForceJoin : public Command
{
 public:
 
	CommandForceJoin(Module* Creator) : Command(Creator, "FORCEJOIN", 1)
	{
		last_empty_ok = false;
		flags = 'e';
		syntax = "[<instance>] <channel>[,<channel>]+";
		INTERPRET2(TR_INSTANCE, TR_TEXT);
	}

	COMMAND_RESULT Handle(User* user, const Params& parameters)
	{
		const unsigned int channelindex = (parameters.size() > 1) ? 1 : 0;
	
		if (CommandHandler::HasLoop(user, this, parameters, channelindex))
		{
			return FAILED;
		}

		const std::string& channel = parameters[channelindex];
		const std::string& instance = parameters.size() > 1 ? parameters[0] : user->instance;

		User* dest = Kernel->Clients->FindInstance(instance);
		
		if ((dest) && (dest->registered == REG_OK))
		{
			if (IS_LOCAL(user) && !Kernel->Engine->ValidChannel(channel))
			{
				user->SendProtocol(ERR_INPUT, INVALID_CHAN);
				return FAILED;
			}

			Channel* chan = Kernel->Channels->Find(channel);
			
			if ((chan) && (chan->IsSubscribed(dest)))
			{
				user->SendRemoteProtocol(ERR_INPUT, PROCESS_ALREADY);
				return FAILED;
			}

			LocalUser* localuser = IS_LOCAL(dest);
			
			if (localuser)
			{
				chan = Channel::JoinUser(false, localuser, channel, true);
	
				if (chan)
				{
					return SUCCESS;
				}
				else
				{
					user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
					return FAILED;
				}
			}
			else
			{
				return SUCCESS;
			}
		}
		else
		{
			user->SendProtocol(ERR_INPUT, PROCESS_NULL);
			return FAILED;
		}
	}

	RouteParams GetRouting(User* user, const Params& parameters) 
	{
		return ROUTE_OPT_UCAST(parameters[0]);
	}
};

class ModuleForceSubs : public Module
{
  private:
  
	CommandForceJoin cmdforcejoin;
 
 public:
 
	ModuleForceSubs() : cmdforcejoin(this)
	{
	
	}

	Version GetDescription() 
	{
		return Version("Adds FORCEJOIN command.", VF_OPTCOMMON | VF_BERYLDB);
	}
};

MODULE_LOAD(ModuleForceSubs)
