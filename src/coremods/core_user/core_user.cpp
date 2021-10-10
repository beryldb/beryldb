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
#include "core_user.h"

class CommandPing : public MultiCommand
{
 public:

	CommandPing(Module* parent) : MultiCommand(parent, "PING", 1)
	{
		syntax = "<cookie> [<servername>]";
	}

	COMMAND_RESULT HandleLocal(LocalUser* user, const Params& parameters) 
	{
		size_t origin = parameters.size() > 1 ? 1 : 0;

		if (parameters[origin].empty())
		{
			user->SendProtocol(ERR_NO_ORIGIN, "No origin specified");
			return FAILED;
		}

		ProtocolTrigger::Messages::Pong pong(parameters[0], origin ? parameters[1] : "");
		user->Send(Kernel->GetBRLDEvents().pong, pong);
		return SUCCESS;
	}
};

class CommandPong : public Command
{
 public:

	CommandPong(Module* parent) : Command(parent, "PONG", 1)
	{
		syntax = "<cookie> [<servername>]";
	}


	COMMAND_RESULT Handle(User* user, const Params& parameters) 
	{
		size_t origin = parameters.size() > 1 ? 1 : 0;

		if (parameters[origin].empty())
		{
			user->SendProtocol(ERR_NO_ORIGIN, "No origin specified");
			return FAILED;
		}

		LocalUser* localuser = IS_LOCAL(user);

		if (localuser)
		{
				localuser->lastping = 1;
		}
		
		return SUCCESS;
	}
};

void MessageWrapper::Wrap(const std::string& message, std::string& out)
{
	out.assign(prefix);

	if (!fixed)
	{
		out.append(message).append(suffix);
	}
}

void MessageWrapper::ConfigReading(const char* prefixname, const char* suffixname, const char* fixedname)
{
	config_rule* tag = Kernel->Config->GetConf("options");
	prefix = tag->as_string(fixedname);
	fixed = (!prefix.empty());
	
	if (!fixed)
	{
		prefix = tag->as_string(prefixname);
		suffix = tag->as_string(suffixname);
	}
}

class CoreModUser : public Module
{
  private:
 
	CommandILogin 		cmdilogin;
	CommandLogin 		cmdlogin;
	CommandPing 		cmdping;
	CommandPong 		cmdpong;
	CommandExit 		cmdexit;
	CommandQuit		cmdquit;
	CommandAgent 		cmdagent;
	CommandAuth 		cmdauth;
	CommandQ		cmdq;	

  public:
 
	CoreModUser() :   cmdilogin(this)
	                , cmdlogin(this)
			, cmdping(this)
			, cmdpong(this)
			, cmdexit(this)
			, cmdquit(this)
			, cmdagent(this)
			, cmdauth(this)
			, cmdq(this)

	{
	
	}

	Version GetDescription() 
	{
		return Version("Provides user-related commands.", VF_BERYLDB|VF_CORE);
	}
};

MODULE_LOAD(CoreModUser)
