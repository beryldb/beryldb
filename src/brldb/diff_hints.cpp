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
#include "brldb/list_handler.h"
#include "brldb/multimap_handler.h"
#include "brldb/map_handler.h"
#include "brldb/vector_handler.h"
#include "brldb/diff.h"
#include "brldb/dbmanager.h"
#include "helpers.h"

void diff_query::Keys()
{
    std::string lookup  = to_bin(this->value) + ":" + convto_string(convto_string(this->select_query)) + ":" + this->identified;
    
    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);     
    
    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }
    
    RocksData query_result = this->Get(this->dest);
    
    if (query_result.value == dbvalue)
    {
          this->response = "0";
    }
    else
    {
          this->response = "1";
    }

}

void diff_query::Maps()
{
    RocksData query_result = this->Get(this->dest);

    std::shared_ptr<MapHandler> handler1 = MapHandler::Create(query_result.value);

    std::string lookup  = to_bin(this->value) + ":" + convto_string(convto_string(this->select_query)) + ":" + this->identified;

    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);

    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }

    std::shared_ptr<MapHandler> handler2 = MapHandler::Create(dbvalue);

    StringVector flist = DiffHandler::CompareMap(handler2->GetList(), handler1->GetList());

    StringVector result;

    unsigned int total_counter = 0;
    unsigned int aux_counter = 0;
    unsigned int tracker = 0;

    for (StringVector::const_iterator i = flist.begin(); i != flist.end(); i++)
    {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }

                std::string key_as_string = *i;

                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;

                                    result.push_back(key_as_string);

                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                tracker++;
                                                std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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
                                        std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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

void diff_query::Multis()
{
    RocksData query_result = this->Get(this->dest);

    std::shared_ptr<MultiMapHandler> handler1 = MultiMapHandler::Create(query_result.value);
    
    std::string lookup  = to_bin(this->value) + ":" + convto_string(convto_string(this->select_query)) + ":" + this->identified;
    
    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);     
    
    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }
    
    std::shared_ptr<MultiMapHandler> handler2 = MultiMapHandler::Create(dbvalue);
    
    StringVector flist = DiffHandler::CompareMultiMap(handler2->GetList(), handler1->GetList());
    
    StringVector result;

    unsigned int total_counter = 0;
    unsigned int aux_counter = 0;
    unsigned int tracker = 0;

    for (StringVector::const_iterator i = flist.begin(); i != flist.end(); i++)
    {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }
                
                std::string key_as_string = *i;

                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    
                                    result.push_back(key_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                tracker++;
                                                std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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
                                        std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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

void diff_query::Geos()
{
    std::string lookup  = to_bin(this->value) + ":" + convto_string(this->select_query) + ":" + this->identified;
    
    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);     
    
    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }
    
    RocksData query_result = this->Get(this->dest);
    
    if (query_result.value == dbvalue)
    {
          this->response = "1";
    }
    else
    {
          this->response = "0";
    }

}

void diff_query::Vectors()
{
    RocksData query_result = this->Get(this->dest);

    std::shared_ptr<VectorHandler> handler1 = VectorHandler::Create(query_result.value);
    
    std::string lookup  = to_bin(this->value) + ":" + convto_string(this->select_query) + ":" + this->identified;
    
    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);     
    
    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }
    
    std::shared_ptr<VectorHandler> handler2 = VectorHandler::Create(dbvalue);
    
    StringVector flist = DiffHandler::CompareVector(handler2->GetList(), handler1->GetList());
    
    StringVector result;

    unsigned int total_counter = 0;
    unsigned int aux_counter = 0;
    unsigned int tracker = 0;

    for (StringVector::const_iterator i = flist.begin(); i != flist.end(); i++)
    {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }
                
                std::string key_as_string = *i;

                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    
                                    result.push_back(key_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                tracker++;
                                                std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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
                                        std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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

void diff_query::Lists()
{
    RocksData query_result = this->Get(this->dest);
    
    std::shared_ptr<ListHandler> handler1 = ListHandler::Create(query_result.value);
    
    std::string lookup  = to_bin(this->value) + ":" + convto_string(this->select_query) + ":" + this->identified;
    
    std::string dbvalue;
    rocksdb::Status fstatus = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);     
    
    if (!fstatus.ok())
    {
          access_set(DBL_NOT_FOUND);
          return;
    }
    
    std::shared_ptr<ListHandler> handler2 = ListHandler::Create(dbvalue);
    
    ListMap flist = DiffHandler::CompareList(handler2->GetList(), handler1->GetList());
    
    StringVector result;

    unsigned int total_counter = 0;
    unsigned int aux_counter = 0;
    unsigned int tracker = 0;

    for (ListMap::const_iterator i = flist.begin(); i != flist.end(); i++)
    {
                if (!Dispatcher::CheckIterator(this))
                {
                       return;
                }
                
                std::string key_as_string = *i;

                if (this->limit != -1 && ((signed int)total_counter >= this->offset))
                {
                             if (((signed int)aux_counter < limit))
                             {
                                    aux_counter++;
                                    
                                    result.push_back(key_as_string);
             
                                    if (aux_counter % ITER_LIMIT == 0)
                                    {
                                                tracker++;
                                                std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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
                                        std::shared_ptr<diff_query> request = std::make_shared<diff_query>();
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


void diff_query::Run()
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
    else if (this->identified == INT_VECTOR)
    {	 
          this->Vectors();
    }	

}

void diff_query::Process()
{
    if (this->identified == INT_LIST)
    {
          Dispatcher::VectorFlush(false, "List", this);
    }
    else if (this->identified == INT_VECTOR)
    {
          Dispatcher::VectorFlush(false, "Vector", this);
    }
    else if (this->identified == INT_MMAP || this->identified == INT_MAP)
    {
         Dispatcher::MMapFlush(false, "Map", "Key", this);
    }
    else if (this->identified == INT_KEY || this->identified == INT_GEO)
    {
         Dispatcher::JustAPI(this->user, BRLD_START_LIST);
         this->user->SendProtocol(BRLD_ITEM, this->response.c_str());
         Dispatcher::JustAPI(this->user, BRLD_END_LIST);
    }
    
}
