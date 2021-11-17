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
#include "core_vector.h"

CommandVExist::CommandVExist(Module* Creator) : Command(Creator, "VEXISTS", 2, 2)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<vector> \"value\"";
}

COMMAND_RESULT CommandVExist::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Simple(user, std::make_shared<vexist_query>(), parameters[0], parameters.back());
       return SUCCESS;  
}

CommandVPos::CommandVPos(Module* Creator) : Command(Creator, "VPOS", 2, 2)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<vector> <value>";
}

COMMAND_RESULT CommandVPos::Handle(User* user, const Params& parameters)
{  
       const std::string& value 	= 	parameters[1];

       if (!CheckValidPos(user, value))
       {
              return FAILED;
       }
       
       KeyHelper::SimpleRetro(user, std::make_shared<vpos_query>(), parameters[0], value);
       return SUCCESS;  
}

CommandVGet::CommandVGet(Module* Creator) : Command(Creator, "VGET", 1, 3)
{
       run_conf		=	true;
       check_key        =       0;
       group 		= 	'v';
       syntax 		= 	"<key> <offset> <limit>";
}

COMMAND_RESULT CommandVGet::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<vget_query>(), parameters[0], this->offset, this->limit);
       return SUCCESS;  
}

CommandVCount::CommandVCount(Module* Creator) : Command(Creator, "VCOUNT", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVCount::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<vcount_query>(), parameters[0]);
       return SUCCESS;  
}

CommandVResize::CommandVResize(Module* Creator) : Command(Creator, "VRESIZE", 2, 2)
{
       check_key       =       0;
       group 	       =       'v';
       syntax 	       =       "<key> <size>";
}

COMMAND_RESULT CommandVResize::Handle(User* user, const Params& parameters)
{  
       const std::string& value 	= 	parameters[1];

       if (!CheckValidPos(user, value))
       {
              return FAILED;
       }

       KeyHelper::Simple(user, std::make_shared<vresize_query>(), parameters[0], value);
       return SUCCESS;  
}

CommandVKeys::CommandVKeys(Module* Creator) : Command(Creator, "VKEYS", 1, 3)
{
       run_conf		=	true;
       group  		= 	'v';
       syntax 		= 	"<%key> <offset> <limit>";
}

COMMAND_RESULT CommandVKeys::Handle(User* user, const Params& parameters)
{  
       KeyHelper::RetroLimits(user, std::make_shared<vkeys_query>(), parameters[0], this->offset, this->limit, true);
       return SUCCESS;  
}

CommandVDel::CommandVDel(Module* Creator) : Command(Creator, "VDEL", 2, 2)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVDel::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<vdel_query>(), parameters[0], parameters.back());
        return SUCCESS;  
}

CommandVReverse::CommandVReverse(Module* Creator) : Command(Creator, "VREVERSE", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVReverse::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Retro(user, std::make_shared<vreverse_query>(), parameters[0]);
        return SUCCESS;  
}

CommandVRepeats::CommandVRepeats(Module* Creator) : Command(Creator, "VREPEATS", 2, 2)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVRepeats::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<vrepeats_query>(), parameters[0], parameters.back());
        return SUCCESS;  
}

CommandVSort::CommandVSort(Module* Creator) : Command(Creator, "VSORT", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVSort::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Retro(user, std::make_shared<vsort_query>(), parameters[0]);
        return SUCCESS;  
}

CommandVPush::CommandVPush(Module* Creator) : Command(Creator, "VPUSH", 2, 2)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVPush::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<vpush_query>(), parameters[0], parameters.back());
        return SUCCESS;  
}

CommandVPopFront::CommandVPopFront(Module* Creator) : Command(Creator, "VPOPFRONT", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVPopFront::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Retro(user, std::make_shared<vpop_front_query>(), parameters[0]);
        return SUCCESS;  
}

CommandVPopBack::CommandVPopBack(Module* Creator) : Command(Creator, "VPOPBACK", 1, 1)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVPopBack::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Retro(user, std::make_shared<vpop_back_query>(), parameters[0]);
        return SUCCESS;  
}

CommandVAvg::CommandVAvg(Module* Creator) : Command(Creator, "VAVG", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVAvg::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<vavg_query>(), parameters[0]);
       return SUCCESS;  
}

CommandVHigh::CommandVHigh(Module* Creator) : Command(Creator, "VHIGH", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVHigh::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<vhigh_query>(), parameters[0]);
       return SUCCESS;  
}

CommandVLow::CommandVLow(Module* Creator) : Command(Creator, "VLOW", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVLow::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<vlow_query>(), parameters[0]);
       return SUCCESS;  
}

CommandVSum::CommandVSum(Module* Creator) : Command(Creator, "VSUM", 1, 1)
{
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key>";
}

COMMAND_RESULT CommandVSum::Handle(User* user, const Params& parameters)
{  
       KeyHelper::Retro(user, std::make_shared<vsum_query>(), parameters[0]);
       return SUCCESS;  
}

CommandVPushNX::CommandVPushNX(Module* Creator) : Command(Creator, "VPUSHNX", 2, 2)
{
        check_value     =       true;
        check_key       =       0;
        group 		= 	'v';
        syntax 		= 	"<key> \"value\"";
}

COMMAND_RESULT CommandVPushNX::Handle(User* user, const Params& parameters)
{  
        KeyHelper::Simple(user, std::make_shared<vpushnx_query>(), parameters[0], parameters.back());
        return SUCCESS;  
}

CommandVFind::CommandVFind(Module* Creator) : Command(Creator, "VFIND", 2, 4)
{
        run_conf	=	true;
        check_value     =       true;
        check_key       =       0;
        group 		= 	'l';
        syntax 		= 	"<key> \"%value\" <offset> <limit>";
}

COMMAND_RESULT CommandVFind::Handle(User* user, const Params& parameters)
{  
       std::shared_ptr<vdel_query> query = std::make_shared<vdel_query>();
       query->value 			 = stripe(parameters.back());
       
       KeyHelper::RetroLimits(user, query, parameters[0], this->offset, this->limit);
       return SUCCESS;  
}
