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
#include "core_mmaps.h"

CommandMGet::CommandMGet(Module* Creator) : Command(Creator, "MGET", 1, 3)
{
        run_conf	=	true;
        check_key       =       0;
        group 		= 	'x';
        syntax 		= 	"<map> <limit> <offset>";
}

COMMAND_RESULT CommandMGet::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<mget_query>(), parameters[0], this->offset, this->limit);
       return SUCCESS;
}

CommandMCount::CommandMCount(Module* Creator) : Command(Creator, "MCOUNT", 1, 1)
{
        check_key       =       0;
        group 		= 	'x';
        syntax 		= 	"<map>";
}

COMMAND_RESULT CommandMCount::Handle(User* user, const Params& parameters)
{  
       std::shared_ptr<mget_query> query = std::make_shared<mget_query>();
       query->flags 			 = QUERY_FLAGS_COUNT;

       KeyHelper::Retro(user, query, parameters[0]);
       return SUCCESS;
}

CommandMPush::CommandMPush(Module* Creator) : Command(Creator, "MPUSH", 3, 3)
{
       check_value      =       true;
       check_key        =       0;
       check_hash       =       1;
       group 		= 	'x';
       syntax 		= 	"<map> <hash> \"value\"";
}

COMMAND_RESULT CommandMPush::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleHesh(user, std::make_shared<mset_query>(), parameters[0], parameters[1], parameters.back());
       return SUCCESS;
}

CommandMPushNX::CommandMPushNX(Module* Creator) : Command(Creator, "MPUSHNX", 3, 3)
{
       check_value      =       true;
       check_key        =       0;
       check_hash       =       1;
       group 		= 	'v';
       syntax 		= 	"<map> <hash> \"value\"";
}

COMMAND_RESULT CommandMPushNX::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleHesh(user, std::make_shared<msetnx_query>(), parameters[0], parameters[1], parameters.back());
       return SUCCESS;
}

CommandMKeys::CommandMKeys(Module* Creator) : Command(Creator, "MKEYS", 1, 3)
{
        run_conf	=	true;
        group  		= 	'x';
        syntax 		= 	"<map> <limit> <offset>";
}

COMMAND_RESULT CommandMKeys::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<mkeys_query>(), parameters[0], this->offset, this->limit, true);
       return SUCCESS;
}

CommandMDel::CommandMDel(Module* Creator) : Command(Creator, "MDEL", 2, 2)
{
        check_key       =       0;
        check_hash	=	1;
        group 		= 	'x';
        syntax 		= 	"<map> <hash>";
}

COMMAND_RESULT CommandMDel::Handle(User* user, const Params& parameters)
{
       KeyHelper::Simple(user, std::make_shared<mdel_query>(), parameters[0], parameters[1]);
       return SUCCESS;
}

CommandMRepeats::CommandMRepeats(Module* Creator) : Command(Creator, "MREPEATS", 2, 2)
{
        check_key       =       0;
        check_hash      =       1;
        group 		= 	'x';
        syntax 		= 	"<map> <hash>";
}

COMMAND_RESULT CommandMRepeats::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Simple(user, std::make_shared<mrepeats_query>(), parameters[0], parameters[1]);
       return SUCCESS;
}

CommandMVals::CommandMVals(Module* Creator) : Command(Creator, "MVALS", 1, 3)
{
        run_conf	=	true;
        check_key       =       0;
        group 		= 	'x';
        syntax 		= 	"<map> <offset> <limit>";
}

COMMAND_RESULT CommandMVals::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<mvals_query>(), parameters[0], this->offset, this->limit);
       return SUCCESS;
}

CommandMGetAll::CommandMGetAll(Module* Creator) : Command(Creator, "MGETALL", 1, 3)
{
        run_conf	=	true;
        check_key       =       0;
        group 		= 	'x';
        syntax 		= 	"<map> <offset> <limit>";
}

COMMAND_RESULT CommandMGetAll::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<mgetall_query>(), parameters[0], this->offset, this->limit);
       return SUCCESS;
}

CommandMIter::CommandMIter(Module* Creator) : Command(Creator, "MITER", 2, 4)
{
       run_conf		=	true;
       check_key        =       0;
       check_hash       =       1;
       group  		= 	'x';
       syntax 		= 	"<map> <hash> <limit> <offset>";
}

COMMAND_RESULT CommandMIter::Handle(User* user, const Params& parameters)
{  
       std::shared_ptr<miter_query> query = std::make_shared<miter_query>();
       query->value 			  = parameters[1];
       
       KeyHelper::RetroLimits(user, query, parameters[0], this->offset, this->limit);
       return SUCCESS;
}
