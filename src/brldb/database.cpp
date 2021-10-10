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
#include "exit.h"
#include "engine.h"
#include "brldb/datathread.h"
#include "managers/user.h"
#include "managers/settings.h"

void Database::SetClosing(bool flag)
{
        this->Closing = flag;
}

bool Database::IsClosing()
{
        return this->Closing;
}

Database::Database(const std::string& dbname, const std::string& dbpath) : created(Kernel->Now()), name(dbname), path(Kernel->Config->Paths->SetWDDB(dbpath))
{
        this->SetClosing(false);
}

CoreDatabase::CoreDatabase() : Database(CORE_DB, CORE_DB + ".db")
{

}

UserDatabase::UserDatabase(const std::string& dbname, const std::string& dbpath) : Database(dbname, dbpath)
{

}

void Database::Close()
{
        if (!this->db)
        {
            return;
        }

        slog("DATABASE", LOG_DEFAULT, "Closing database: %s.", this->GetName().c_str());
        //bprint(INFO, "Closing database: %s.", this->GetName().c_str());

        delete this->db;
}

bool Database::Open()
{	
        this->db = NULL;
        
        options.IncreaseParallelism(Kernel->Config->DB.increaseparal);
        options.OptimizeLevelStyleCompaction(2);

        options.env 				= Kernel->Store->GetEnv();
        options.create_if_missing 		= Kernel->Config->DB.createim;
        options.keep_log_file_num 		= 1;
        options.write_thread_max_yield_usec 	= Kernel->Config->DB.yieldusec;
        options.enable_thread_tracking 		= true;
        options.enable_pipelined_write 		= Kernel->Config->DB.pipeline;

        this->status 				= rocksdb::DB::Open(options, this->path, &this->db);

        slog("DATABASE", LOG_VERBOSE, "Database opened: %s", this->path.c_str());

        if (!this->status.ok()) 
        {
                bprint(ERROR, "Error while opening database: %s, %s", this->path.c_str(), this->status.ToString().c_str());
                slog("DATABASE", LOG_DEFAULT, "Error while opening database: %s, %s", this->path.c_str(), this->status.ToString().c_str()); 
                Kernel->Exit(EXIT_CODE_DATABASE, true, true);
        }
        else
        {
                slog("DATABASE", LOG_DEFAULT, "Database opened: %s: %s", this->path.c_str(), this->status.ToString().c_str());
        }

        return true;
}

bool Database::FlushDB()
{
        if (!this->db)
        {
                return false;
        }
        
        this->SetClosing(true);

        //bprint(INFO, "Processing Flushdb: %s.", this->name.c_str());
        slog("DATABASE", LOG_DEFAULT, "Processing Flushdb: %s.", this->name.c_str());
        
        /* We must close our database before flushing it. */
        
        this->Close();
        
        Kernel->Store->Expires->DatabaseDestroy(this->GetName());
        Kernel->Store->Futures->DatabaseDestroy(this->GetName());
        
        rocksdb::Options d_options;
        rocksdb::DestroyDB(this->path, d_options);
        
        this->SetClosing(false);
        
        /* Open the database again. */
        
        return this->Open();
}

CoreManager::CoreManager()
{

}

void CoreManager::Open()
{
       std::shared_ptr<CoreDatabase> New = std::make_shared<CoreDatabase>();
       New->Open();
       
       if (!New->db)
       {
             bprint(ERROR, "Error while creating core database.");
             Kernel->Exit(EXIT_CODE_DATABASE);
       }
       
       this->DB = New;
}

void CoreManager::UserDefaults()
{
       if (!Kernel->Store->IsFirst())
       {
            return;
       }
     
       /* We add our default user. */
  
       UserHelper::Add(ROOT_USER, "default");
       UserHelper::SetFlags(ROOT_USER, "r");
}

void CoreManager::CheckDefaults()
{
       bprint(DONE, "Starting BerylDB.");
       
       const std::string& result = STHelper::Get("instance", "first_ran");
       
       /* Creating instance, unless otherwise specified. */
       
       const time_t instance = Kernel->Now();

       /* No entry set. */
       
       if (result.empty() || result == "")
       {
                 STHelper::Set("instance", "first_ran", convto_string(instance));
                      
                 bprint(DONE, "Welcome to BerylDB.");

                 Kernel->Store->DBM->Create("default", "default");
                 Kernel->Store->DBM->SetDefault("default");
                 Kernel->Sets->SetDefaults();
                 
                 Kernel->Groups->Add("default", DEFAULT_GFLAGS);

                 bprint(DONE, "Default settings created.");
                 
                 /* Default settings. */
                 
                 Kernel->Store->First = true;
       }
       else
       {		
               bprint(DONE, "Instance created: %s", Daemon::HumanEpochTime(convto_num<time_t>(result)).c_str());
               Kernel->Store->First = false;
       }
       
       Kernel->Store->created = instance;
}

StoreManager::StoreManager() : env(rocksdb::Env::Default()), Default(NULL), First(false)
{
       env->SetBackgroundThreads(2, rocksdb::Env::LOW);
       env->SetBackgroundThreads(3, rocksdb::Env::HIGH);
}

StoreManager::~StoreManager() 
{
       env = NULL;
}

void StoreManager::OpenAll()
{
       unsigned int counter = 0;

       for (unsigned int i = 1; i <= Kernel->Config->DB.datathread; i++)
       {
              DataThread* New = new DataThread();   
              New->Create();
              Kernel->Store->Flusher->threadslist.push_back(New);
              counter++;
       }

       if (!counter)
       {
              bprint(ERROR, "Threads must be greater than 0.");
              Kernel->Exit(EXIT_CODE_THREADS);
       }

       iprint(counter, "Thread%s initialized.", counter > 1 ? "s" : "");
       slog("DATABASE", LOG_VERBOSE, "Thread%s initialized.", counter > 1 ? "s" : "");
}

void StoreManager::Push(std::shared_ptr<QueryBase> request)
{
      User* const user = request->user;

      if (user == NULL || user->IsQuitting())
      {
           return;
      }

      user->pending.push_back(request);       
}

