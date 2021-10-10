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
#include "brldb/query.h"
#include "brldb/dbmanager.h"

bool QueryBase::Swap(const std::string& newdest, const std::string& ldest, const std::string& lvalue, std::shared_ptr<Database> db)
{
       if (db == NULL)
       {
           db = this->database;
       }       
       
       rocksdb::WriteBatch batch;

       batch.Put(newdest, lvalue);
       batch.Delete(ldest);
       rocksdb::Status status = db->GetAddress()->Write(rocksdb::WriteOptions(), &batch);
       
       if (status.ok())
       {
            return true;
       }
       
       return false;
}

bool QueryBase::SwapWithExpire(const std::string& newdest, const std::string& ldest, const std::string& lvalue, unsigned int select, const std::string& lkey, unsigned int ttl, const std::string& oldkey)
{
       rocksdb::WriteBatch batch;

       batch.Put(newdest, lvalue);
       batch.Delete(ldest);
       
       const std::string& lookup = to_bin(lkey) + ":" + convto_string(select) + ":" + INT_EXPIRE + ":" + this->database->GetName();

       batch.Put(lookup, convto_string(ttl));

       rocksdb::Status stats = this->database->GetAddress()->Write(rocksdb::WriteOptions(), &batch);

       if (stats.ok())
       {
             Kernel->Store->Expires->Delete(this->database, oldkey, select);
             Kernel->Store->Expires->Add(this->database, ttl, lkey, select, true);
             return true;
       }
       
       return false;
}


void QueryBase::ExpireBatch(const std::string& wdest, const std::string& lvalue, const std::string& e_key, unsigned int select, unsigned int ttl)
{
       rocksdb::WriteBatch batch;
       std::string lookup = to_bin(e_key) + ":" + convto_string(select) + ":" + INT_EXPIRE + ":" + this->database->GetName();
       
       batch.Put(lookup, convto_string(ttl));
       batch.Put(wdest, to_bin(lvalue));
       
       rocksdb::Status stats = this->database->GetAddress()->Write(rocksdb::WriteOptions(), &batch);
       
       if (stats.ok())
       {
            Kernel->Store->Expires->Add(this->database, ttl, e_key, select, true);
       }
}

bool QueryBase::Write(const std::string& wdest, const std::string& lvalue)
{
       rocksdb::Status status = this->database->GetAddress()->Put(rocksdb::WriteOptions(), wdest, lvalue);
       
       if (status.ok())
       {
             return true;
       }
       
       return false;
}

void QueryBase::Delete(const std::string& wdest)
{
       this->database->GetAddress()->Delete(rocksdb::WriteOptions(), wdest);
}

void QueryBase::WriteExpire(const std::string& e_key, unsigned int select, unsigned int ttl, std::shared_ptr<Database> db)
{
       if (db == NULL)
       {
            db = this->database;
       }
       
       const std::string& lookup = to_bin(e_key) + ":" + convto_string(select) + ":" + INT_EXPIRE + ":" + db->GetName();
       
       if (this->Write(lookup, convto_string(ttl)))
       {
           Kernel->Store->Expires->Add(db, ttl, e_key, select, true);
       }
}

void QueryBase::WriteFuture(const std::string& e_key, unsigned int select, unsigned int ttl, const std::string& fvalue)
{
       std::string lookup = to_bin(e_key) + ":" + convto_string(select) + ":" + INT_FUTURE + ":" + this->database->GetName();
       std::string lvalue = convto_string(ttl) + ":" + fvalue;
       
       if (this->Write(lookup, lvalue))
       {
            Kernel->Store->Futures->Add(this->database, ttl, e_key, fvalue, select, true);
       }
}

void QueryBase::DelFuture()
{
        Kernel->Store->Futures->Delete(this->database, this->key, this->select_query);
        std::string lookup = to_bin(this->key) + ":" + convto_string(this->select_query) + ":" + INT_FUTURE + ":" + this->database->GetName();
        this->Delete(lookup);
}

signed int QueryBase::IsExpiring()
{
      const signed int ttl = ExpireManager::GetTIME(this->database, this->key, this->select_query);
      
      if (ttl > 0)
      {
             return ttl;
      }
      
      return -1;
}

void QueryBase::DelExpire()
{
        /* Deletes key in case it is expiring. */
        
        Kernel->Store->Expires->Delete(this->database, this->key, this->select_query);
        std::string lookup = to_bin(this->key) + ":" + convto_string(this->select_query) + ":" + INT_EXPIRE + ":" + this->database->GetName();
        this->Delete(lookup);
}

RocksData QueryBase::Get(const std::string& where)
{
       if (this->mapped.loaded)
       {
            return this->mapped;   
       }
       
       RocksData result;
       std::string dbvalue;
       
       result.status = this->database->GetAddress()->Get(rocksdb::ReadOptions(), where, &dbvalue);
       result.value = dbvalue;
       return result;
}

int QueryBase::CheckDest(unsigned int select, const std::string& regkey, const std::string& ltype, std::shared_ptr<Database> db)
{
       if (db == NULL)
       {
            db = this->database;
       }

       if (db == this->database && select == this->select_query && this->key == regkey && ltype == this->identified)
       {
              return 1;
       }
       
       for (std::vector<std::string>::const_iterator iter = TypeRegs.begin(); iter != TypeRegs.end(); ++iter)
       {
              std::string found_type = *iter;
              std::string saved = to_bin(regkey) + ":" + convto_string(select) + ":" + found_type;
       
              std::string dbvalue;
              rocksdb::Status fstatus2 = db->GetAddress()->Get(rocksdb::ReadOptions(), saved, &dbvalue);
       
              if (!dbvalue.empty())
              {
                   if (found_type == ltype)
                   {
                        /* Ltype found, same type. */
                        
                        return 1;
                   }
                   else 
                   {
                        /* Different ltype */
                        
                        return 2;
                   }
              }
       }
       
       /* 0 means entry is not defined. */
       
       return 0;
}

bool QueryBase::GetRegistry(unsigned int select, const std::string& regkey, bool do_load)
{
       for (std::vector<std::string>::const_iterator iter = TypeRegs.begin(); iter != TypeRegs.end(); ++iter)
       {
             std::string found_type = *iter;
             std::string lookup = to_bin(regkey) + ":" + convto_string(select) + ":" + found_type;
             
             std::string dbvalue;
             rocksdb::Status fstatus2 = this->database->GetAddress()->Get(rocksdb::ReadOptions(), lookup, &dbvalue);

             if (fstatus2.ok())
             {
                    this->identified = found_type;
                    this->SetDest(regkey, select, found_type);
                   
                    if (do_load)
                    {
                          mapped.value = dbvalue;
                          mapped.status = fstatus2;
                          mapped.loaded = true;
                    }
                   
                    return true;
             }
       }

       this->identified = PROCESS_NULL;
       return false;
}

void QueryBase::SetDest(const std::string& regkey, unsigned int regselect, const std::string& regtype)
{
       this->dest = to_bin(regkey) + ":" + convto_string(regselect) + ":" + regtype;
}

bool QueryBase::Check()
{
       if (this->database && this->database->IsClosing())
       {
              this->access_set(DBL_STATUS_BROKEN);
       }
       else
       {
              this->access_set(DBL_STATUS_OK);
              return true;
       }
       
       return false;
}

bool QueryBase::CheckKey()
{
      if (this->key_required)
      {
             if (this->key.empty())
             {
                   this->access_set(DBL_MISS_ARGS);
                   return false;
             }
             else
             {
                   return true;
             }
      }
      
      return true;
}

bool QueryBase::Prepare()
{
 
     if (!this->Check())
     {
           return false;
     }
     
     if (!this->CheckKey())
     {
            return false;
     }
     
     switch (this->type)
     {
           case QUERY_TYPE_WRITE:
           {
                 if (this->GetRegistry(this->select_query, this->key, true))
                 {
                      if (this->identified == this->base_request)
                      {
                             this->Run();
                             return true;
                      }   
                      else
                      {  
                             this->access_set(DBL_INVALID_TYPE);
                             this->response = this->identified;
                             return false;
                      }
                 }
                 else
                 {
                      if (this->identified == PROCESS_NULL)
                      {
                            this->dest = to_bin(this->key) + ":" + convto_string(this->select_query) + ":" + this->base_request;
                            this->Run();
                            return true;
                      }  
                 }
           }
           
           break;
           
           case QUERY_TYPE_LAT:
           case QUERY_TYPE_LONG:
           case QUERY_TYPE_READ:
           {
                 if (this->GetRegistry(this->select_query, this->key, true))
                 {
                     if (this->identified == this->base_request)
                     {
                            this->Run();
                            return true;
                     }   
                     else
                     {  
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                            return false;
                      }
                     
                 }
                 else
                 {
                 
                     if (this->identified == PROCESS_NULL)
                     {
                            this->access_set(DBL_NOT_FOUND);
                            return false;
                     }  
                 }
            
           }
           
           break;

           case QUERY_TYPE_READ_ALLOW:
           {
                 if (this->GetRegistry(this->select_query, this->key, true))
                 {
                     if (this->identified == this->base_request)
                     {
                            this->Run();
                            break;
                     }   
                     else
                     {  
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                      }
                     
                 }
                 else
                 {
                     if (this->identified == PROCESS_NULL)
                     {
                            this->Run();
                            break;
                     }  
                 }
            
           }
           
           break;
           
           case QUERY_TYPE_DIFF:
           {
                   GetRegistry(this->select_query, this->key, true);
                   
                   if (this->identified == PROCESS_NULL)
                   {
                         this->access_set(DBL_NOT_FOUND);
                         this->response = this->identified;
                         return false;
                   }
                   
                   int result = this->CheckDest(this->select_query, this->value, this->identified); 
                  
                   if (result == 1)
                   {
                         if (this->identified == PROCESS_NULL)
                         {
                              this->access_set(DBL_NOT_FOUND);
                              this->response = this->identified;
                              return false;
                         }
                         
                         this->Run();
                         return true;
                   }
                   else if (result == 2)
                   {
                         this->access_set(DBL_INVALID_TYPE);
                         this->response = this->identified;
                   }
                   
                   else
                   {
                         this->access_set(DBL_NOT_FOUND);
                         this->response = this->identified;
                   }
           }
           
           break;
           
           case QUERY_TYPE_RENAME:
           {
                   GetRegistry(this->select_query, this->key, true);
                   
                   if (this->identified == PROCESS_NULL)
                   {
                                this->access_set(DBL_NOT_FOUND);
                                return false;
                   }
                   
                   int result = this->CheckDest(this->select_query, this->value, this->identified); 
                   
                   if (result == 0 || result == 1)
                   {
                         this->Run();
                         return true;
                   }
                   else if (result == 2)
                   {
                         this->access_set(DBL_INVALID_TYPE);
                         this->response = this->identified;
                   }
                   else
                   {
                         if (this->identified == PROCESS_NULL)
                         {
                              this->access_set(DBL_NOT_FOUND);
                              this->response = this->identified;
                         }
                   }
           }
           
           break;
           
           case QUERY_TYPE_TRANSFER:
           {
                   GetRegistry(this->select_query, this->key, true);
 
                   if (this->identified == PROCESS_NULL)
                   {
                         this->access_set(DBL_NOT_FOUND);
                         return false;
                   }
                   
                   if (!this->transf_db)
                   {
                          this->access_set(DBL_UNABLE_WRITE);
                          return false;
                   }

                   int result = this->CheckDest(this->select_query, this->key, this->identified, this->transf_db); 
                   
                   if (result == 0 || result == 1)
                   {
                         this->Run();
                         return true;
                   }
                   else if (result == 2)
                   {
                         this->access_set(DBL_INVALID_TYPE);
                         this->response = this->identified;
                         return false;
                   }
                   
                   return false;

           }

           break;
           
           case QUERY_TYPE_RENAMENX:
           {
                   GetRegistry(this->select_query, this->key, true);
                   
                   if (this->identified == PROCESS_NULL)
                   {
                                this->access_set(DBL_NOT_FOUND);
                                return false;
                   }
                   
                   int result = this->CheckDest(this->select_query, this->value, this->identified); 
                   
                   if (result == 0)
                   {
                         this->Run();
                         return true;
                   }
                   else 
                   {
                         this->access_set(DBL_ENTRY_EXISTS);
                   }
           }
           
           break;
           
           case QUERY_TYPE_CLONE:
           {
                   GetRegistry(this->select_query, this->key, true);
                   int result = this->CheckDest(convto_num<unsigned int>(this->value), this->key, this->identified); 
                   
                   if (this->identified == PROCESS_NULL)
                   {
                                this->access_set(DBL_NOT_FOUND);
                                this->response = this->identified;
                                return false;
                   }
                   
                   if (result == 0 || result == 1)
                   {
                             this->Run();
                             return true;
                   }
                   else if (result == 2)
                   {
                             this->access_set(DBL_INVALID_TYPE);
                             this->response = this->identified;
                   }
           
           }
           
           break;

           case QUERY_TYPE_MOVE:
           {
                   GetRegistry(this->select_query, this->key, true);
                   
                   if (this->identified == PROCESS_NULL)
                   {
                                this->access_set(DBL_NOT_FOUND);
                                return false;
                   }
                   
                   int result = this->CheckDest(convto_num<unsigned int>(this->value), this->key, this->identified); 

                   if (result == 0 || result == 1)
                   {
                          this->Run();
                          return true;
                   }
                   else if (result == 2)
                   {
                          this->access_set(DBL_INVALID_TYPE);
                          this->response = this->identified;
                          return false;
                   }
                   else
                   {
                         if (this->identified == PROCESS_NULL)
                         {
                                this->access_set(DBL_NOT_FOUND);
                                this->response = this->identified;
                                return false;
                         }
                   }
           
           }

           break;
           
           case QUERY_TYPE_COPY:
           {
                   GetRegistry(this->select_query, this->key, true);
                   int result = this->CheckDest(this->select_query, this->value, this->identified); 
                   
                   if (result == 0 || result == 1)
                   {
                        this->Run();
                        return true;
                   }
                   else if (result == 2)
                   {
                             this->access_set(DBL_INVALID_TYPE);
                             this->response = this->identified;
                             return false;
                   }
                   else
                   {
                         if (this->identified == PROCESS_NULL)
                         {
                               this->access_set(DBL_NOT_FOUND);
                               this->response = this->identified;
                               return false;
                         }
                   }

           }
           
           break;
           
           case QUERY_TYPE_EXISTS:
           case QUERY_TYPE_TYPE:
           {
                 this->GetRegistry(this->select_query, this->key, false);
                 this->Run();
                 return true;
           }
           
           break;
           
           case QUERY_TYPE_SKIP:
           {
                 this->Run();
                 return true;
           }
           
           break;
           
           case QUERY_TYPE_FUTURE:
           {
           
                 if (this->GetRegistry(this->select_query, this->key, false))
                 {
                     if (this->identified == this->base_request)
                     {
                           this->Run();
                           return true;
                     }   
                     else
                     {  
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                            return false;
                      }
                }
                else
                {
                         this->Run();
                         return true;
                }
                
           }
           
           break;
           
           case QUERY_TYPE_FUTURE_RUN:
           {
                 if (this->GetRegistry(this->select_query, this->key, false))
                 {
                     if (this->identified == this->base_request)
                     {
                           this->Run();
                           return true;
                     }   
                     else
                     {  
                            this->DelFuture();
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                            return false;
                      }
                }
                else
                {
                         this->Run();
                         return false;
                }
                 
           
           }
           
           break;
           
           case QUERY_TYPE_EXPIRE:
           {
                 if (this->GetRegistry(this->select_query, this->key, false))
                 {
                     if (this->identified == this->base_request)
                     {
                           this->Run();
                           return true;
                     }   
                     else
                     {  
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                            return false;
                     }
                 }
                 else
                 {
                        this->access_set(DBL_NOT_FOUND);
                        return false;
                 }
           }
           
           break;
           
           case QUERY_TYPE_ITER:
           {
                 if (this->GetRegistry(this->select_query, this->key, true))
                 {
                     if (this->identified == this->base_request)
                     {
                           this->Run();
                           return true;
                     }   
                     else
                     {  
                            this->access_set(DBL_INVALID_TYPE);
                            this->response = this->identified;
                            return false;
                      }
                     
                }
           }
           
           break;
             
           case QUERY_TYPE_DELETE:
           {
                 if (this->GetRegistry(this->select_query, this->key, false))
                 {
                           this->Run();
                           return true;
                 }
                 else
                 {
                         if (this->identified == PROCESS_NULL)
                         {
                              this->access_set(DBL_NOT_FOUND);
                              this->response = this->identified;
                              return false;
                         }
                 }
           }

           break;
           
     }

     return false;
}
