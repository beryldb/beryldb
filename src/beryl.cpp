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

std::unique_ptr<Beryl> Kernel = NULL;

/* Beryl's main. */
 
int main(int argc, char** argv)
{
	Kernel = std::make_unique<Beryl>(argc, argv);
	return 1;
}

Beryl::Beryl(int argc, char** argv) : ConfigFile(DEFAULT_CONFIG), Ready(false)
{
	/* Main link. */
	
	Kernel = std::unique_ptr<Beryl>(this);

	/* Refreshing this->TIME for the very first time. */
	
	this->Refresh();
	
	/* 
	 * Startup time. 
	 *
	 * time_t startup is used in order to calculate uptime.
	 * You may see your instance's uptime in cli
	 * by calling the 'status u' command.
 	 */
	
	this->startup = this->Now();

	/* Improve core limits, thus leading to a better debugging experience. */
	
	this->Engine->SetCoreLimits();
	
	/* Generates a random base from TIME */
	
	this->RandomSeed();

	/* Creates sockets fds */
	
	SocketPool::Start();

        this->Sets 		= 	std::make_unique<Settings>();

	/* Channel initializer */ 
	
        this->Channels 		= 	std::make_unique<ChannelManager>();

	/* Notification handler */
	
        this->Notify 		= 	std::make_unique<Notifier>();

	/* Monitor initializer */
	
        this->Monitor 		= 	std::make_unique<MonitorHandler>();
        
        /* Login cache initializer */
        
        this->Logins 		= 	std::make_unique<LoginCache>();
        
        /* Core database initializer */
        
        this->Core 		= 	std::make_unique<CoreManager>();

        /* Store manager initializer */ 
        
        this->Store 		= 	std::make_unique<StoreManager>();

        /* Daemon initializer */ 
        
        this->Engine 		= 	std::make_unique<Daemon>();

        /* Group manager initializer */
        
        this->Groups 		= 	std::make_unique<GroupManager>();

	/* Configuration class. This class will read and our config file. */
	
	this->Config 		= 	std::make_unique<Configuration>();
	
	/* Stats initializer. */
	
	this->Stats 		=	std::make_unique<Serverstats>();
	
	/* Keep a copy of arguments from main() */
	
	this->Config->usercmd.argv = argv;
	
	this->Config->usercmd.argc = argc;

	/* Goes over user cmdline arguments by parsing with getopt. */
	
	this->CommandLine();

	{
		/* Service Providers for basic functions, such as join and publish. */
		
		ServiceProvider* provs[] =
		{
			&brldevents.numeric, &brldevents.join, &brldevents.part, &brldevents.quit, &brldevents.login,
			&brldevents.publish, &brldevents.ping, &brldevents.pong,
			&brldevents.error
		};

		this->Modules->AttachServices(provs, sizeof(provs)/sizeof(provs[0]));
	}

	/* Let's verify whether provided configuration file exists. */
	
	if (!Daemon::ConfigFileExists(this->ConfigFile))
	{
		bprint(ERROR, "Unable to open config file: %s", Daemon::Welcome(this->ConfigFile).c_str());
		this->Exit(EXIT_CODE_CONFIG);
	}

        this->Initialize();
}

void Beryl::Initialize()
{
        /* Signals to recognize inside mainloop. */

        Signalizers();

	/* Checks whether we are allowed start the server as root. */
	
	if (!this->Config->usercmd.asroot)
	{
		this->Engine->CheckRoot();
	}

	/* 
	 * Checks whether the --nofork arg has been provided, and if so,
         * move to the background. 
         * Keep in mind that Beryl detaches to the background by default.
	 */
	  
	if (!this->Config->usercmd.nofork)
	{
		this->DaemonFork();
	}
	else
	{
	        do_newline;
	        bprint(INFO, "This server will %s detach (--nofork provided).", Daemon::Welcome("NOT").c_str());
        	do_newline;
	}
	
	/* Load configuration file. */

	this->Config->Load();
	
	/* Once our configuration file is loaded, we must set internal variables. */
	
	this->Config->SetAll();

	/* Opens core database. */
	
	this->Core->Open();
		
        /* Checks if this instance has ever been ran. */

        this->Core->CheckDefaults();

	/* Configuration file is valid and ready. */
	
	if (this->Config->Ready)
	{
		bprint(DONE, "Configuration file: %s", this->ConfigFile.c_str());
	}
	
	/* Opens all logs as provided in the configuration file. */
	
	this->Logs->OpenLogs();

	/* We must initialize our UID handler at this point. */
	
	this->Config->sid = UIDHandler::Create();

	/* Sets up the very first unique ID for future clients. */

	this->UID->Initialize();

	/* User that acts as a server. */
	
	this->Clients->Global = new GlobalUser(this->Config->sid, this->Config->ServerName);

	/* We are ready to start listening for connections. */
	
	this->StartListening();

        /* 
         * Resets all pending flushes. 
         * Given that ResetAll also removes timers, this function
         * should be called before loading all modules, which loads 
         * unflushed timers at startup.
         */
        
        DataFlush::ResetAll();

	/* Opens database processing threads. */

	this->Store->OpenAll();

        /* Open all databases. */

        this->Store->DBM->OpenAll();
	
        /* Loads all modules (both, core and modules will be loaded). */

        this->Modules->LoadAll();
	
	bprint(DONE, "PID: %u", getpid());

        /* If applies, Detach() sends BerylDB to the background. */

        this->Detach();
	
        /* server name in bold. */
        
        bprint(DONE, "Beryl is now running as '%s'", Daemon::Welcome(this->Config->ServerName).c_str());

	/* Saves beryldb.pid file so we can later make calls to our bin */
	
	this->Engine->SavePID();

	/* Run as different user. */
	
	this->Engine->RunAs();

	slog("STARTUP", LOG_DEFAULT, "BerylDB running as %s [%s], with max. sockets: %lu ", this->Config->ServerName.c_str(), Config->GetSID().c_str(), SocketPool::GetMaxFds());

        this->Core->UserDefaults();

	/* Checks whether user wants to run TestOffice. */
	
	this->CheckOffice();
	
	/* Start mainloop */

	this->Dispatcher();
}

void Beryl::Dispatcher()
{
	/* We wait 1 second before start processing requests. */

	usleep(PRELOOP_WAIT);

	/* We may now accept queries */
	
        this->Store->Flusher->Resume();
        
	/* Refresh timer before entering mainloop. */
	
	this->Refresh();
	
	time_t PREV_TIME = this->TIME.tv_sec;

	/* Main loop */

	do
	{
		this->Refresh();

                /* this->TIME should be updated in every loop. */

		if (this->TIME.tv_sec != PREV_TIME)
		{	
		    	PREV_TIME = TIME.tv_sec;
			
		    	/* Run functions that are meant to run every 1 second. */
		    	
		    	this->Timed(this->TIME.tv_sec);
		}

	        /* Signal detector. */

        	if (s_signal)
	        {
        		this->SignalManager(s_signal);
        		Beryl::s_signal = 0;
		}

	        /* Delivers pending data to clients. */

	        this->Clients->Flush(this->TIME.tv_sec);
		
		/* Main processes. */
		
		this->Loop();
	}		
	while (true);
}

void Beryl::Loop()
{
        /* Flushes pending commands. */

        this->Commander->Queue->Flush();

        /*
         * Our socket pool needs to actively await for data in active file descriptors.
         * The SocketPool handles all socket-related writes, from modules to clients.
         * In other words, these two functions below will read and/or write
         * events that are later routed.
         */

        SocketPool::Writes();
        SocketPool::Events();

	/* Removes all quitting clients. */
	
        this->Reducer->Apply();

        /* Dispatches both, pending queries and notifications. */
        
        DataFlush::Process();

        /* Delivers data to monitors. */

        this->Monitor->Flush();
        
        /* Pending notifications */
        
        this->Notify->Flush();

        /* Functions queued to run outside current loop. */
        
        this->Atomics->Run();
}

void Beryl::Timed(time_t current)
{
 	/* We call all pending timers to be delivered. */

        this->Tickers->Flush(current);
        
        /* Only works every 2 secs */
        
        if ((current % 2) == 0)
        {
	           /* Forces the exit of users with quitting status. */
	           
	           this->Clients->ForceExits();

 	           /* Function called every two seconds. */
 	           
                   NOTIFY_MODS(OnEveryTwoSeconds, (current));

                   if ((current % 30) == 0)
                   {
                             NOTIFY_MODS(OnEveryHalfMinute, (current));
                   }

                   if ((current % 3600) == 0)
                   {
			     /* 
			      * Login cache should only be in memory for
			      * an hour. There is no need to store ACL
			      * data for weeks.
			      */
			     
			     this->Logins->Reset();
			     
			     /* Runs every one hour. */
			     
			     NOTIFY_MODS(OnEveryHour, (current));
                   }
        }
         
        /* Removes expiring entries. */
 	
  	this->Store->Expires->Flush(current);        

  	/* Futures exerciser. */
  	
  	this->Store->Futures->Flush(current);
        
        /* 
         * CPU stats, we need this information to handle
         * overall usage of resources. 
         */
                    
        this->Engine->SnapshotStats();
}

sig_atomic_t Beryl::s_signal = 0;

void Beryl::Signalizer(int signal)
{
	s_signal = signal;
}

void Beryl::PrepareExit(int status, const std::string& quitmsg)
{
	Ready = false;
	
	/* Timers reset */
	
	this->Tickers->Reset();
	
        falert(NOTIFY_DEFAULT, "Server is preparing to shutdown.");
	
	/* One last flush. */
	
        this->Notify->Flush();

        /* Remove pending queues. */
        
        this->Commander->Queue->Reset();

	bprint(INFO, "Preparing exit: %s (code %i).", ExitMap[status], status);
	
	/* Log our exit: always a great idea to keep track of exiting date. */
	
	slog("EXIT", LOG_DEFAULT, "Preparing exit: %s (code %i).", ExitMap[status], status);
	
	/* Stop all monitoring and pending flushes. */
	
	this->Monitor->Reset();
	
	/* Login cache will not be needed anymore. */
	
	this->Logins->Reset();

	/* 
	 * ResetAll removes all pending flushes. 
	 *
	 * Flushes can be either notifications or pending 
	 * queries.
	 */
	
	DataFlush::ResetAll();
	
        /* Close all data threads */

        this->Store->Flusher->CloseThreads();

        /* No more requests will be processed. */

        this->Store->Flusher->Pause();
	
	/* Stop listening in all ports. */
	
	for (unsigned int i = 0; i < binds.size(); i++)
	{
		binds[i]->discard();
		delete binds[i];
	}
	
	binds.clear();

	/* We delete all pending expires. */
	
	this->Store->Expires->Reset();
	
	/* Removes pending futures. */
	
	this->Store->Futures->Reset();

        /* 
         * Removes PID file (if any). Keep in mind that this function
         * will not have any effect when running using --nofork.
         *
         * Just to recap, --nofork prevents a pid file to be written.
         */

        this->Engine->DeletePID();
	
	/* Modules should remove their local data. */
	
	NOTIFY_MODS(OnHalt, (SERVER_EXITING));

	/* Let's disconnect all active clients. */
	
	const ClientManager::LocalList& clients = this->Clients->GetLocals();
	
	/* Disconnecting counter. */
	
	unsigned int csize = 0;
	
	/* We disconnect all connected clients. */
	
	while (!clients.empty())
	{
		this->Clients->Disconnect(clients.front(), quitmsg);
		csize++;
	}
	
	/* Displays counter only if it is greater than zero. */
	
	if (csize > 0)
	{
		bprint(DONE, "%i client%s disconnected.", csize, csize > 1 ? "s" : "");
                slog("EXIT", LOG_DEFAULT, "%i client%s disconnected.", csize, csize > 1 ? "s" : "");
	}
	else
	{
		bprint(INFO, "No clients were disconnected.");
                slog("EXIT", LOG_DEFAULT, "No clients were disconnected.");
	}

	/* Reset channels. */
	
	this->Channels->Reset();	
	
	/* Runs pending reducers. */
	
	this->Reducer->Apply();

	/* It is now safe to unload all modules. */
	
	this->Modules->UnloadAll();

	/* Shuts down socket pool. */
	
	SocketPool::CloseAll();
	
	/* Close all databases */
	
        this->Store->DBM->CloseAll();
        
        /* Close core db. */
         
        this->Core->GetDatabase()->Close();

	/* Exit msg */
	
	if (this->GetUptime() > 1)
	{
		const std::string& exit = Daemon::Uptime("Exiting after uptime: ", this->GetUptime());
	
		/* Log uptime. */
		
	        slog("EXIT", LOG_DEFAULT, exit);
		bprint(INFO, exit);
	}

	/* Reset groups */
	
	this->Groups->Reset();
	
        /* Beryl should stop logging at this point. */
        
        this->Logs->CloseLogs();

        /* Configuration class is not needed anymore, so we reset it. */

        this->Config.reset();
        
	/* Cleans up config file. */
	
        this->ConfigFile.clear();

        /* Reset memory assignations. */
        
        memset(&this->PendingBuffer, BUFFERSIZE, sizeof(char));

        /* Free memory */
        
        this->Groups	  = 	NULL;
        this->Notify      =     NULL;
        this->Core        =     NULL;
        this->Store       =     NULL;
        this->Monitor     =     NULL;
        this->Logins      =     NULL;
        this->Channels    =     NULL;
        this->Config      =     NULL;
        this->Sets	  =	NULL;
        this->Engine	  =	NULL;
        this->Stats	  = 	NULL;
        
        /* The END. */

        Kernel            =     NULL;
}
