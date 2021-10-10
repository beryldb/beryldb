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
#include "core_hints.h"

CommandTransfer::CommandTransfer(Module* Creator) : Command(Creator, "TRANSFER", 2, 2)
{
        check_key       =       0;
        group 		= 	'h';
        syntax 		= 	"<key> <dest database>";
}

COMMAND_RESULT CommandTransfer::Handle(User* user, const Params& parameters)
{  
       const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(parameters[1]);
       
       if (!database)
       {
            user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
            return FAILED; 
       }
       
       if (database == user->GetDatabase())
       {
            user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
            return FAILED;        
       }

       GlobalHelper::Transfer(user, parameters[0], database);
       return SUCCESS;
}

CommandRename::CommandRename(Module* Creator) : Command(Creator, "RENAME", 2, 2)
{
        check_key       =       0;
        check_hash	=	1;
        group  		= 	'h';
        syntax 		= 	"<key> <new key>";
}

COMMAND_RESULT CommandRename::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleRetro(user, std::make_shared<rename_query>(), parameters[0], parameters[1]);
       return SUCCESS;
}

CommandRenameNX::CommandRenameNX(Module* Creator) : Command(Creator, "RENAMENX", 2, 2)
{
        check_key       =       0;
        check_hash	=	1;
        group  		= 	'h';
        syntax 		= 	"<key> <new key>";
}

COMMAND_RESULT CommandRenameNX::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleRetro(user, std::make_shared<renamenx_query>(), parameters[0], parameters[1]);
       return SUCCESS;
}

CommandDel::CommandDel(Module* Creator) : Command(Creator, "DEL", 1)
{
        check_key       =       0;
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandDel::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<del_query>(), parameters[0]);
       return SUCCESS;
}

CommandCopy::CommandCopy(Module* Creator) : Command(Creator, "COPY", 2, 2)
{
        check_key       =       0;
        check_hash	=	1;
        group  		= 	'h';
        syntax 		= 	"<key> <dest key>";
}

COMMAND_RESULT CommandCopy::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleRetro(user, std::make_shared<copy_query>(), parameters[0], parameters[1]);
       return SUCCESS;
}

CommandExists::CommandExists(Module* Creator) : Command(Creator, "EXISTS", 1, 1)
{
        check_key       =       0;
        group  		= 	'h';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandExists::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<exists_query>(), parameters[0]);
       return SUCCESS;
}

CommandMove::CommandMove(Module* Creator) : Command(Creator, "MOVE", 2, 2)
{
        check_key       =       0;
        group  		=	'h';
        syntax 		= 	"<key> <select>";
}

COMMAND_RESULT CommandMove::Handle(User* user, const Params& parameters)
{  
       const std::string& new_select 	= 	parameters[1];

       if (!CheckValidPos(user, new_select))
       {
              return FAILED;
       }

       if (!Daemon::CheckRange(user, new_select, INVALID_RANGE, 1, 100))
       {
               return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<move_query>(), parameters[0], new_select, false);
       return SUCCESS;
}

CommandType::CommandType(Module* Creator) : Command(Creator, "TYPE", 1, 1)
{
        check_key       =       0;
        group 		= 	'h';
}

COMMAND_RESULT CommandType::Handle(User* user, const Params& parameters)
{  
       KeyHelper::SimpleType(user, std::make_shared<type_query>(),  parameters[0], QUERY_TYPE_TYPE);
       return SUCCESS;
}

CommandTouch::CommandTouch(Module* Creator) : Command(Creator, "TOUCH", 1, 1)
{
        group  		= 	'h';
        syntax 		= 	"[keys]";
}

COMMAND_RESULT CommandTouch::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<touch_query>(), "", parameters.back(), false);
        return SUCCESS;
}

CommandNTouch::CommandNTouch(Module* Creator) : Command(Creator, "NTOUCH", 1, 1)
{
        syntax 		= 	"[keys]";
        group  		= 	'h';
}

COMMAND_RESULT CommandNTouch::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<ntouch_query>(), "", parameters.back(), false);
        return SUCCESS;
}

CommandClone::CommandClone(Module* Creator) : Command(Creator, "CLONE", 2, 2)
{
        check_key       =       0;
        group  		= 	'h';
        syntax 		= 	"<key> <select>";
}

COMMAND_RESULT CommandClone::Handle(User* user, const Params& parameters)
{  
       const std::string& value 	= 	parameters[1];

       if (!CheckValidPos(user, value))
       {
              return FAILED;
       }

       if (!Daemon::CheckRange(user, value, INVALID_RANGE, 1, 100))
       {
               return FAILED;
       }

       KeyHelper::SimpleRetro(user, std::make_shared<clone_query>(), parameters[0], value);
       return SUCCESS;
}

CommandDiff::CommandDiff(Module* Creator) : Command(Creator, "DIFF", 2, 4)
{
        run_conf	=	true;
        check_key       =       0;
        group  		= 	'h';
        syntax 		= 	"<key> <select> <offset> <limit>";
}

COMMAND_RESULT CommandDiff::Handle(User* user, const Params& parameters)
{  
       std::shared_ptr<diff_query> query = std::make_shared<diff_query>();
       query->value 			 = parameters[1];

       KeyHelper::RetroLimits(user, query, parameters[0], this->offset, this->limit);
       return SUCCESS;
}
