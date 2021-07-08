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
#include "brldb/dbnumeric.h"
#include "brldb/query.h"
#include "managers/keys.h"
#include "engine.h"
#include "core_hints.h"
#include "managers/globals.h"

CommandClone::CommandClone(Module* Creator) : Command(Creator, "CLONE", 2, 2)
{
         syntax = "<key> <select>";
}

COMMAND_RESULT CommandClone::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       const std::string& new_select = parameters[1];

       if (!is_number(new_select))
       {
                 user->SendProtocol(ERR_USE, DBL_NOT_NUM, MUST_BE_NUMERIC.c_str());
                 return FAILED;
       }

       if (!is_positive_number(new_select))
       {
                user->SendProtocol(ERR_USE, ERR_MUST_BE_POS_INT, MUST_BE_POSIT.c_str());
                return FAILED;
       }

       if (!Daemon::CheckRange(user, new_select, INVALID_RANGE, 1, 100))
       {
               return FAILED;
       }

       GlobalHelper::Clone(user, key, new_select);
       return SUCCESS;
}
