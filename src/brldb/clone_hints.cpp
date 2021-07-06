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

void clone_query::Keys()
{

}

void clone_query::Lists()
{

}

void clone_query::Multis()
{

}

void clone_query::Geos()
{

}

void clone_query::Maps()
{

}

void clone_query::Run()
{
    RocksData result = this->Get(this->dest);
    const std::string& newdest = this->key + ":" + this->value + ":" + this->identified;
    this->Write(newdest, result.value);

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

}

void clone_query::Process()
{
       user->SendProtocol(BRLD_QUERY_OK, PROCESS_OK);
}