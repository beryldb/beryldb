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

#include "brldb/map_handler.h"

void hfind_query::Run()
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
                                  if (token != INT_MAP)
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
                                                std::shared_ptr<hfind_query> request = std::make_shared<hfind_query>();
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->key    = this->key;
                                                
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
                                        std::shared_ptr<hfind_query> request = std::make_shared<hfind_query>();
                                        request->user = this->user;
                                        request->partial = true;
                                        request->key    = this->key;
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

void hfind_query::Process()
{
        Dispatcher::VectorFlush(false, this->key, this);
}

void hset_query::Run()
{
       if (this->value.empty() || this->hesh.empty())
       {
               this->access_set(DBL_MISS_ARGS);
               return;
       }
       
       RocksData resultq = this->Get(this->dest);
       
       std::shared_ptr<MapHandler> handler;
       
       if (!resultq.status.ok())
       {
               handler = std::make_shared<MapHandler>();
               handler->Add(this->hesh, this->value);
               
               if (handler->GetLast() == HANDLER_MSG_OK)
               {
                    if (this->Write(this->dest, handler->as_string()))
                    {
                        this->SetOK();
                    }
               }
               else
               {
                        access_set(DBL_UNABLE_WRITE);
               }
               
               return;
       }
       
       handler = MapHandler::Create(resultq.value);
       handler->Add(this->hesh, this->value);
       
       if (handler->GetLast() == HANDLER_MSG_OK)
       {
            if (this->Write(this->dest, handler->as_string()))
            {
                   this->SetOK();
                   return;
            }
        }
      
        access_set(DBL_UNABLE_WRITE);
}

void hset_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void hsetnx_query::Run()
{
       RocksData resultq = this->Get(this->dest);

       std::shared_ptr<MapHandler> handler = MapHandler::Create(resultq.value);

       if (resultq.status.ok())
       {
               if (handler->Exists(this->hesh))
               {
                    access_set(DBL_ENTRY_EXISTS);
                    return;
               }
       }
       
       handler->Add(this->hesh, this->value);

       if (handler->GetLast() == HANDLER_MSG_OK)
       {  
                 if (this->Write(this->dest, handler->as_string()))
                 {
                      this->SetOK();
                 }
       }
       else
       {
            access_set(DBL_UNABLE_WRITE);
       }
}

void hsetnx_query::Process()
{
       user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void hexists_query::Run()
{
       if (this->hesh.empty())
       {
                this->access_set(DBL_MISS_ARGS);
                return;
       }

       RocksData resultq = this->Get(this->dest);
       
       if (resultq.status.ok())
       {
              std::shared_ptr<MapHandler> handler = MapHandler::Create(resultq.value);

               if (handler->Exists(this->hesh))
               {
                    this->response = "1";
                    this->SetOK();
                    return;
               }
       }
       
       this->response = "0"; 
       this->SetOK();
}

void hexists_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void hstrlen_query::Run()
{
       if (this->hesh.empty())
       {
                this->access_set(DBL_MISS_ARGS);
                return;
       }

       RocksData result = this->Get(this->dest);

       if (!result.status.ok())
       {
               access_set(DBL_NOT_FOUND);
               return;
       }

       std::shared_ptr<MapHandler> handler = MapHandler::Create(result.value);
       this->response = convto_string(handler->Strlen(this->hesh));
       
       if (handler->GetLast() == HANDLER_MSG_OK)
       {
               this->SetOK();
       }
       else
       {
               access_set(DBL_NOT_FOUND);
       }
}

void hstrlen_query::Process()
{
       user->SendProtocol(BRLD_OK, this->response);
}

void hget_query::Run()
{
       if (this->hesh.empty())
       {
                this->access_set(DBL_MISS_ARGS);
                return;
       }

       RocksData result = this->Get(this->dest);
       
       if (!result.status.ok())
       {
               access_set(DBL_NOT_FOUND);
               return;
       }

       std::shared_ptr<MapHandler> handler = MapHandler::Create(result.value);
       this->response = handler->Get(this->hesh);

       if (handler->GetLast() == HANDLER_MSG_OK)
       {
               this->SetOK();
       }
       else
       {
               access_set(DBL_NOT_FOUND);
       }
}

void hget_query::Process()
{
       user->SendProtocol(BRLD_OK, Helpers::Format(this->response));
}

void hdel_query::Run()
{
       RocksData result = this->Get(this->dest);

       std::shared_ptr<MapHandler> handler = MapHandler::Create(result.value);

       if (handler->Get(this->hesh).empty())
       {
               if (handler->GetLast() == HANDLER_MSG_NOT_FOUND)
               {
                          access_set(DBL_NOT_FOUND);
                          return;
                }
       }
       else
       {
               handler->Remove(this->hesh);
       }

       if (handler->Count() > 0)
       {
               if (this->Write(this->dest, handler->as_string()))
               {
                     this->SetOK();
               }
               else
               {
                     access_set(DBL_UNABLE_WRITE);
               }
               
               return;
       }
       else
       {
               this->Delete(this->dest);
       } 
       
       this->SetOK();
}

void hdel_query::Process()
{
        user->SendProtocol(BRLD_OK, PROCESS_OK);
}

void hlist_query::Run()
{
       unsigned int total_counter = 0;
       unsigned int aux_counter = 0;
       unsigned int tracker = 0;
       
       RocksData query_result = this->Get(this->dest);
       
       std::shared_ptr<MapHandler> handler = MapHandler::Create(query_result.value);
       
       DualMap result = handler->GetList();

       StringVector result_return;
       
       for (DualMap::iterator i = result.begin(); i != result.end(); ++i)
       {
                std::string hesh_as_string = i->first;
                
                if (this->flags == QUERY_FLAGS_CORE)
                {
                       result_return.push_back(hesh_as_string);
                       total_counter++;
                       continue;
                }
                
                if (this->flags == QUERY_FLAGS_COUNT)
                {
                      total_counter++;
                      continue;
                }
                
                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    result_return.push_back(hesh_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                std::shared_ptr<hlist_query> request = std::make_shared<hlist_query>();
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->key    = this->key;
                                                
                                                request->subresult = ++tracker;
                                                request->VecData = result_return;
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
                             result_return.push_back(hesh_as_string);
            
                             if (aux_counter % ITER_LIMIT == 0)
                             {
                                        std::shared_ptr<hlist_query> request = std::make_shared<hlist_query>();
                                        request->user = this->user;
                                        request->partial = true;
                                        request->key    = this->key;
                                        
                                        request->subresult = ++tracker;
                                        request->VecData = result_return;
                                        result.clear();
                                        request->SetOK();
                                        DataFlush::AttachResult(request);
                             }
                }
                         
                total_counter++;
    }

     this->subresult = ++tracker;
     this->partial = false;
     this->counter = total_counter;
     this->VecData = result_return;
     this->SetOK();
}

void hlist_query::Process()
{
        if (this->flags == QUERY_FLAGS_COUNT)
        {
               user->SendProtocol(BRLD_OK, convto_string(counter));
               return;
        }
        
        Dispatcher::VectorFlush(false, "Map", this);
}

void hvals_query::Run()
{
       unsigned int total_counter = 0;
       unsigned int aux_counter = 0;
       unsigned int tracker = 0;
       
       RocksData query_result = this->Get(this->dest);
       
       std::shared_ptr<MapHandler> handler = MapHandler::Create(query_result.value);
       
       StringVector result = handler->GetValues();

       StringVector result_return;
       
       for (StringVector::iterator i = result.begin(); i != result.end(); ++i)
       {
                std::string hesh_as_string = (*i);
                
                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    result_return.push_back(hesh_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                std::shared_ptr<hvals_query> request = std::make_shared<hvals_query>();
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->subresult = ++tracker;
                                                request->key    = this->key;
                                                
                                                request->VecData = result_return;
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
                             result_return.push_back(hesh_as_string);
            
                             if (aux_counter % ITER_LIMIT == 0)
                             {
                                        std::shared_ptr<hvals_query> request = std::make_shared<hvals_query>();
                                        request->user = this->user;
                                        request->partial = true;
                                        request->key    = this->key;
                                        
                                        request->subresult = ++tracker;
                                        request->VecData = result_return;
                                        result.clear();
                                        request->SetOK();
                                        DataFlush::AttachResult(request);
                             }
                }
                         
                total_counter++;
    }

     this->subresult = ++tracker;
     this->partial = false;
     this->counter = total_counter;
     this->VecData = result_return;
     this->SetOK();
}

void hvals_query::Process()
{
       Dispatcher::VectorFlush(false, this->key, this);
}

void hgetall_query::Run()
{
       unsigned int total_counter = 0;
       unsigned int aux_counter = 0;
       unsigned int tracker = 0;
       
       RocksData query_result = this->Get(this->dest);
       
       std::shared_ptr<MapHandler> handler = MapHandler::Create(query_result.value);
       
       DualMap result = handler->GetAll();

       std::multimap<std::string, std::string> result_return;
       
       for (DualMap::iterator i = result.begin(); i != result.end(); ++i)
       {
                std::string vkey = i->first;
                std::string vvalue = i->second;
                
                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    result_return.insert(std::make_pair(vkey, vvalue));
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                std::shared_ptr<hgetall_query> request = std::make_shared<hgetall_query>();
                                                request->user = this->user;
                                                request->partial = true;                                  
                                                request->subresult = ++tracker;
                                                request->mmap = result_return;
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
                             result_return.insert(std::make_pair(vkey, vvalue));
            
                             if (aux_counter % ITER_LIMIT == 0)
                             {
                                        std::shared_ptr<hgetall_query> request = std::make_shared<hgetall_query>();
                                        request->user = this->user;
                                        request->partial = true;
                                        request->subresult = ++tracker;
                                        request->mmap = result_return;
                                        result.clear();
                                        request->SetOK();
                                        DataFlush::AttachResult(request);
                             }
                }
                         
                total_counter++;
    }

     this->subresult = ++tracker;
     this->partial = false;
     this->counter = total_counter;
     this->mmap = result_return;
     this->SetOK();
}

void hgetall_query::Process()
{
     Dispatcher::MMapFlush(true, "Map", "Value", this);
}


