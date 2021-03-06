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
#include "engine.h"

#include "managers/databases.h"

bool DBHelper::FlushDB(std::shared_ptr<Database> database, bool notify)
{
        Kernel->Store->Flusher->Pause();
        bool result = database->FlushDB();
        Kernel->Store->Flusher->Resume();

        if (notify)
        {
             bprint(DONE, "Database flushed.");
        }
        
        return result;
}

MapData DBHelper::CType(const std::string& key)
{
       std::shared_ptr<type_query> query = std::make_shared<type_query>();
       Helpers::make_cquery(query, key);
       query->Prepare();
       query->Run();
       return MapData(query->access, Helpers::TypeString(query->identified));
}

void DBHelper::DBSize(User* user, std::shared_ptr<Database> db)
{
       std::shared_ptr<dbsize_query> query = std::make_shared<dbsize_query>();
       query->user = user;
       query->database = db;
       
       Kernel->Store->Push(query);
}

void DBHelper::SFlush(User* user, const std::string& key)
{
       std::shared_ptr<sflush_query> query = std::make_shared<sflush_query>();
       Helpers::make_query(user, query, key);
       Kernel->Store->Push(query);
}

void DBHelper::List(User* user)
{
       std::shared_ptr<list_query> query = std::make_shared<list_query>();
       Helpers::make_query(user, query);
       Kernel->Store->Push(query);
}

void DBHelper::GList(User* user)
{
       std::shared_ptr<glist_query> query = std::make_shared<glist_query>();
       Helpers::make_query(user, query);
       Kernel->Store->Push(query);
}

void DBHelper::Total(User* user)
{
       std::shared_ptr<total_query> query = std::make_shared<total_query>();
       Helpers::make_query(user, query);
       
       Kernel->Store->Push(query);
}

void DBHelper::DatabaseReset(User* user, const std::string& dbname)
{
       std::shared_ptr<dbreset_query> query = std::make_shared<dbreset_query>();
       
       query->user 		= user;
       query->database 		= Kernel->Store->DBM->Find(dbname);
       query->key 		= dbname;
       
       Kernel->Store->Push(query);
}
