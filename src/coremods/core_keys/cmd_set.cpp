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
#include "core_keys.h"

CommandSet::CommandSet(Module* Creator) : Command(Creator, "SET", 2, 2)
{
         group = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandSet::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	 parameters[0];
       const std::string& value 	=	 parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<set_query>(), key, value);
       return SUCCESS;
}

CommandSetNX::CommandSetNX(Module* Creator) : Command(Creator, "SETNX", 2, 2)
{
         group = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandSetNX::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<setnx_query>(), key, value);
       return SUCCESS;
}

CommandSetTX::CommandSetTX(Module* Creator) : Command(Creator, "SETTX", 2, 2)
{
         groups = { 'e', 'k' };
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandSetTX::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<settx_query>(), key, value);
       return SUCCESS;
}

CommandGetSet::CommandGetSet(Module* Creator) : Command(Creator, "GETSET", 2, 2)
{
         group = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandGetSet::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<getset_query>(), key, value);
       return SUCCESS;
}


CommandAppend::CommandAppend(Module* Creator) : Command(Creator, "APPEND", 2, 2)
{
         group  = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandAppend::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<append_query>(), key, value);
       return SUCCESS;
}

CommandCount::CommandCount(Module* Creator) : Command(Creator, "COUNT", 0, 1)
{
       group  = 'k';
       syntax = "<\%key>";
}

COMMAND_RESULT CommandCount::Handle(User* user, const Params& parameters)
{  
       std::string key;

       /* We use a wildcardcard if no key is provided. */
       
       if (!parameters.size())
       {
            key = "*";
       }
       else
       {
             key = parameters[0];
       }
       
       KeyHelper::Retro(user, std::make_shared<count_query>(), key, true);
       return SUCCESS;
}

CommandGetOccurs::CommandGetOccurs(Module* Creator) : Command(Creator, "OCCURS", 2, 2)
{
         group = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandGetOccurs::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		=	 parameters[0];
       const std::string& value 	= 	 parameters.back(); 

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<get_occurs_query>(), key, value);
       return SUCCESS;
}

CommandGet::CommandGet(Module* Creator) : Command(Creator, "GET", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandGet::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       
       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       KeyHelper::Retro(user, std::make_shared<get_query>(), key);
       return SUCCESS;
}

CommandStrlen::CommandStrlen(Module* Creator) : Command(Creator, "STRLEN", 1, 1)
{
         group  = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandStrlen::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       
       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       KeyHelper::Retro(user, std::make_shared<strlen_query>(), key);
       return SUCCESS;
}

CommandGetDel::CommandGetDel(Module* Creator) : Command(Creator, "GETDEL", 1, 1)
{
         group  = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandGetDel::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       KeyHelper::Retro(user, std::make_shared<getdel_query>(), key);
       return SUCCESS;
}

CommandGetSubstr::CommandGetSubstr(Module* Creator) : Command(Creator, "SUBSTR", 3, 3)
{
         group  = 'k';
         syntax = "<key> <from> <to>";
}

COMMAND_RESULT CommandGetSubstr::Handle(User* user, const Params& parameters)
{
       const std::string& key       =     parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       int limit 		    =     convto_num<int>(parameters[2]); 
       int offset 		    =     convto_num<int>(parameters[1]);

       KeyHelper::RetroLimits(user, std::make_shared<get_substr_query>(), key, offset, limit);
       return SUCCESS;
}

CommandGetExp::CommandGetExp(Module* Creator) : Command(Creator, "GETEXP", 2, 2)
{
         groups = { 'e', 'k' };
         syntax = "<seconds> <key>";
}

COMMAND_RESULT CommandGetExp::Handle(User* user, const Params& parameters)
{  
       const std::string& seconds 	= 	parameters[0];
       const std::string& key 		= 	parameters[1];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       if (!CheckValidPos(user, parameters[0]))
       {
              return FAILED;
       }

       KeyHelper::IDRetro(user, std::make_shared<getexp_query>(), key, seconds);
       return SUCCESS;
}

CommandIsAlpha::CommandIsAlpha(Module* Creator) : Command(Creator, "ISALPHA", 1, 1)
{
         group  = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandIsAlpha::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       
       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       KeyHelper::Retro(user, std::make_shared<alpha_query>(), key);
       return SUCCESS;
}

CommandIsNum::CommandIsNum(Module* Creator) : Command(Creator, "ISNUM", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandIsNum::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       
       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       KeyHelper::Retro(user, std::make_shared<isnum_query>(), key);
       return SUCCESS;
}

CommandGetPersist::CommandGetPersist(Module* Creator) : Command(Creator, "GETPERSIST", 1, 1)
{
         groups = { 'e', 'k' };
         syntax = "<key>";
}

COMMAND_RESULT CommandGetPersist::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];
       
       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       KeyHelper::Retro(user, std::make_shared<getpersist_query>(), key);
       return SUCCESS;
}

CommandWDel::CommandWDel(Module* Creator) : Command(Creator, "WDEL", 1, 1)
{
         group = 'k';
         syntax = "<%key>";
}

COMMAND_RESULT CommandWDel::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];

       if (!CheckFormat(user, key))
       {
            return FAILED;
       }

       KeyHelper::Retro(user, std::make_shared<wdel_query>(), stripe(key), true);
       return SUCCESS;
}

CommandIsBool::CommandIsBool(Module* Creator) : Command(Creator, "ISBOOL", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandIsBool::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       KeyHelper::Retro(user, std::make_shared<isbool_query>(), key);
       return SUCCESS;
}

CommandAsBool::CommandAsBool(Module* Creator) : Command(Creator, "ASBOOL", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandAsBool::Handle(User* user, const Params& parameters)
{  
       const std::string& key = parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       KeyHelper::Retro(user, std::make_shared<asbool_query>(), key);
       return SUCCESS;
}

CommandIsMatch::CommandIsMatch(Module* Creator) : Command(Creator, "ISMATCH", 2, 2)
{
         group = 'k';
         syntax = "<key> \"value\"";
}

COMMAND_RESULT CommandIsMatch::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }
       
       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<ismatch_query>(), key, value);
       return SUCCESS;
}

CommandInsert::CommandInsert(Module* Creator) : Command(Creator, "INSERT", 3, 3)
{
         group = 'k';
         syntax = "<key> <where> \"value\"";
}

COMMAND_RESULT CommandInsert::Handle(User* user, const Params& parameters)
{  
       const std::string& key 		= 	parameters[0];
       const std::string& where 	= 	parameters[1];
       const std::string& value 	= 	parameters.back();

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       if (!CheckFormat(user, value))
       {
            return FAILED;
       }

        if (!is_number(where, false))
        {
                user->SendProtocol(ERR_INPUT, MUST_BE_NUMERIC);
                return FAILED;
        }

        std::shared_ptr<insert_query> query = std::make_shared<insert_query>();
        query->id 			    = convto_num<unsigned int>(where);
        
        KeyHelper::Simple(user, query, key, value, true);
        return SUCCESS;
}

CommandToLower::CommandToLower(Module* Creator) : Command(Creator, "TOLOWER", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandToLower::Handle(User* user, const Params& parameters)
{  
       const std::string& key           =       parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       std::shared_ptr<modify_query> query = std::make_shared<modify_query>();
       query->function = STR_TO_LOW;
       KeyHelper::Retro(user, query, key);
       
       return SUCCESS;
}

CommandToUpper::CommandToUpper(Module* Creator) : Command(Creator, "TOUPPER", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandToUpper::Handle(User* user, const Params& parameters)
{  
       const std::string& key           =       parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       std::shared_ptr<modify_query> query = std::make_shared<modify_query>();
       query->function 			   = STR_TO_UPPER;
       
       KeyHelper::Retro(user, query, key);
       return SUCCESS;
}

CommandToCap::CommandToCap(Module* Creator) : Command(Creator, "TOCAP", 1, 1)
{
         group = 'k';
         syntax = "<key>";
}

COMMAND_RESULT CommandToCap::Handle(User* user, const Params& parameters)
{  
       const std::string& key           =       parameters[0];

       if (!CheckKey(user, key))
       {
            return FAILED;
       }

       std::shared_ptr<modify_query> query = std::make_shared<modify_query>();
       query->function 			   = STR_TO_CAP;
  
       KeyHelper::Retro(user, query, key);
       return SUCCESS;
}


