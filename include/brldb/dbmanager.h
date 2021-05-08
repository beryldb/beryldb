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

#pragma once

#include "brldb/dbflush.h"
#include "brldb/expires.h"

class Externalize CoreManager : public safecast<CoreManager>
{
    public:
    
        std::shared_ptr<CoreDatabase> DB;
        
        /* Opens core database. */
        
        void Open();
        
        CoreManager();
        
        void SetDefaults();
        
        /* Checks if defaults have been set. */
        
        void CheckDefaults();
};

class Externalize StoreManager : public safecast<StoreManager>
{
    public:
            
        /* Time this instance was created. */
        
        time_t instance;
        
        /* Map of active databases. */
        
        DataMap databases;

        /* Handles expiring keys. */
        
        ExpireManager Expires;
        
        /* Constructor. */
        
        StoreManager();
        
        /* Opens threads. */

        void OpenAll();

        std::shared_ptr<Database> Create(const std::string& name, const std::string& path);

        DataFlush Flusher; 

        void Push(std::shared_ptr<query_base> request);
        
        std::shared_ptr<Database> Default;
};