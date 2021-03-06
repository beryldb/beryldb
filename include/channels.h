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

#include "channelmanager.h"
#include "subscription.h"

/*
 * Class Channel maintains data of a given channel. This class
 * handles relevant data, such as subscribed users, name, and provides
 * basic functionalities, such as join and part functions.
 */

class ExportAPI Channel : public Expandable
{
   friend class ChannelManager;
   friend class User;
   friend class MessageTarget;
  
  public:
	
	/* Subscription map typedef. */
	
 	typedef std::map<User*, brld::aligned_storage<Subscription> > SubscriptionMap;

  private:

        /* This channel's name. */

        std::string name;

        /* Time at which this channel was created. */

        time_t created;
	
	/* 
	 * DeleteUser is invoked from PartUser() and removes references
	 * of a given subscription in this->subscribedlist.
	 */

	void DeleteUser(const SubscriptionMap::iterator& subsiter);
	
	/* Deletes an user from subscribedlist. */

	void DeleteUser(User* user);

  public:
	
	bool muted;
	
	/* Map that contains all subscribed clients to a given chan. */
	
	SubscriptionMap subscribedlist;

	/* Channel constructor. */
	
	Channel(const std::string &name, time_t current);
	
	/* 
	 * Returns a counter of all clients subscribed (joined)
	 * in a given channel.
	 */
	 
	size_t GetClientCounter() const 
	{ 
		return subscribedlist.size(); 
	}

	/*
	 * Adds a user to the reference list.
	 * 
	 * @parameters:
	 *
	 *           · User	: user to add.
	 */

	Subscription* AddUser(User* user);

	/* Returns a keyed map of subscribed users */

	const SubscriptionMap& GetInstances() const 
	{ 
		return subscribedlist; 
	}

	/* 
	 * Checks whether an user is subscribed to a channel. 
         *
         * @parameters:
	 *
	 *         · User: User to verify if subscribed,
	 * 
         * @return:
 	 *
         *         · True: User is subscribed.
         *         · False: User is not subscribed.
         */    

	bool IsSubscribed(User* user);

	/* Returns subscription data for a given user. */

	Subscription* GetUser(User* user);

	/* 
	 * Parts an user from channel.
         *
         * @parameters:
	 *
	 *         · User	   : User to unsubscribe.
	 * 
         * @return:
 	 *
         *         · bool (true)   : OK
         *         · bool (false)  : Operation failed.
         */    	
	
	bool PartUser(User* user);
	
	/* 
	 * Joins an user to a given channel.
         * 
         * @parameters:
	 *
	 *         · bool	: If this is a join calling from the core_db.cpp (database).
	 *         · user	: User to join.
	 *         · string	: Channel's name.
	 * 
         * @return:
 	 *
         *         · Channel: Reference to channel's dynamic object.
         */    	

	static Channel* JoinUser(bool fromconnect, LocalUser* user, std::string channame, bool override = false);

        /* 
         * Returns the name of this channel.
	 * 
         * @return:
 	 *
         *         · string	: Channel name.
         */    
         
	std::string GetName() const
	{
		return this->name;
	}
	
        /* 
         * Returns the time at which this channel was created.
	 * 
         * @return:
 	 *
         *         · time_t	: this->created.
         */    
         	
	time_t GetCreated()
	{
		return this->created;
	}
	
	/* Creates a channel if not exists. */
	
	Subscription* ExecuteJoin(bool fromconnect, User* user, bool bursting = false, bool created_by_local = false);
	
	void Write(ProtocolTrigger::Event& protoev, char status = 0, const DiscardList& except_list = DiscardList(), User* joining = NULL);

	void Write(ProtocolTrigger::EventProvider& protoevprov, ProtocolTrigger::Message& msg, char status = 0, const DiscardList& except_list = DiscardList(), User* joining = NULL);
};

inline bool Channel::IsSubscribed(User* user)
{
	return (this->subscribedlist.find(user) != this->subscribedlist.end());
}
