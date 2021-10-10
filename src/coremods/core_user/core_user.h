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

#pragma once

#include "beryl.h"
#include "modules/message.h"
#include "managers/settings.h"
#include "brldb/dbmanager.h"

/* 
 * Logins an user into the system.
 * 
 * @parameters:
 *
 *             · string         : Login to submit.
 */ 


class CommandLogin : public MultiCommand
{
   public:
	
	CommandLogin(Module* parent);
	
	COMMAND_RESULT HandleLocal(LocalUser* user, const Params& parameters);
};

/* 
 * Authenticates a given user.
 * 
 * @parameters:
 *
 *             · string	 	: Password to use.
 */ 

class CommandAuth : public MultiCommand
{
   public:
	
	CommandAuth(Module* parent);

	COMMAND_RESULT HandleLocal(LocalUser* user, const Params& parameters) ;
};

/* 
 * Disconnects user from server.
 * 
 * @protocol:
 *
 *         · enum: OK or ERROR.
 */ 

class CommandExit : public Command
{
 public:
 
	MessageWrapper msgwrap;
	
	CommandExit(Module* parent);
	
	COMMAND_RESULT Handle(User* user, const Params& parameters);

	RouteParams GetRouting(User* user, const Params& parameters);
};

/* 
 * Sets an agent for an user. Keep in mind that this command may be used
 * only once.
 *
 * NOTE: A valid agent must be have at least 3 characters, and no more than 15.
 * 
 * @protocol:
 *
 *         · enum   : OK or ERROR.
 */ 
 
class CommandAgent : public MultiCommand
{
    public:

	CommandAgent(Module* parent);
	
	COMMAND_RESULT HandleLocal(LocalUser* user, const Params& parameters) ;
	
	static COMMAND_RESULT CheckRegister(LocalUser* user);
};

/* 
 * ILogin sends 3 commands in one single line: AGENT, AUTH and LOGIN.
 * 
 * @protocol:
 *
 *         · enum   : OK or ERROR.
 */ 
 
class CommandILogin : public Command
{
   public:
        
        CommandILogin(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);

};

/* 
 * Alias command for quit.
 * 
 * @protocol:
 *
 *         · enum   : OK or ERROR.
 */ 

class CommandQuit : public Command
{
 public:

        CommandQuit(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

/* 
 * Alias command for quit.
 * 
 * @protocol:
 *
 *         · enum   : OK or ERROR.
 */ 

class CommandQ : public Command
{
 public:

        CommandQ(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};