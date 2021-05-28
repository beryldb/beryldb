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
#include "brldb/dbmanager.h"
#include "engine.h"

class CommandNotifier : public Command 
{
    public: 

        CommandNotifier(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandStopNotify : public Command 
{

    public: 

        CommandStopNotify(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

class CommandNotifyReset : public Command 
{
    public: 

        CommandNotifyReset(Module* parent);

        COMMAND_RESULT Handle(User* user, const Params& parameters);
};

