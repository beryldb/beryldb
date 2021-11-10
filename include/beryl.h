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

#pragma once

#include <csignal>
#include <string>
#include <memory>
#include <vector>

#include "devel.h"
#include "constants.h"
#include "ilist.h"
#include "flat_map.h"
#include "adaptables.h"
#include "aligned_storage.h"
#include "forwards.h"
#include "typedefs.h"
#include "bmaps.h"

ExportAPI extern std::unique_ptr<Beryl> Kernel;

#include "config.h"
#include "dynref.h"
#include "reducer.h"
#include "serialize.h"
#include "expandable.h"
#include "chandler.h"
#include "protocols.h"
#include "numeric.h"
#include "uid.h"
#include "server.h"
#include "timer.h"
#include "nodes.h"
#include "logs.h"
#include "clientmanager.h"
#include "commandproc.h"
#include "socketstream.h"
#include "message.h"
#include "modules.h"
#include "clientprotocol.h"
#include "configprocess.h"
#include "brldstr.h"
#include "protocol.h"

int main(int argc, char** argv);

/*
 * Class Beryl: This is the main class of the server.
 * This class contains instances to other relevant classes,
 * such as storage, timers, clients and more.
 */
 
class ExportAPI Beryl
{
  private:
  	
	/* Configuration file. */

	std::string ConfigFile;

	/* Time this BerylDB instance was initialized. */
	
	time_t startup;
	
	/* Keeps track of current time. TIME is updated in the mainloop. */

	struct timespec TIME;
	
	/* A buffer used to read/write pending data. */
	
	char PendingBuffer[BUFFERSIZE];

	/* Major events handler, such as publish, join, etc. */
	
	ProtocolTrigger::BRLDEvents brldevents;

	/* 
	 * Generates a random seed, which is later
	 * used by some random functions. 
	 */
	
	void RandomSeed();
	
	/* Sends BerylDB to the background. */
        
	void Detach();

	/* Starts listening in provided ports. */
	
	void StartListening();
	
	/* 
	 * Prepares BerylDB for a shut down. 
	 *
         * @parameters:
	 *
	 *         · int	: Exit code.
	 *         · string	: Exit msg to display to clients.
         */    	        
         
	void PrepareExit(int status, const std::string& quitmsg);
	
	/* Handles signals. */
	
	void SignalManager(int signal);
	
	/* Runs every one second. Function meant to dispatch timers. */
	
	void Timed(time_t current);
	
	/* This function is called during every cycle. */
	
	void Loop();

	/* Checks if the test office should be run. */

	void CheckOffice();

	/* Tries to listen in X port. */
	
	bool ListenPort(config_rule* tag, const engine::sockets::sockaddrs& sa);

	/* Tries to listen in all ports in config file. */
		
	size_t TryListening(FailedListenings &failed_ports);

        /* CommandLine arguments reader. */

        void CommandLine();

        /* Refreshes TIME, this function is called in every loop. */

        void Refresh();
 
        static void Signalizers();

        static void VoidSignalManager(int);

        /* 
         * Sends Beryl to the background.
         * This function should be called only once, during startup.
         */    

        void DaemonFork();
      
  public:

	int timeout;
		
	bool Ready;
	
        /* Starts Beryl instance. */
       
        void Initialize();

	/* Ports where your BerylDB instance is listening to. */
	
	std::vector<BindingPort*> binds;

	/* Signals that are received to BerylDB. */
	
	static sig_atomic_t s_signal;

        /* Handles unique ids. */

        UIDHandler UID;

	/* Handles BerylDB's configuration files. */
	
	std::unique_ptr<Configuration> Config;
	
	/* Handles monitors. */
	
	std::unique_ptr<MonitorHandler> Monitor;

        /* Handles Cache and Session information */

        std::unique_ptr<LoginCache> Logins;

	/* Handles core databases. */
	
        std::unique_ptr<CoreManager> Core;
	
	/* Manages databases, both user-related and core ones. */
	
        std::unique_ptr<StoreManager> Store;
	
	/* Notification manager. */
	
	std::unique_ptr<Notifier> Notify;
	
	/* Handles groups */
	
	std::unique_ptr<GroupManager> Groups;
	
	/* Handles sets */
	
        std::unique_ptr<Settings> Sets;
	
        /* Handles subscriptions channels. */

        std::unique_ptr<ChannelManager> Channels;
        
        /* Utils function to the Beryl class. */

        std::unique_ptr<Daemon> Engine;
	
        /* Handles class objects that are awaiting to be removed. */

        ReducerHandler Reducer;

        /* Actions that will take place outside the current loop. */

        ActionHandler Atomics;

        ExtensionManager Extensions;

	/* Parses and processes user-provided commands. */
	
	CommandHandler Commander;
	
	/* Handles logs. This class is used actively during BerylDB. */
	
	LogHandler Logs;
	
	/* Manages modules. */
	
	ModuleHandler Modules;

	/* Internal statistics. */
	
	std::unique_ptr<Serverstats> Stats;
	
	/* Manages tick timers. */
	
	TickManager Tickers;

	/* Handles user's connections. */
	
	ClientManager Clients;
	
	/*
	 * Beryl's main. This function will initialize most of the Beryl's core system
	 * and read all configuration files.
         * 
         * @parameters:
	 *
	 *         · User commandline arguments.
	 * 
         * @return:
 	 *
         *         · int      : Beryl is exiting.
         */    

	Beryl(int argc, char** argv);
	
	/*
	 * This function contains Beryl's mainloop. The mainloop runs
	 * continuously, unless a signal is received.
	 */
	 
	void Dispatcher();

        /* 
         * Exits Beryl. This function will call PrepareExit() before exiting.
         * 
         * @parameters:
	 *
	 *         · int      : Exiting code.
	 *         · bool     : Whether to print a newline before exiting.
	 *         · bool     : Whether to skip PrepareExit().
	 *         · string   : Exit message. If exitmsg is defined, users
	 *                      will be sent this msg before disconnecting.
         */    	
         
	void Exit(int status, bool nline = false, bool skip = false, const std::string& exitmsg = "");

	/* Returns current time. */
	
	inline time_t Now() 
	{ 
		return this->TIME.tv_sec; 
	}
	
	/* Returns current time, as expressed in microseconds. */
	
	inline long TimeStamp() 
	{ 
		return this->TIME.tv_nsec; 
	}
	
	/* Returns startup time. */

	time_t GetStartup()
	{
		return this->startup;
	}
	
	/* Returns full path to configuration file. */
	
	const std::string& GetConfigFile()
	{
		return	this->ConfigFile;
	}

        /* 
         * Returns BerylDB's version.
         * 
         * @parameters:
         *
	 *        · bool    : Whether to return full or basic version.
	 * 
         * @return:
         *
         *        · string  : Version.
         */    
	 
	std::string GetVersion(bool Full = false);

	/* Sets a signal, which is later processed with SignalManager() */

	static void Signalizer(int signal);
	
	/* Returns pending buffer. */
	
	char* GetPendingBuffer()
	{
		return this->PendingBuffer;
	}
	
	/* Returns BRLD events. */

	ProtocolTrigger::BRLDEvents& GetBRLDEvents() 
	{ 
		return brldevents; 
	}

        /* 
         * Returns current uptime, as expressed in seconds.
	 * 
         * @return:
 	 *
         *         · uint  : Uptime in seconds.
         */    
         	
	unsigned int GetUptime()
	{
		return static_cast<unsigned int>(this->Now() - this->GetStartup());		
	}
};

#include "inlines.h"
#include "triggers.h"
#include "clientmsg.h"
#include "clientevent.h"

