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
#include "devel.h"
#include "extras.h"
#include "engine.h"

#ifdef ENABLE_TESTOFFICE

#include <iostream>

#include "exit.h"

#include "testoffice.h"
#include "managers/keys.h"
#include "managers/maps.h"
#include "extras.h"
#include "brldb/diff.h"
#include "brldb/database.h"
#include "managers/user.h"
#include "brldb/list_handler.h"
#include "brldb/map_handler.h"
#include "brldb/multimap_handler.h"
#include "managers/settings.h"

TestOffice::TestOffice()
{

}

TestOffice::~TestOffice()
{

}

void TestOffice::Run()
{
        std::shared_ptr<MultiMapHandler> handler3 = std::make_shared<MultiMapHandler>();
        
        handler3->Add("key", "value");
        handler3->Add("key", "value3");
        handler3->Add("key2", "value2");

        handler3->Dump();
        
        STHelper::Set("tests", "hello", "world");
        STHelper::Set("tests", "hello3", "world3");
        STHelper::Set("tests", "hello6", "world6");

        StringVector items = STHelper::HList("tests");
       
        for (StringVector::iterator i = items.begin(); i != items.end(); ++i)
        {
             std::string item_list = *i;
             std::cout << "item: " <<  item_list << std::endl;
        }
       
        std::shared_ptr<ListHandler> handler = std::make_shared<ListHandler>();

        handler->Add("test4");

        handler->Add("test");
        handler->Add("test2");
        handler->Add("test2");
        handler->Add("test3");
        handler->Add("test3");

        std::cout << "pos: " << handler->Index(3) << std::endl;
        std::cout << "Repeats: " << handler->Repeats("test2") << std::endl;
       
        handler->Reverse();
        
        handler->Sort();
        
        std::shared_ptr<ListHandler> handler2 = ListHandler::Create(handler->as_string());
        handler2->Dump();
        
        const std::string& ntest = "300";
        
        std::cout << "is_zero_or_great (" << ntest << ") : " << is_zero_or_great(ntest) << std::endl;
        
        /* Let's verify if a given string is in the right format. */
        
        const std::string& test = "\"hello world\"";

        std::cout << "is_correct: " << is_correct(test) << std::endl;
        std::cout << "stripe: " << stripe(test) << std::endl;
        
        std::cout << "count occur: " << count_occur("testtesttest", "test") << std::endl;

        /* Simulate an auto join list. */

        const std::string& username = "nobody";
        const std::string& userchans = username + "/chans";
        
        STHelper::Set(userchans, "#test1", convto_string(Kernel->Now()));
        STHelper::Set(userchans, "#test2", convto_string(Kernel->Now()));
        
        StringVector chans = STHelper::HList(userchans);

        for (StringVector::iterator i = chans.begin(); i != chans.end(); i++)
        {
                       std::string channel = *i;

                       if (!Kernel->Engine->ValidChannel(channel))
                       {
                              continue;
                       }
                       
                       std::cout << username << " channel: " << channel << std::endl;
        }
       
       /* Remove user channels. */
       
       STHelper::Erase(userchans);
}

#endif