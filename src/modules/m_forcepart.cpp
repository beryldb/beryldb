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
#include "channelmanager.h"

class CommandForcePart : public Command
{
 public:
       
        CommandForcePart(Module* Creator) : Command(Creator, "FORCEPART", 2, 2)
        {
                flags = 'e';
                syntax = "<instance> <channel>[,<channel>]";
                INTERPRET3(TR_INSTANCE, TR_TEXT, TR_TEXT);
        }
        
        COMMAND_RESULT Handle(User* user, const Params& parameters)
        {
                if (CommandHandler::HasLoop(user, this, parameters, 1))
                {
                        return FAILED;
                }
                
                User* dest = Kernel->Clients->FindInstance(parameters[0]);
                Channel* channel = Kernel->Channels->Find(parameters[1]);
                
                if (!channel)
                {
                         user->SendRemoteProtocol(ERR_INPUT, PROCESS_NULL);
                         return FAILED;
                }
                
                if ((dest) && (dest->registered == REG_OK) && (channel))
                {
                        if (!channel->IsSubscribed(dest))
                        {
                                user->SendRemoteProtocol(ERR_INPUT, NOT_SUBSCRIBED);
                                return FAILED;
                        }
        
                        if (IS_LOCAL(dest))
                        {
                                channel->PartUser(dest);
                        }
                
                        return SUCCESS;
                }
                else
                {
                        user->SendProtocol(ERR_INPUT, INVALID_CHAN);
                }
                
                return FAILED;
        }
};

class ModuleForcePart : public Module
{ 
  private:
  
        CommandForcePart cmd;
        
 public:
 
        ModuleForcePart() : cmd(this)
        {
        
        }

        Version GetDescription()
        {
                return Version("Adds FORCEPART command.", VF_BERYLDB|VF_OPTCOMMON);
        }
};

MODULE_LOAD(ModuleForcePart)
