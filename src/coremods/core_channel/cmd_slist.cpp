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
#include "engine.h"
#include "channelmanager.h"
#include "modules/slist.h"

CommandSlist::CommandSlist(Module* parent) : MultiCommand(parent, "SLIST", 0, 0), namesevprov(parent, "event/slist")
{
	flags  = 'e';
	syntax = "[<channel>[,<channel>]+]";
}

COMMAND_RESULT CommandSlist::HandleLocal(LocalUser* user, const Params& parameters)
{
	if (parameters.empty())
	{
		user->SendProtocol(ERR_INPUT, syntax);
		return SUCCESS;
	}

	if (CommandHandler::HasLoop(user, this, parameters, 0))
	{
		return SUCCESS;
	}

        Channel* const chan = Kernel->Channels->Find(parameters[0]);

	if (chan)
	{
		SendSlist(user, chan);
		return SUCCESS;
	}
	
	user->SendProtocol(Protocols::NoChannel(parameters[0]));
	return FAILED;
}

void CommandSlist::SendSlist(LocalUser* user, Channel* chan)
{
	const Channel::SubscriptionMap& members = chan->GetInstances();

	Dispatcher::JustAPI(user, BRLD_START_LIST);

	Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
	Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-22s|", chan->GetName().c_str()));
	Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));

	for (Channel::SubscriptionMap::const_iterator i = members.begin(); i != members.end(); ++i)
	{
		std::string const instance = i->first->instance;
		Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-22s|", instance.c_str()), Daemon::Format("%s", instance.c_str()));
	}

	Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-22s+", Dispatcher::Repeat("-", 23).c_str()));
        Dispatcher::JustAPI(user, BRLD_END_LIST);
}
