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
#include "brldb/database.h"
#include "brldb/query.h"
#include "brldb/dbnumeric.h"
#include "brldb/expires.h"
#include "helpers.h"

void move_query::Keys()
{
     unsigned int ttl = this->IsExpiring();

     if (ttl > 0)
     {
          this->DelExpire();
          this->WriteExpire(this->key, this->value, ttl);
     }

}

void move_query::Maps()
{

}

void move_query::Multis()
{

}

void move_query::Geos()
{

}

void move_query::Lists()
{

}

void move_query::Run()
{
    if (this->identified == INT_KEY)
    {
          this->Keys();
    } 
    else if (this->identified == INT_MAP)
    {
          this->Maps();
    }
    else if (this->identified == INT_MMAP)
    {
          this->Multis();
    }
    else if (this->identified == INT_GEO)
    {
          this->Geos();
    }
    else if (this->identified == INT_LIST)
    {
          this->Lists();
    }
    
    RocksData result = this->Get(this->dest);
    const std::string& newdest = to_bin(this->key) + ":" + this->value + ":" + this->identified;
    this->Write(newdest, result.value);
    this->Delete(this->dest);
    
}

void move_query::Process()
{
       user->SendProtocol(BRLD_QUERY_OK, PROCESS_OK);
}
