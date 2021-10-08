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
#include "core_dbmanager.h"

CommandSFlush::CommandSFlush(Module* Creator) : Command(Creator, "SFLUSH", 0, 1)
{
         flags  = 'r';
         syntax = "<select>";
}

COMMAND_RESULT CommandSFlush::Handle(User* user, const Params& parameters)
{  
       std::string select;
       
       if (parameters.size())
       {
             select = parameters[0];
       }
       else
       {
             select = user->select;
       }
       
       if (!CheckValidPos(user, select))
       {
              return FAILED;
       }
       
       if (!Daemon::CheckRange(user, select, INVALID_RANGE, 1, 100))
       {
              return FAILED;
       }
       
       KeyHelper::Simple(user, std::make_shared<sflush_query>(), select, "", false);
       return SUCCESS;
}

CommandUsing::CommandUsing(Module* Creator) : Command(Creator, "USING", 1, 1)
{
         flags  = 'm';
         syntax = "<instance>";
}

COMMAND_RESULT CommandUsing::Handle(User* user, const Params& parameters)
{  
       const std::string& instance = parameters[0];

       User* const found = Kernel->Clients->FindInstance(instance);

       if (!found)
       {
              user->SendProtocol(ERR_INPUT, NOT_FOUND);
              return FAILED;
       }

       user->SendProtocol(BRLD_OK, convto_string(found->GetSelect()).c_str());
       return SUCCESS;
}

CommandUse::CommandUse(Module* Creator) : Command(Creator, "USE", 1)
{
       no_hint_until_reg 	= 	true;

       /* Users should be able to use this command before registering. */

       pre_reg_ok 		= 	true;
       syntax 			= 	"<id between 1 and 100>";
}

COMMAND_RESULT CommandUse::Handle(User* user, const Params& parameters)
{  
       const std::string& use = parameters[0];

       if (!CheckValidPos(user, use))
       {
              return FAILED;
       }

       if (!Daemon::CheckRange(user, use, INVALID_RANGE, 1, 100))
       {
               return FAILED;
       }
       
       unsigned int as_int = convto_num<unsigned int>(use);

       if (user->select == as_int)
       {
             user->SendProtocol(ERR_INPUT, PROCESS_ALREADY);
             return FAILED;
       }

       user->select = as_int;

       if (user->agent == DEFAULT_EMERALD)
       {
           user->SendProtocol(BRLD_NEW_USE, use, PROCESS_OK);
       }
       else
       {
            user->SendProtocol(BRLD_OK, PROCESS_OK);       
       }
       
       return SUCCESS;
}

CommandCurrent::CommandCurrent(Module* Creator) : Command(Creator, "CURRENT", 0)
{

}

COMMAND_RESULT CommandCurrent::Handle(User* user, const Params& parameters)
{  
       const std::string& use = user->GetSelect();
       
       if (use.empty())
       {
            user->SendProtocol(ERR_INPUT, PROCESS_NULL);
            return FAILED;
       }
       
       user->SendProtocol(BRLD_OK, use.c_str());
       return SUCCESS;
}

CommandDBSize::CommandDBSize(Module* Creator) : Command(Creator, "DBSIZE", 0, 1)
{
       group   = 'w';
       syntax  = "<database>";
}

COMMAND_RESULT CommandDBSize::Handle(User* user, const Params& parameters)
{  
       std::string db;
       
       if (parameters.size())
       {
            db = parameters[0];
       }
       else
       {
            if (!user->GetDatabase())
            {
                 user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
                 return FAILED;
            }
            
            db = user->GetDatabase()->GetName();
       }
       
       const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(db);

       if (!database)
       {
              user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
              return FAILED;
       }

       DBHelper::DBSize(user, database);
       return SUCCESS;
}

CommandPWD::CommandPWD(Module* Creator) : Command(Creator, "PWD", 0)
{
       group = 'w';
}

COMMAND_RESULT CommandPWD::Handle(User* user, const Params& parameters)
{  
       if (!BASE_PATH.empty())
       {
             user->SendProtocol(BRLD_OK, BASE_PATH.c_str());
             return SUCCESS;
       }

       user->SendProtocol(ERR_INPUT, PROCESS_NULL);
       return FAILED;
}

CommandDefault::CommandDefault(Module* Creator) : Command(Creator, "DEFAULT", 0)
{

}

COMMAND_RESULT CommandDefault::Handle(User* user, const Params& parameters)
{  
       const std::string& dbname = STHelper::Get("dbconf", "dbdefault");
       
       if (dbname.empty())
       {
             user->SendProtocol(ERR_INPUT, PROCESS_NULL);
             return FAILED;
       }
       
       user->SendProtocol(BRLD_OK, dbname.c_str());
       return SUCCESS;
}

CommandDB::CommandDB(Module* Creator) : Command(Creator, "DB", 0)
{

}

COMMAND_RESULT CommandDB::Handle(User* user, const Params& parameters)
{  
       if (user->GetDatabase())
       {
             const std::string& dbname = user->GetDatabase()->GetName();                                  
             user->SendProtocol(BRLD_OK, dbname);
             return SUCCESS;	
       }
       
       user->SendProtocol(ERR_INPUT, PROCESS_NULL);
       return FAILED;
}

CommandChange::CommandChange(Module* Creator) : Command(Creator, "CHANGE", 1, 1)
{
      
}

COMMAND_RESULT CommandChange::Handle(User* user, const Params& parameters)
{  
       const std::string& dbname 		    =   parameters[0];
       const std::shared_ptr<UserDatabase> database =   Kernel->Store->DBM->Find(dbname);

       if (!database)
       {
             user->SendProtocol(ERR_INPUT, DB_NULL);
             return FAILED;
       }

       user->SetDatabase(database);
       STHelper::Set("dbuser", user->login, dbname);
       user->SendProtocol(BRLD_OK, PROCESS_OK);
       return SUCCESS;
}

CommandDBLIST::CommandDBLIST(Module* Creator) : Command(Creator, "DBLIST", 0, 1)
{
      flags = 'r';
}

COMMAND_RESULT CommandDBLIST::Handle(User* user, const Params& parameters)
{
      const DataMap& dbases = Kernel->Store->DBM->GetDatabases();

      Dispatcher::JustAPI(user, BRLD_START_LIST);

      Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
      Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", "Name", "Path"));
      Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
       
      for (DataMap::const_iterator it = dbases.begin(); it != dbases.end(); ++it)
      {
            std::shared_ptr<UserDatabase> udb = it->second;
            
            const std::string& dbname = udb->GetName();
            const std::string& dbpath = udb->GetPath();
            
            if (parameters.size())
            {
                   if (!Daemon::Match(dbname, parameters[0]) && !Daemon::Match(dbpath, parameters[0])) 
                   {
                        continue;
                   }
            }
            
            Dispatcher::ListDepend(user, BRLD_ITEM_LIST, Daemon::Format("| %-16s| %-29s", dbname.c_str(), dbpath.c_str()), Daemon::Format("%s %s", dbname.c_str(), dbpath.c_str()));
      }     
      
      Dispatcher::JustAPI(user, BRLD_END_LIST);
      Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
      
      return SUCCESS;
}

CommandDBCreate::CommandDBCreate(Module* Creator) : Command(Creator, "DBCREATE", 1, 2)
{
      flags  = 'r';
      syntax = "<name> <path>";
}

COMMAND_RESULT CommandDBCreate::Handle(User* user, const Params& parameters)
{
      const std::string& dbname    =    to_lower(parameters[0]);
      std::string dbpath;
      
      if (parameters.size() > 1)
      {
            dbpath    =    to_lower(parameters[1]);
      }
      else
      {
            dbpath    =    to_lower(dbname);
      }

      /* 'dbdefault' is a reserved database name. */
      
      if (dbname == "dbdefault" || dbname == CORE_DB || dbname == ROOT_USER)
      {
             user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
             return FAILED;
      }
      
      if (Kernel->Store->DBM->CountDatabases() > MAX_DATABASES)
      {
             user->SendProtocol(ERR_INPUT, USER_MIMMAX_LENGTH);         
             return FAILED;
      }

      if (!Kernel->Engine->IsDatabase(name))
      {
             user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
             return FAILED;
      }
      
      if (Kernel->Store->DBM->Create(dbname, dbpath))
      {			
             Kernel->Store->DBM->Load(dbname);
             user->SendProtocol(BRLD_OK, PROCESS_OK);
             sfalert(user, NOTIFY_DEFAULT, "Added database: %s", dbname.c_str());      
             
             if (user->GetDatabase() == NULL)
             {
                  const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(dbname);
                  
                  if (database)
                  {
                        user->SetDatabase(database);
                        STHelper::Set("dbuser", user->login, dbname);
                  }
             }
             
             return SUCCESS;
      }
      
      user->SendProtocol(ERR_INPUT, PROCESS_ALREADY);
      return FAILED;
}

CommandDBDelete::CommandDBDelete(Module* Creator) : Command(Creator, "DBDELETE", 1, 1)
{
      flags = 'r';
      syntax = "<name>";
}

COMMAND_RESULT CommandDBDelete::Handle(User* user, const Params& parameters)
{
      const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(parameters[0]);

      if (!database)
      {
             user->SendProtocol(ERR_INPUT, PROCESS_NULL);
             return FAILED;
      }
      
      /* We cannot remove a database whenever is locked. */
      
      if (database->IsClosing())
      {
             user->SendProtocol(ERR_INPUT, DATABASE_BUSY);
             return FAILED;
      }
      
      DBHelper::DatabaseReset(user, database->GetName());
      return SUCCESS;
}

CommandDBTest::CommandDBTest(Module* Creator) : Command(Creator, "TEST", 0, 0)
{

}

COMMAND_RESULT CommandDBTest::Handle(User* user, const Params& parameters)
{
      user->SendProtocol(BRLD_OK, PROCESS_OK);
      TestHelper::Dump(user);
      return SUCCESS;
}

CommandDBSetDefault::CommandDBSetDefault(Module* Creator) : Command(Creator, "SETDEFAULT", 1, 1)
{
      flags = 'r';
      syntax = "<db name>";
}

COMMAND_RESULT CommandDBSetDefault::Handle(User* user, const Params& parameters)
{
      const std::shared_ptr<UserDatabase>& database = Kernel->Store->DBM->Find(parameters[0]);

      if (!database)
      {
             user->SendProtocol(ERR_INPUT, PROCESS_NULL);
             return FAILED;
      }

      sfalert(user, NOTIFY_DEFAULT, "Setting default database to: %s", database->GetName().c_str());

      DBManager::SetDefault(database->GetName().c_str());
      user->SendProtocol(BRLD_OK, PROCESS_OK);
      return SUCCESS;
}

CommandFlushAll::CommandFlushAll(Module* Creator) : Command(Creator, "FLUSHALL", 0, 0)
{
      flags = 'r';
}

COMMAND_RESULT CommandFlushAll::Handle(User* user, const Params& parameters)
{  
      const DataMap& dbs = Kernel->Store->DBM->GetDatabases();

      Kernel->Ready = false;
     
      for (DataMap::const_iterator i = dbs.begin(); i != dbs.end(); ++i)
      {
               std::shared_ptr<UserDatabase> db = i->second;
               db->FlushDB();
      }
     
      Kernel->Ready = true;
                
      user->SendProtocol(BRLD_OK, PROCESS_OK);                
      return SUCCESS;
}

CommandFlushDB::CommandFlushDB(Module* Creator) : Command(Creator, "FLUSHDB", 0, 1)
{
       flags = 'r';
}

COMMAND_RESULT CommandFlushDB::Handle(User* user, const Params& parameters)
{  
       std::shared_ptr<UserDatabase> database;
       
       if (parameters.size())
       {
              database = Kernel->Store->DBM->Find(parameters[0]);
       }
       else
       {
              database = user->GetDatabase();
       }
       
       if (!database)
       {
              user->SendProtocol(ERR_INPUT, PROCESS_NULL);
              return FAILED;
       }
       
       if (DBHelper::FlushDB(database, true))
       {
              sfalert(user, NOTIFY_DEFAULT, "Flushed database: %s", user->GetDatabase()->GetName().c_str());      
              user->SendProtocol(BRLD_OK, PROCESS_OK);
              return SUCCESS;
       }

       sfalert(user, NOTIFY_DEFAULT, "Unable to flush database: %s", user->GetDatabase()->GetName().c_str());      
       user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
       return FAILED;
}

