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
#include "brldb/expires.h"
#include "helpers.h"

void renamenx_query::Vectors()
{

}

void renamenx_query::Lists()
{

}

void renamenx_query::Keys()
{
     RocksData result = this->Get(this->dest);
     const std::string& newdest = to_bin(this->value) + ":" + convto_string(this->select_query) + ":" + this->identified;

     if (!this->SwapWithExpire(newdest, this->dest, result.value, this->select_query, this->value, this->id, this->key))
     {
          access_set(DBL_BATCH_FAILED);
     }
     else
     {
         this->SetOK();
     }
}

void renamenx_query::Multis()
{

}

void renamenx_query::Maps()
{

}

void renamenx_query::Geos()
{

}

void renamenx_query::Run()
{
    if (this->identified == INT_KEY)
    {
          signed int ttl = this->IsExpiring();

          if (ttl > 0)
          {
               this->id = ttl;
               this->Keys();
               return;
          }
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
    else if (this->identified == INT_VECTOR)
    {    
          this->Vectors();
    }   

    RocksData result = this->Get(this->dest);
    const std::string& newdest = to_bin(this->value) + ":" + convto_string(this->select_query) + ":" + this->identified;

    if (!this->Swap(newdest, this->dest, result.value))
    {
          access_set(DBL_BATCH_FAILED);
    }
    else
    {
         this->SetOK();
    }
}

void renamenx_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}
