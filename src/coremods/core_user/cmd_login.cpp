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

CommandLogin::CommandLogin(Module* parent) : MultiCommand(parent, "LOGIN", 1, 1)
{
	pre_reg_ok 		= 	true;
	no_hint_until_reg 	= 	true;
	check_login		=	0;
	syntax 			= 	"<login>";
}

COMMAND_RESULT CommandLogin::HandleLocal(LocalUser* user, const Params& parameters)
{
	const std::string& newlogin = parameters[0];

	ModuleResult MOD_RESULT;
	UNTIL_RESULT(OnUserPreLogin, MOD_RESULT, (user, newlogin));

	if (MOD_RESULT == MOD_RES_STOP)
	{
		return FAILED;
	}

	if (!user->SetLogin(newlogin))
	{
		return FAILED;
	}

        /* Default database assignation. */

        const std::string& dbuser = STHelper::Get("dbuser", newlogin);
        
        if (!dbuser.empty())
        {
                const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(dbuser);

                if (!database)
                {
	                user->SetDatabase(Kernel->Store->GetDefault());
                }
                else
                {
	                user->SetDatabase(database);
                }
        }
        else
        {
                user->SetDatabase(Kernel->Store->GetDefault());
        }

        if (user->registered < REG_LOGINUSER)
        {
                user->registered = (user->registered | REG_LOGIN);
                return CommandAgent::CheckRegister(user);
        }

	return SUCCESS;
}

CommandAuth::CommandAuth(Module* parent) : MultiCommand(parent, "AUTH", 1, 1)
{
	pre_reg_ok 	= true;
	syntax 		= "<password>";
}

COMMAND_RESULT CommandAuth::HandleLocal(LocalUser* user, const Params& parameters)
{
	const std::string& password = parameters[0];
	
	if (password.length() < 3 || password.length() > 100)
	{
		/* Password must be of at least 3 characters */
		
		Kernel->Clients->Disconnect(user, PASS_MIMMAX_LENGTH);	
		return FAILED;
	}
	else 
	{
		user->auth = password;
	}

	return SUCCESS;
}

CommandILogin::CommandILogin(Module* parent) : Command(parent, "ILOGIN", 3, 3)
{
	pre_reg_ok 	= 	true;
	syntax 		= 	"<agent> <auth> <login>";
}

COMMAND_RESULT CommandILogin::Handle(User* user, const Params& parameters)
{
	if (user->GetLogged())
	{
		user->SendProtocol(ERR_INPUT, ALREADY_LOGGED);
		return FAILED;
	}
	
        const std::string& agent   	 =  parameters[0];
        const std::string& auth    	 =  parameters[1];
        const std::string& login   	 =  parameters[2];
        
        const std::string& AGENT_CMD     =  "AGENT";
        const std::string& AUTH_CMD      =  "AUTH";
        const std::string& LOGIN_CMD     =  "LOGIN";
        
	CommandModel::Params params;
        CommandModel::Params params2;
        CommandModel::Params params3;
	
	params.push_back(agent);
        params2.push_back(auth);
        params3.push_back(login);

	LocalUser* luser           =  IS_LOCAL(user);
	
        Kernel->Commander->Queue->Add(luser, AGENT_CMD, params);
        Kernel->Commander->Queue->Add(luser, AUTH_CMD, params2);
        Kernel->Commander->Queue->Add(luser, LOGIN_CMD, params3);
        return SUCCESS;
}

CommandExit::CommandExit(Module* parent) : Command(parent, "EXIT", 0) 
{
        pre_reg_ok = true;
}

COMMAND_RESULT CommandExit::Handle(User* user, const Params& parameters)
{
        user->SendProtocol(BRLD_QUITTING);
        Kernel->Clients->Disconnect(user);
        return SUCCESS;
}

RouteParams CommandExit::GetRouting(User* user, const Params& parameters)
{
        return (IS_LOCAL(user) ? ROUTE_LOCALONLY : ROUTE_BROADCAST);
}


CommandQ::CommandQ(Module* parent) : Command(parent, "Q", 0)
{
        alias = "EXIT";
}

COMMAND_RESULT CommandQ::Handle(User* user, const Params& parameters)
{
        return SUCCESS;
}

CommandQuit::CommandQuit(Module* parent) : Command(parent, "QUIT", 0)
{
	alias = "EXIT";
}

COMMAND_RESULT CommandQuit::Handle(User* user, const Params& parameters)
{
	return SUCCESS;
}
