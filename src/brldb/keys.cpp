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
#include "extras.h"

#include "brldb/expires.h"
#include "brldb/functions.h"

void expire_list_query::Run()
{
       unsigned int total_counter = 0;

       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());
       
       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                std::string key_as_string;
                std::string key_as_bin;
                std::string select_as_string;
                std::string db_name;
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
                             case 0:
                             {
                                  key_as_string = to_string(token);
                                  key_as_bin = token;
                             }

                             break;

                             case 1:
                             {
                                  select_as_string = token;    
                             }

                             break;
                             
                             case 3:
                             {
                                  db_name = token;
                                  
                                  if (db_name.empty() || db_name != this->database->GetName())
                                  {
                                       skip = true;
                                  }
                             }
                             
                             break;
                             
                             case 2:
                             {
                                   if (token != INT_EXPIRE)
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

                std::string rawvalue = it->value().ToString();
                
                if (Kernel->Config->KeepExpires)
                {
                     Kernel->Store->Expires->Add(this->database, convto_num<signed int>(rawvalue), key_as_string, convto_num<unsigned int>(select_as_string), true);
                     total_counter++;
                }
                else
                {
                     this->Delete(rawmap);
                }
        }
        
        if (Kernel->Config->KeepExpires && total_counter > 0)
        {
             iprint((int)total_counter, "Expires loaded from %s.", this->database->GetName().c_str());
        }
}

void expire_list_query::Process()
{
        return;
}

void setex_query::Run()
{
       this->ExpireBatch(this->dest, this->value, this->key, this->select_query, this->id);
       this->SetOK();
}

void setex_query::Process()
{
       NOTIFY_MODS(OnExpireAdd, (this->user, this->database->GetName(), this->key, this->select_query, this->id));
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void expireat_query::Run()
{
       this->WriteExpire(this->key, this->select_query, this->id);
       this->SetOK();
}

void expireat_query::Process()
{
       NOTIFY_MODS(OnExpireAdd, (this->user, this->database->GetName(), this->key, this->select_query, this->id));
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void expire_del_query::Run()
{	
      this->DelExpire();
      this->SetOK();
}

void expire_del_query::Process()
{
        if (this->flags != QUERY_FLAGS_QUIET)
        {
                user->SendProtocol(BRLD_OK, PROCESS_OK);
        }
}

void set_query::Run()
{
       if (this->Write(this->dest, to_bin(this->value)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void set_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);    
}

void get_substr_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       
       if ((unsigned int)(this->offset + this->limit) > this->response.length())
       {	
             access_set(DBL_INVALID_RANGE);
             return;
       }
       
       this->response = this->response.substr(this->offset, this->limit);
       
       if (this->Write(this->dest, to_bin(this->response)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void get_substr_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response).c_str());
}

void modify_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       
       if (this->function == STR_TO_UPPER)
       {
            std::transform(this->response.begin(), this->response.end(), this->response.begin(), ::toupper);
       }
       else if (this->function == STR_TO_LOW)
       {
            std::transform(this->response.begin(), this->response.end(), this->response.begin(), ::tolower);
       }
       else if (this->function == STR_TO_CAP)
       {
            if (!this->response.empty())
            {
                this->response[0] = ::toupper(this->response[0]);
            }
       }
 
       if (this->Write(this->dest, to_bin(this->response)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void modify_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void get_occurs_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       this->response = convto_string(count_occur(this->response, this->value));
       this->SetOK();
}

void get_occurs_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response.c_str());
}

void alpha_query::Run()
{
       RocksData result = this->Get(this->dest);

       if (isalpha(to_string(result.value)))
       {
             this->response = "1";
       }
       else
       {
            this->response = "0";
       }
       
       this->SetOK();
}

void alpha_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void char_query::Run()
{
       RocksData result = this->Get(this->dest);

       const std::string& found = to_string(result.value);
       const unsigned int flength = convto_num<unsigned int>(this->value);
      
       if (flength >= found.length())
       {
             access_set(DBL_INVALID_RANGE);
             return;
       }
       else
       {
            this->response = found[flength];
       }

       this->SetOK();
}

void char_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void isnum_query::Run()
{
       RocksData result = this->Get(this->dest);

       if (is_number(to_string(result.value), true))
       {
             this->response = "1";
       }
       else
       {
            this->response = "0";
       }
       
       this->SetOK();
}

void isnum_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void isbool_query::Run()
{
       RocksData result = this->Get(this->dest);
       const std::string& as_str = to_string(result.value);
       
       if (as_str == "0" || as_str == "1" || as_str == "true" || as_str == "false" || as_str == "on" || as_str == "off")
       {
             this->response = "1";
       }
       else
       {
            this->response = "0";
       }
       
       this->SetOK();
}

void isbool_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void getpersist_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       
       this->DelExpire();
       this->SetOK();
}

void getpersist_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}

void get_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       this->SetOK();
}

void get_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}

void strlen_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = convto_string(to_string(result.value).length());
       this->SetOK();
}

void strlen_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response.c_str());
}

void count_query::Run()
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
                             case 0:
                             {
                                  key_as_string = to_string(token);
                                  
                                  if (!Daemon::Match(key_as_string, this->key))
                                  {
                                        skip = true;
                                  }
                             }

                             break;

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
                                  if (token != INT_KEY)
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

void count_query::Process()
{
       user->SendProtocol(BRLD_OK, convto_string(this->counter));
}

void getdel_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       this->Delete(this->dest);
       this->SetOK();
}

void getdel_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}

void getset_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);

       if (this->Write(this->dest, to_bin(this->value)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void getset_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response.c_str()));
}

void wdel_query::Run()
{
       unsigned int total_counter = 0;

       std::string rawmap;
       
       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());

       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                rawmap = it->key().ToString();
                std::string key_as_string;
                std::string token;
                bool skip = false;
                
                unsigned int strcounter = 0;
                
                engine::colon_node_stream stream(rawmap);
                
                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {
                             case 0:
                             {
                                  key_as_string = to_string(token);
                                  
                                  if (!Daemon::Match(key_as_string, this->key))
                                  {
                                        skip = true;
                                  }
                             }

                             break;

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
                                  if (token != INT_KEY)
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
                Kernel->Store->Expires->Delete(this->database, key_as_string, this->select_query);
                this->DelExpire();
                total_counter++;
       }
       
       this->counter = total_counter;
       this->SetOK();
}

void wdel_query::Process()
{
       user->SendProtocol(BRLD_OK, convto_string(this->counter));
}

void setnx_query::Run()
{
       RocksData result = this->Get(this->dest);
       
       if (!result.status.ok())
       {
            if (this->Write(this->dest, to_bin(this->value)))
            {
                this->SetOK();
            }
            else
            {
                 access_set(DBL_UNABLE_WRITE);
            }
            
            return;
       }
       
       access_set(DBL_ENTRY_EXISTS);
}

void setnx_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void settx_query::Run()
{
       RocksData result = this->Get(this->dest);
       
       if (this->IsExpiring() < 0)
       {
            this->Write(this->dest, to_bin(this->value));
            this->SetOK();
            return;
       }
       
       access_set(DBL_ENTRY_EXPIRES);
}

void settx_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void search_query::Run()
{
       DualMMap result;

       unsigned int total_counter = 0;
       unsigned int aux_counter = 0;
       unsigned int tracker = 0;

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
                
                if (!Daemon::Match(rawvalue, this->key))
                {
                        continue;
                }

                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {
                             case 0:
                             {
                                  key_as_string = to_string(token);
                             }

                             break;

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
                                  if (token != INT_KEY)
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

                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    
                                    result.insert(std::make_pair(key_as_string, rawvalue));
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                std::shared_ptr<search_query> request = std::make_shared<search_query>();
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->subresult = ++tracker;
                                                request->mmap = result;
                                                result.clear();
                                                request->SetOK();
                                                DataFlush::AttachResult(request);
                                      }
                                      
                                      if (aux_counter == (unsigned int)limit)
                                      {
                                                break;               
                                      }
                             }
                }
                else if (limit == -1)
                {
                             aux_counter++;
                             result.insert(std::make_pair(key_as_string, rawvalue));
            
                             if (aux_counter % ITER_LIMIT == 0)
                             {
                                        std::shared_ptr<search_query> request = std::make_shared<search_query>();
                                        request->user = this->user;
                                        request->partial = true;
                                        request->subresult = ++tracker;
                                        request->mmap = result;
                                        result.clear();
                                        request->SetOK();
                                        DataFlush::AttachResult(request);
                             }
                }
                         
                total_counter++;
    }

    this->subresult = ++tracker;
    this->partial = false;
    this->counter = aux_counter;
    this->mmap = result;
    this->SetOK();
}

void search_query::Process()
{
        Dispatcher::MMapFlush(true, "Key", "Value", this);
}

void keys_query::Run()
{
       StringVector result;
       unsigned int total_counter = 0;
       unsigned int aux_counter = 0;
       unsigned int tracker = 0;

       std::string rawmap;
       
       rocksdb::Iterator* it = this->database->GetAddress()->NewIterator(rocksdb::ReadOptions());
       
       for (it->SeekToFirst(); it->Valid(); it->Next()) 
       {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }
                
                rawmap = it->key().ToString();
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
                             case 0:
                             {
                                  key_as_string = to_string(token);
                                
                                  if (!Daemon::Match(key_as_string, this->key))
                                  {
                                       skip = true;
                                  }
                             }
                             
                             break;
                             
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
                                  if (token != INT_KEY)
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
                
                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    
                                    result.push_back(key_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                tracker++;
                                                std::shared_ptr<keys_query> request = std::make_shared<keys_query>();
                                                request->key	= this->key;
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->subresult = tracker;
                                                request->VecData = result;
                                                result.clear();
                                                request->SetOK();
                                                DataFlush::AttachResult(request);
                                      }
                                      
                                      if (aux_counter == (unsigned int)limit)
                                      {
                                                break;               
                                      }
                             }
                }
                else if (limit == -1)
                {
                             aux_counter++;
                             result.push_back(key_as_string);
            
                             if (aux_counter % ITER_LIMIT == 0)
                             {
                                        tracker++;
                                        std::shared_ptr<keys_query> request = std::make_shared<keys_query>();
                                        request->key	= this->key;
                                        request->user = this->user;
                                        request->partial = true;
                                        request->subresult = tracker;
                                        request->VecData = result;
                                        result.clear();
                                        request->SetOK();
                                        DataFlush::AttachResult(request);
                             }
                }
                         
                total_counter++;
    }

    this->subresult = ++tracker;
    this->partial = false;
    this->counter = aux_counter;
    this->VecData = result;
    this->SetOK();
}

void keys_query::Process()
{
        Dispatcher::VectorFlush(false, this->key, this);
}

void append_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value) + this->value;
       
       if (this->Write(this->dest, to_bin(this->response)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void append_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response).c_str());
}

void random_query::Run()
{
       unsigned int total_counter = 0;
       
       std::string foundkey;

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

                std::string key_as_string;
          
                while (stream.items_extract(token))
                {
                        if (skip)
                        {
                            break;
                        }

                        switch (strcounter)
                        {
                             case 0:
                             {
                                  key_as_string = to_string(token);
                             }

                             break;

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
                                  if (token != INT_KEY)
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
                
                if (foundkey.empty())
                {
                     foundkey = key_as_string;
                     continue;
                }
                
                unsigned int rvalue = IntRand(0, 1);
                
                if (rvalue == 0)
                {
                      foundkey = key_as_string;
                }	
                
                unsigned int stopvalue = IntRand(2, 1000);
                
                if (stopvalue == total_counter)
                {
                     break;
                }
      }
      
      if (total_counter == 0 && foundkey.empty())
      {
            access_set(DBL_NOT_FOUND);
            return;
      }
       
      this->response = foundkey;
      this->SetOK();
}

void random_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response.c_str());
}

void getexp_query::Run()
{
       RocksData result = this->Get(this->dest);
       this->response = to_string(result.value);
       
       this->WriteExpire(this->key, this->select_query, this->id);
       this->SetOK();
}

void getexp_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}

void asbool_query::Run()
{
       RocksData result = this->Get(this->dest);

       std::string as_str = to_string(result.value);
       
       if (as_str == "1" || as_str == "true" || as_str == "on")
       {
             this->response = "true";
       }
       else
       {
            this->response = "false";
       }
       
       this->SetOK();
}

void asbool_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void ismatch_query::Run()
{
       RocksData result = this->Get(this->dest);

       std::string as_str = to_string(result.value);

       if (Daemon::Match(as_str, this->value))
       {
             this->response = "1";
       }
       else
       {
            this->response = "0";
       }

       this->SetOK();
}

void ismatch_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void insert_query::Run()
{       
       RocksData result = this->Get(this->dest);
       std::string as_str = to_string(result.value);
       
       if (as_str.length() < this->id)
       {
             access_set(DBL_INVALID_RANGE);
             return;
       }
       
       as_str.insert(this->id, this->value);
       this->response = as_str;
       
       if (this->Write(this->dest, to_bin(as_str)))
       {
            this->SetOK();
       }
       else
       {
           access_set(DBL_UNABLE_WRITE);
       }
}

void insert_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}
