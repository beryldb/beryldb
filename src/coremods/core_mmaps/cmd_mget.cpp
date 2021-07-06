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
#include "maker.h"
#include "managers/mmaps.h"
#include "engine.h"
#include "core_mmaps.h"

CommandMGet::CommandMGet(Module* Creator) : Command(Creator, "MGET", 1, 3)
{
         syntax = "<map> <limit> <offset>";
}

COMMAND_RESULT CommandMGet::Handle(User* user, const Params& parameters)
{  
       const std::string& kmap = parameters[0];
       signed int offset;
       signed int limit;

       if (parameters.size() == 2)
       {
             limit = convto_num<signed int>(parameters[1]); 
             offset = 0;
       }
       else if (parameters.size() == 3)
       {
             limit = convto_num<signed int>(parameters[2]); 
             offset = convto_num<signed int>(parameters[1]);
       }
       else
       {
            limit = -1;
            offset = 0;
       }

       MMapsHelper::Get(user, kmap, offset, limit);
       return SUCCESS;
}

CommandMCount::CommandMCount(Module* Creator) : Command(Creator, "MCOUNT", 1, 1)
{
         syntax = "<map>";
}

COMMAND_RESULT CommandMCount::Handle(User* user, const Params& parameters)
{  
       const std::string& kmap = parameters[0];

       MMapsHelper::Count(user, kmap);
       return SUCCESS;
}

CommandMSet::CommandMSet(Module* Creator) : Command(Creator, "MSET", 3, 3)
{
         syntax = "<map> <key> <value>";
}

COMMAND_RESULT CommandMSet::Handle(User* user, const Params& parameters)
{  
       const std::string& kmap  = parameters[0];
       const std::string& key   = parameters[1];
       const std::string& value = parameters.back();

       if (!Daemon::CheckFormat(user, value))
       {
            return FAILED;
       }

       MMapsHelper::Set(user, kmap, key, value);
       return SUCCESS;
}


CommandMIter::CommandMIter(Module* Creator) : Command(Creator, "MKEYS", 1, 3)
{
         syntax = "<map> <limit> <offset>";
}

COMMAND_RESULT CommandMIter::Handle(User* user, const Params& parameters)
{  
       const std::string& kmap = parameters[0];
       std::vector<signed int> lms = GetLimits(user, this->max_params, parameters);

       if (lms[0] == 0)
       {
            return FAILED; 
       }

       signed int offset = lms[1];
       signed int limit = lms[2];

       MMapsHelper::Iter(user, kmap, offset, limit);
       return SUCCESS;
}

CommandMDel::CommandMDel(Module* Creator) : Command(Creator, "MDEL", 2, 2)
{
         syntax = "<map> <key>";
}

COMMAND_RESULT CommandMDel::Handle(User* user, const Params& parameters)
{  
       const std::string& kmap  = parameters[0];
       const std::string& key   = parameters[1];

       MMapsHelper::Del(user, kmap, key);
       return SUCCESS;
}

CommandMSeek::CommandMSeek(Module* Creator) : Command(Creator, "MSEEK", 2, 4)
{
         syntax = "<map> <hash> <offset> <limit>";
}

COMMAND_RESULT CommandMSeek::Handle(User* user, const Params& parameters)
{  
       const std::string& mname = parameters[0];
       const std::string& hesh = parameters[1];

       std::vector<signed int> lms = GetLimits(user, this->max_params, parameters);

       if (lms[0] == 0)
       {
            return FAILED; 
       }

       signed int offset = lms[1];
       signed int limit = lms[2];

       if (!Daemon::CheckFormat(user, hesh))
       {
            return FAILED;
       }

       MMapsHelper::Seek(user, mname, hesh, offset, limit);
       return SUCCESS;
}