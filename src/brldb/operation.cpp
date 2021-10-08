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

#include <math.h>
#include <cstdlib>

#include "beryl.h"
#include "engine.h"
#include "brldb/database.h"
#include "brldb/query.h"
#include "brldb/dbnumeric.h"
#include "brldb/dbmanager.h"
#include "helpers.h"

void dbsize_query::Run()
{
    rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());
    std::string rawmap;
    double size_calc = 0;

    for (it->SeekToFirst(); it->Valid(); it->Next()) 
    {
            if (!Dispatcher::CheckIterator(this))
            {
                    return;
            }
    
            /* We directly count byte size from binary keys/values. */
            
            size_calc += it->key().size() + 2;
            size_calc += it->value().size() + 2;  
    }
    
    float as_mb = size_calc / 1024 / 1024;
    
    if (as_mb <= 1)
    {
         this->size  = size_calc / 1024;
         this->response = "KB";
    }
    else if (as_mb > 1 && as_mb < 1024)
    {
        this->size = as_mb;
        this->response = "MB";
    }
    else if (as_mb > 1024)
    {
          this->size = size_calc / 1024 / 1024 / 1024;
          this->response = "GB";   
    }
    
    this->size = std::trunc(this->size * 10)/10;
    this->SetOK();
}

void dbsize_query::Process()
{
       user->SendProtocol(BRLD_OK, Daemon::Format("%s %s", convto_string(this->size).c_str(), this->response.c_str()));
}

void type_query::Run()
{
       this->SetOK();
}

void type_query::Process()
{
       if (this->identified != PROCESS_NULL)
       {
            user->SendProtocol(BRLD_OK, Helpers::TypeString(this->identified));
       }
       else
       {
            user->SendProtocol(ERR_INPUT, NOT_FOUND);
       }
}

void op_query::Run()
{    
    double real_oper = 0;
    
    std::string oper;
    std::string dbvalue;
    
    if (!this->value.empty())
    {
         oper = this->value;
         real_oper = convto_num<double>(oper);
    }

    RocksData result = this->Get(this->dest);
    this->response = to_string(result.value);
  
    if (!result.status.ok())
    {
          /* dbvalue not found, so we start it at 0 */

          dbvalue = "0";
    }
    else
    {
          dbvalue = to_string(result.value);
    }

    if (!is_number(dbvalue, true))
    {
          this->access_set(DBL_NOT_NUM);
          return;
    }
        
    double real_value = convto_num<double>(dbvalue);

    switch (this->operation)
    {
        case OP_INCR:
            
                real_value++;
        break;
        
        case OP_DECR:
                
                real_value--;
        break;
        
        case OP_MULT:
            
            real_value = real_value * real_oper;
            
        break;
        
        case OP_AVG:

                real_value = (real_value + real_oper) / 2;
                
        break;
        
        case OP_DIV:

                real_value = double(real_value / real_oper);
        break;
        
        
        case OP_ADD:
            
                real_value += real_oper;
        break;
        
        case OP_MIN:
            
                real_value -= real_oper;
        break;
        
        case OP_SQRT:
        
               real_value = std::sqrt(real_value);
        break;
        
        default:
        
         break;
        
    }
    
    std::string inserting = convto_string(real_value);        
    std::string newvalue = to_bin(inserting);
    
    if (this->Write(this->dest, newvalue))
    {
             this->response = inserting;
             this->SetOK();
    }
    else
    {
             access_set(DBL_UNABLE_WRITE);
    }
}

void op_query::Process()
{       
        if (this->GetStatus())
        {
             user->SendProtocol(BRLD_OK, this->response.c_str());
             return;
        }
}

void sflush_query::Run()
{
     rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());

     for (it->SeekToFirst(); it->Valid(); it->Next()) 
     {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }
                
                std::string rawmap = it->key().ToString();
                
                engine::colon_node_stream stream(rawmap);
                std::string token;

                unsigned int strcounter = 0;
                bool skip = false;
                
               while (stream.items_extract(token))
               {
                        if (skip)
                        {
                            break;
                        }
                     
                        switch (strcounter)
                        {
                             case 1:
                             {
                                   if (this->key != token)
                                   {
                                          skip = true;
                                   }
                             }
                             
                             break;
                             
                             default:
                             {
                                 break;   
                             }                                     
                        }
                        
                        strcounter++;
                }
                
                if (skip)
                {
                        continue;
                }
                
                this->Delete(rawmap);
    }    
     
    this->SetOK();	
}

void sflush_query::Process()
{
        user->SendProtocol(BRLD_OK, PROCESS_OK);
        sfalert(user, NOTIFY_DEFAULT, "SFlushed database: %s", user->GetDatabase()->GetName().c_str());
}

void touch_query::Run()
{
        engine::space_node_stream tlist(this->value);
        std::string token;

        unsigned int touch_count = 0;

        while (tlist.items_extract(token))
        {
                unsigned int result =  this->CheckDest(this->select_query, token, this->base_request);

                if (result == 0)
                {
                        continue;
                }

                touch_count++;
        }

        this->counter = touch_count;
        this->SetOK();
}

void touch_query::Process()
{
       user->SendProtocol(BRLD_OK, convto_string(this->counter));
}

void ntouch_query::Run()
{
        engine::space_node_stream tlist(this->value);
        std::string token;
        
        unsigned int n_touch_count = 0;

        while (tlist.items_extract(token))
        {    
                unsigned int result =  this->CheckDest(this->select_query, token, this->base_request);
                
                if (result != 0)
                {
                        continue;
                }

                n_touch_count++;
        }        

        this->counter = n_touch_count;
        this->SetOK();
}

void ntouch_query::Process()
{
       user->SendProtocol(BRLD_OK, convto_string(this->counter));
}

void list_query::Run()
{
       std::map<std::string, unsigned int> result;

       for (std::vector<std::string>::const_iterator iter = TypeRegs.begin(); iter != TypeRegs.end(); ++iter)
       {
              std::string ltype = *iter;
              result[ltype] = 0;
       }

       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());

       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                std::string rawmap = it->key().ToString();
                std::string rawvalue = to_string(it->value().ToString());
                         
                engine::colon_node_stream stream(rawmap);
                std::string token;
                unsigned int strcounter = 0;
                bool skip = false;

                std::string key_as_string;
                
                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {

                             case 1:
                             {
                                   if (convto_string(this->select_query) != token)
                                   {
                                        skip = true;
                                   }
                             }

                             break;
                             
                             case 2:
                             {
                                  result[token]++; 
                             }

                             break;

                             default:
                             {
                                 break;   
                             }
                        }

                        strcounter++;
                }
    }
                
    this->nmap = result;
    this->SetOK();
}

void list_query::Process()
{
        Dispatcher::JustAPI(user, BRLD_START_LIST);
        
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", "Type", "Count"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));

        for (std::map<std::string, unsigned int>::iterator i = this->nmap.begin(); i != this->nmap.end(); ++i)
        {
                 std::string ikey  = Helpers::TypeString(i->first, true);
                 unsigned int item = i->second;

                 Dispatcher::ListDepend(user, BRLD_USER_ITEM, Daemon::Format("| %-16s| %-29s|", ikey.c_str(), convto_string(item).c_str()), Daemon::Format("%s %s", ikey.c_str(), convto_string(item).c_str()));
        }
        
        Dispatcher::JustAPI(user, BRLD_END_LIST);
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
}

void total_query::Run()
{
       unsigned int total_counter = 0;
       
       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());

       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                std::string rawmap = it->key().ToString();
                std::string rawvalue = to_string(it->value().ToString());
                         
                engine::colon_node_stream stream(rawmap);
                std::string token;
                unsigned int strcounter = 0;
                bool skip = false;

                std::string key_as_string;
                
                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {

                             case 1:
                             {
                                   if (convto_string(this->select_query) != token)
                                   {
                                        skip = true;
                                   }
                             }

                             break;
                             
                             default:
                             {
                                 break;   
                             }
                        }

                        strcounter++;
                }
                
                if (skip)
                {
                     continue;
                }
                
                total_counter++;
    }
                
    this->counter = total_counter;
    this->SetOK();
}

void total_query::Process()
{
       user->SendProtocol(BRLD_OK, convto_string(this->counter).c_str());
}

void glist_query::Run()
{
       std::map<std::string, unsigned int> result;

       for (std::vector<std::string>::const_iterator iter = TypeRegs.begin(); iter != TypeRegs.end(); ++iter)
       {
              std::string ltype = *iter;
              result[ltype] = 0;
       }

       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());

       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                std::string rawmap = it->key().ToString();
                std::string rawvalue = to_string(it->value().ToString());
                         
                engine::colon_node_stream stream(rawmap);
                std::string token;
                unsigned int strcounter = 0;
                bool skip = false;

                std::string key_as_string;
                
                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {

                             case 1:
                             {
                             
                             }

                             break;
                             
                             case 2:
                             {
                                  result[token]++; 
                             }

                             break;

                             default:
                             {
                                 break;   
                             }
                        }

                        strcounter++;
                }
    }
                
    this->nmap = result;
    this->SetOK();
}

void glist_query::Process()
{
        Dispatcher::JustAPI(user, BRLD_START_LIST);
        
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", "Type", "Count"));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        
        for (std::map<std::string, unsigned int>::iterator i = this->nmap.begin(); i != this->nmap.end(); ++i)
        {
                 std::string ikey = Helpers::TypeString(i->first, true);
                 unsigned int item = i->second;

                 Dispatcher::ListDepend(user, BRLD_USER_ITEM, Daemon::Format("| %-16s| %-29s|", ikey.c_str(), convto_string(item).c_str()), Daemon::Format("%s %s", ikey.c_str(), convto_string(item).c_str()));
        }
        
        Dispatcher::JustAPI(user, BRLD_END_LIST);
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
}
