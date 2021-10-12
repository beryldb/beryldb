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

#include "moduledefs.h"
#include "dhandler.h"
#include "base.h"
#include "chandler.h"
#include "brldsocket.h"
#include "timer.h"

/*  Used to specify the nature of a given module. */

enum ModuleID
{
	/* Module has no specific id. */
	
	VF_NONE = 0,
	
	/* Module is part of the core. */

	VF_CORE = 1,
	
	/* Module comes with the original BerylDB tarball. */
	
	VF_BERYLDB = 2,
	
	/* Essential module. */

	VF_COMMON = 4,
	
	/* Optional module. */
	
	VF_OPTCOMMON = 8
};

#define MOD_RES_OK (ModuleResult(1))

#define MOD_RES_SKIP (ModuleResult(0))

#define MOD_RES_STOP (ModuleResult(-1))

/* Result from a module. */

class ModuleResult
{
   private:

	char result;

   public:

	ModuleResult() : result(0)
	{
	
	}

	explicit ModuleResult(char res) : result(res)
	{
	
	}

	inline bool operator==(const ModuleResult& res) const
	{
		return result == res.result;
	}
	
	inline bool operator!=(const ModuleResult& res) const
	{
		return result != res.result;
	}

	inline bool operator!() const
	{
		return !result;
	}

	inline bool check(bool def) const
	{
		return (result == 1 || (result == 0 && def));
	}

	inline ModuleResult operator+(const ModuleResult& res) const
	{
		if (result == res.result || res.result == 0)
		{
			return *this;
		}
		
		if (result == 0)
		{
			return res;
		}
		
		return MOD_RES_OK;
	}
};

#define NOTIFY_MODS(y,x) do { \
	const Module::List& _handlers = Kernel->Modules->EventHandlers[I_ ## y]; \
	for (Module::List::const_reverse_iterator _i = _handlers.rbegin(), _next; _i != _handlers.rend(); _i = _next) \
	{ \
		_next = _i+1; \
		try \
		{ \
			if (!(*_i)->exiting) \
				(*_i)->y x ; \
		} \
		catch (KernelException& modexcept) \
		{ \
			slog("MODULE", LOG_DEFAULT, "Exception thrown: " + modexcept.get_reason()); \
		} \
	} \
} while (0);

#define FOREACH_APP(n,v,args) \
do { \
	const Module::List& _handlers = Kernel->Modules->EventHandlers[I_ ## n]; \
	for (Module::List::const_reverse_iterator _i = _handlers.rbegin(), _next; _i != _handlers.rend(); _i = _next) \
	{ \
		_next = _i+1; \
		try \
		{ \
			if (!(*_i)->exiting) \
				v = (*_i)->n args;

#define WHILE_EACH_APP(n) \
		} \
		catch (KernelException& except_ ## n) \
		{ \
			slog("MODULE", LOG_DEFAULT, "Exception caught: " + (except_ ## n).get_reason()); \
		} \
	} \
} while(0)

#define UNTIL_RESULT(n,v,args) do { \
	v = MOD_RES_SKIP; \
	FOREACH_APP(n,v,args) \
	{ \
		if (v != MOD_RES_SKIP) \
			break; \
	} \
	WHILE_EACH_APP(n); \
} while (0)

/* contains information about a module. */

class ExportAPI Version
{
   public:
   
	/* Module's basic information. */

	const std::string description;
	
	/* Flags. */
	
	const int Flags;
	
	/* Server clustering data. */
	
	const std::string cluster_data;

	/* Basic version constructor. */
	
	Version(const std::string &desc, int flags = VF_NONE);

	/* contains basic data plus info about links. */

	Version(const std::string &desc, int flags, const std::string& clusterdata);
};

class ExportAPI DataProvider : public ServiceProvider
{
 
  public:

	DataProvider(Module* Creator, const std::string& Name) : ServiceProvider(Creator, Name, SERVICE_DATA) 
	{
	
	}
};
 
enum Priority 
{ 
	PRIORITY_FIRST, 
	PRIORITY_LAST, 
	PRIORITY_BEFORE, 
	PRIORITY_AFTER 
};

enum Application
{
	I_OnUserAdd,
	I_OnExpireAdd,
	I_OnFutureAdd,
	I_OnInstaceReady,
	I_OnAcceptConnection,
	I_OnAddLine,
	I_OnEveryTwoSeconds,
	I_OnEveryHalfMinute,
	I_OnNeighborBuild,
	I_OnHostSet,
	I_OnSetHost,
	I_OnSetAgent,
	I_OnChannelDelete,
	I_OnChannelPreDelete,
	I_OnUserReady,
	I_OnCommandBlocked,
	I_OnQueryFailed,
	I_OnConnectionFail,
	I_OnDecodeMetaData,
	I_OnExpireLine,
	I_OnEveryHour,
	I_InstanceLogout,
	I_OnLoadModule,
	I_OnAdmin,
	I_OnPassCompare,
	I_OnPostCommand,
	I_OnPostConnect,
	I_OnPostJoin,
	I_OnPreChangeHost,
	I_OnPreCommand,
	I_OnServiceAdd,
	I_OnServiceDel,
	I_OnSetConnectConfig,
	I_OnSetUserIP,
	I_OnHalt,
	I_OnUnloadModule,
	I_OnInstanceConnect,
	I_OnInstanceDisconnect,
	I_OnUserExit,
	I_OnInstanceInit,
	I_OnUserJoin,
	I_OnInstancePublish,
	I_OnInstancePublishBlocked,
	I_OnUserPart,
	I_OnInstancePostInit,
	I_OnInstancePostPublish,
	I_OnUserPostLogin,
	I_OnUserPreJoin,
	I_OnInstancePrePublish,
	I_OnUserPreLogin,
	I_OnUserPreExit,
	I_OnUserRegister,
	I_END
};

class ExportAPI Module : public Discarder, public usecountbase
{
  private:
 
	void DetachEvent(Application app);

   public:

	/* Module typedef. */
	
	typedef std::vector<Module*> List;

	/* Module source file. */
	
	std::string SourceFile;

	/* DLL Handler for this module. */
	
	DLLHandler* DLL;

	/* Indicates that this module will be unloaded soon. */

	bool exiting;

	/* Default constructor. */
	
	Module();

	/* Initializes a module. */
	
	virtual void Initialize() 
	{ 
	
	}

	DiscardResult discard() ;

	virtual ~Module();

	virtual void Prioritize() 
	{
	
	}

        /* 
         * Called the module manager invokes a module to load 
         * a configuration file.
         *
         * @parameters:
	 *
	 *         · config_status: Current configuration status.
         */    
	
	virtual void ConfigReading(config_status& status);

	/*
	 * Returns the version of a given module.
	 * This function should always return a Version object containing
	 * relevant information.
	 */

	virtual Version GetDescription() = 0;

	virtual void OnInstanceConnect(LocalUser* user);

	
	virtual ModuleResult OnUserPreExit(LocalUser* user, std::string& message);

        /* 
         * This overload is invoked everytime an user gets disconnected.
         * 
         * @parameters:
	 *
	 *         · User	: User disconnecting.
	 *         · string	: Reason of exiting.
         */    

	virtual void OnUserExit(User* user, const std::string &message);

        /* 
         * Called as an user gets disconnected.
         * 
         * @parameters:
	 *
	 *         · LocalUser  : User disconnecting.
         */    	
         
	virtual void OnInstanceDisconnect(LocalUser* local);

        /* 
         * Called as a channel is about to be disconnected.
         * 
         * @parameters:
	 *
	 *         · Channel to check.
         */    	
         
	virtual ModuleResult OnChannelPreDelete(Channel *chan);

	virtual void OnChannelDelete(Channel* chan);

        /* 
         * Method called before an user joins a channel.
         *
         * @parameters:
         *
         *         · Subscription	: Membership class.
         *         · bool		: True if channel is synced between servers.
         *         · bool		: true if this channel is created.
         */

	virtual void OnUserJoin(Subscription* memb, bool sync, bool created, DiscardList& except_list);

        /* 
         * Overload called whenever an user has joined a channel.
         * 
         * @parameters:
	 *
	 *         · Subscription	: Membership (user-chan relationship).
         */    
	
	virtual void OnPostJoin(Subscription* memb);

	virtual void OnUserPart(Subscription* memb, DiscardList& except_list);

	virtual ModuleResult OnUserPreJoin(LocalUser* user, Channel* chan, const std::string& cname);

	virtual void OnAdmin(User* user, const std::string &flags);

	virtual ModuleResult OnInstancePrePublish(User* user, const MessageTarget& target, MessageLoad& details);

	virtual void OnNeighborBuild(User* source, SubscriptionList& include_c, std::map<User*, bool>& exceptions);

	virtual ModuleResult OnUserPreLogin(LocalUser* user, const std::string& newlogin);

	virtual void OnInstancePostPublish(User* user, const MessageTarget& target, const MessageLoad& details);

	
	virtual void OnInstancePublish(User* user, const MessageTarget& target, const MessageLoad& details);

	
	virtual void OnInstancePublishBlocked(User* user, const MessageTarget& target, const MessageLoad& details);

	
	
	virtual void OnDecodeMetaData(Expandable* target, const std::string &extname, const std::string &extdata);

	
	virtual void OnHostSet(User* user, const std::string &newhost);

	
	virtual void OnSetHost(User* user, const std::string& newhost);
	
//2	virtual void OnUserAdd(

        /* 
         * Called every time a new expire is added.
         * 
         * @parameters:
	 *
	 *         · Expire information.
         */    
         
	virtual void OnExpireAdd(User* user, const std::string& dbname, const std::string& key, unsigned int select, unsigned int seconds);

        /* 
         * Called every time an future is added.
         * 
         * @parameters:
	 *
	 *         · Information about future.
         */    	
         
        virtual void OnFutureAdd(User* user, const std::string& dbname, const std::string& key, unsigned int select);

        /* 
         * Called as a new user registers its user agent.
         * 
         * @parameters:
	 *
	 *         · agent: Agent setting to user.
         */    
         
	virtual void OnSetAgent(User* user, const std::string &agent);


	virtual void OnCleanup(extension_item::ExpandableType type, Expandable* item);

	
	virtual void OnUserPostLogin(User* user, const std::string &oldlogin);

	
	virtual void OnInstaceReady(std::map<std::string, std::string>& tokens);

	virtual ModuleResult InstanceLogout(User* source, User* dest, const std::string &reason);

	virtual void OnLoadModule(Module* mod);

	virtual void OnUnloadModule(Module* mod);

	virtual void OnEveryTwoSeconds(time_t curtime);
	
	virtual void OnEveryHalfMinute(time_t curtime);

	
	virtual ModuleResult OnPreCommand(std::string& command, CommandModel::Params& parameters, LocalUser* user, bool validated);

	
	virtual void OnPostCommand(Command* command, const CommandModel::Params& parameters, LocalUser* user, COMMAND_RESULT result, bool loop);

	virtual void OnCommandBlocked(const std::string& command, const CommandModel::Params& parameters, LocalUser* user);
	
        /* 
         * Called as a query failed.
         * 
         * @parameters:
	 *
	 *         · DBL_CODE: Failing code. 
	 *         · bquery: Original query.
         */    
         	
	virtual void OnQueryFailed(DBL_CODE code, LocalUser* luser, std::shared_ptr<QueryBase> bquery);
	
	virtual void OnInstanceInit(LocalUser* user);

	
	virtual void OnInstancePostInit(LocalUser* user);

	
	virtual ModuleResult OnUserReady(LocalUser* user);

	
	virtual ModuleResult OnUserRegister(LocalUser* user);



		
	virtual ModuleResult OnPreChangeHost(LocalUser* user, const std::string &newhost);

	virtual ModuleResult OnPassCompare(Expandable* ex, const std::string &password, const std::string &input, const std::string& hashtype);

	virtual void OnPostConnect(User* user);
	
	virtual ModuleResult OnAcceptConnection(int fd, BindingPort* sock, engine::sockets::sockaddrs* client, engine::sockets::sockaddrs* server);

	virtual void OnEveryHour(time_t curtime);
	
	virtual ModuleResult OnSetConnectConfig(LocalUser* user, ConfigConnect* myclass);
	
	virtual void OnSetUserIP(LocalUser* user);

	virtual void OnServiceAdd(ServiceProvider& service);

	virtual void OnServiceDel(ServiceProvider& service);

	virtual ModuleResult OnConnectionFail(LocalUser* user, LiveSocketError error);

        /* 
         * Called as Beryl prepares to shutdown.
         * 
         * @parameters:
	 *
	 *         · string	: Shutdown reason.
         */    
         	
	virtual void OnHalt(const std::string& reason);
};

 
class ExportAPI ModuleHandler : public safecast<ModuleHandler>
{
  public:
 
	typedef std::multimap<std::string, ServiceProvider*, engine::insensitive_swo> DataProviderMap;
	typedef std::vector<ServiceProvider*> ServiceList;

  private:

	std::string last_known_error;

	std::map<std::string, Module*> Modules;

	enum 
	{
		PRIO_STATE_FIRST,
		PRIO_STATE_AGAIN,
		PRIO_STATE_LAST
	} PriorityState;

	void LoadCore(std::map<std::string, ServiceList>& ServiceMap);

	bool PrioritizeAttachs();

 public:

	typedef std::map<std::string, Module*> ModuleMap;

	Module::List EventHandlers[I_END];

	DataProviderMap DataProviders;

	ServiceList* new_services;

	static std::string FullModName(const std::string& name);

	/* Constructor. */
	
	ModuleHandler();

	~ModuleHandler();

	bool SetPriority(Module* mod, Application app, Priority s, Module* which = NULL);
	
	void SetPriority(Module* mod, Priority s);
	
	bool Attach(Application app, Module* mod);

	bool Detach(Application app, Module* mod);

	void Attach(Application* app, Module* mod, size_t sz);

	void DetachAll(Module* mod);
	
	void AttachAll(Module* mod);

	std::string& LastError();
	
	/* 
         * Loads a module.
         * 
         * @parameters:
	 *
	 *         · string	: File to load.
	 *         · bool	: Whether to load this module in the next loop.
	 * 
         * @return:
 	 *
         *         · True	: Module loaded.
         *         · False	: Unable to load module.
         */    
         
	bool Load(const std::string& filename, bool nextloop = false);

	bool Unload(Module* module);

        /* 
         * Loads all modules. This function should be called right after
         * booting beryl.
         */    
         
	void LoadAll();
	
	/* Unloads all modules. */
	
	void UnloadAll();
	
	/* Unloads a module in the next loop. */
	
	void SmartUnload(Module*);

	/*
	 * Checks if a module can be unloaded.
	 * 
	 * @parameters: 
	 *
	 *          · Module*	:	Module to be unloaded.
	 */

	bool CheckUnload(Module* module);

	/*
	 * Finds a module by a given name.
	 *
	 * @parameters:
	 *
	 *         · string	: Name to look for.
	 *
	 * @returns:
	 *
	 *         · Module*	: A pointer referencing given module.
	 */

	Module* Find(const std::string &name);

	void AttachService(ServiceProvider&);

	void UnAttachService(ServiceProvider&);
	 
	void AttachServices(const ServiceList& list);

	inline void AttachServices(ServiceProvider** list, int count)
	{
		for (int i = 0; i < count; i++)
		{
			AttachService(*list[i]);
		}
	}

	/* Find a service by its name. */

	ServiceProvider* FindService(ServiceID Type, const std::string& name);

	template<typename T> inline T* DataModule(const std::string& name)
	{
		return static_cast<T*>(FindService(SERVICE_DATA, name));
	}

	/* 
	 * Returns a map containing all modules.
	 * This mail is keyed by name and key is *module. 
	 */

	const ModuleMap& GetModules() const 
	{ 
		return this->Modules; 
	}

	/* Enables this module to be referenced. */

	void CreateReference(const std::string& name, ServiceProvider* service);

	/* Disables references. */

	void DelReference(ServiceProvider* service);

	/* Counts all modules loaded. */
	
	unsigned int Count() const
	{
		return this->Modules.size();
	}
};
