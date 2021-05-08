/*
 * BerylDB - A modular database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2015-2021 Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#include "beryl.h"
#include "brldb/dbmanager.h"
#include "engine.h"

class CommandExpire : public Command 
{
    public: 

        CommandExpire(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandTTL : public Command 
{
    public: 

        CommandTTL(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandSetex : public Command 
{
    public: 

        CommandSetex(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandPersist : public Command 
{
    public: 

        CommandPersist(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandExpireCount : public Command 
{
    public: 

        CommandExpireCount(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};


class CommandReset : public Command 
{
    public: 

        CommandReset(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandExpireAT : public Command 
{
    public: 

        CommandExpireAT(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};