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
#include "queues.h"
#include "notifier.h"
#include "channelmanager.h"

namespace
{
	class DispatchQuit : public User::for_each_neighbor_handler
	{
		ProtocolTrigger::Messages::Quit quitmsg;
		ProtocolTrigger::Event quitevent;

		void Execute(LocalUser* user) 
		{
			user->Send(quitevent);
		}

	 public:

		DispatchQuit(User* user, const std::string& msg)
        			: quitmsg(user, msg)
	        		, quitevent(Kernel->GetBRLDEvents().quit, quitmsg)
		{
			user->for_each_neighbor(*this, false);
		}
	};

	/*void VerifyPingTimeouts(LocalUser* user)
	{
		if (Kernel->Now() < user->next_ping_check)
		{
			return;
		}
		
		if (user->IsLocked())
		{
			user->next_ping_check += PING_INTVL;
			return;
		}
		
		if (!user->lastping)
		{
			ModuleResult res;
			UNTIL_RESULT(OnConnectionFail, res, (user, L_ERR_TIMEOUT));
		
			if (res == MOD_RES_OK)
			{
				user->lastping = 1;
				user->next_ping_check = Kernel->Now() + PING_INTVL;
				return;
			}

			time_t secs = Kernel->Now() - (user->next_ping_check - PING_INTVL);
			const std::string message = "Ping timeout: " + convto_string(secs) + (secs != 1 ? " seconds" : " second");
			Kernel->Clients->Disconnect(user, message);
			return;
		}

		ProtocolTrigger::Messages::Ping ping;
		user->Send(Kernel->GetBRLDEvents().ping, ping);
		user->lastping = 0;
		user->next_ping_check = Kernel->Now() + PING_INTVL;
	}*/

	void VerifyRegistrationTimeout(LocalUser* user)
	{
		if (user->GetClass() && (Kernel->Now() > (user->GetConnected() + 10)))
		{
			Kernel->Clients.Disconnect(user, "Registration timeout");
		}
	}

	void VerifyModules(LocalUser* user)
	{
		ModuleResult res;
		UNTIL_RESULT(OnUserReady, res, (user));

		if (res == MOD_RES_SKIP)
		{
			user->ack_connection();
			return;
		}

		if (!user->IsQuitting())
		{
			VerifyRegistrationTimeout(user);
		}
	}
}

ClientManager::ClientManager(): dispatch_sent_id(0), Global(NULL), not_registered_track(0)
{

}

ClientManager::~ClientManager()
{
	for (UserMap::iterator i = clientlist.begin(); i != clientlist.end(); ++i)
	{
		delete i->second;
	}

	logins.clear();
}

void ClientManager::AddUser(int socket, BindingPort* via, engine::sockets::sockaddrs* client, engine::sockets::sockaddrs* server)
{
	LocalUser* const New = new LocalUser(socket, client, server);
	InstanceStream* ehandler = &New->usercon;

	this->not_registered_track++;
	this->clientlist[New->instance] = New;
	this->local_users.push_front(New);
	NOTIFY_MODS(OnInstanceInit, (New));

	if (!SocketPool::AddDescriptor(ehandler, Q_FAST_READ | Q_EDGE_WRITE))
	{
		this->Disconnect(New, "Internal error handling connection");
		return;
	}

	for (BindingPort::IOQueueProvList::iterator i = via->ioattachprovs.begin(); i != via->ioattachprovs.end(); ++i)
	{
		BindingPort::IOQueueProvRef& ioattachprovref = *i;

		if (!ioattachprovref)
		{
			if (!ioattachprovref.GetProvider().empty())
			{
				this->Disconnect(New, "Internal error handling connection");
				return;
			}
			
			continue;
		}

		ioattachprovref->OnAccept(ehandler, client, server);

		if (!ehandler->get_error().empty())
		{
			Disconnect(New, ehandler->get_error());
			return;
		}
	}

        if (this->local_users.size() > Kernel->Config->MaxClients)
        {
                this->Disconnect(New, "New connections are not allowed.");
                return;
        }

	New->set_class();

	if (New->IsQuitting())
	{
		return;
	}

	NOTIFY_MODS(OnSetUserIP, (New));

	if (!New->IsQuitting())
	{
		NOTIFY_MODS(OnInstancePostInit, (New));
	}
}

void ClientManager::EraseLogin(User* user)
{
        const LoginHash& users = this->GetLogins();

        for (LoginHash::const_iterator i = users.begin(); i != users.end(); ++i)
        {
	      if (i->second == user)
	      {
		    logins.erase(i);
	   	    break;
	      }
        }
}

void ClientManager::ForceExits()
{
	for (UserVector::iterator i = this->AwaitingExit.begin(); i != this->AwaitingExit.end(); )
	{
		User* user = *i;
		
		if (user->IsQuitting() && !user->IsLocked())
		{
			Kernel->Reducer.Add(user);
			this->AwaitingExit.erase(i);
		}
		else
		{
		 	++i;
	        }
	}
}

void ClientManager::Disconnect(User* user, const std::string& quitmessage)
{
	if (user->IsQuitting())
	{
		return;
	}
	
	if (IS_SERVER(user))
	{
		return;
	}

	std::string quitmsg(quitmessage);

	LocalUser* const localuser = IS_LOCAL(user);

	if (localuser)
	{
		ModuleResult MOD_RESULT;
		UNTIL_RESULT(OnUserPreExit, MOD_RESULT, (localuser, quitmsg));

		if (MOD_RESULT == MOD_RES_STOP)
		{
			return;
		}
	}

	user->SetQuit(true);
	
	if (localuser)
	{
		user->SendProtocol(BRLD_ERROR, quitmsg);
		ProtocolTrigger::Messages::Error errormsg(Daemon::Format("Closing connection: (%s@%s) [%s]", user->agent.c_str(), user->GetRealHost().c_str(), quitmsg.c_str()));
		localuser->Send(Kernel->GetBRLDEvents().error, errormsg);
	}

	if (!user->IsLocked())
	{
		Kernel->Reducer->Add(user);
	}
	else
	{
		AwaitingExit.push_back(user);
	}
	
	if (user->registered == REG_OK)
	{
		NOTIFY_MODS(OnUserExit, (user, quitmsg));
		DispatchQuit(user, quitmsg);
	}
	else
	{
		not_registered_track--;
	}

	if (IS_LOCAL(user))
	{
		LocalUser* lu = IS_LOCAL(user);
		NOTIFY_MODS(OnInstanceDisconnect, (lu));
		lu->usercon.Close();

		local_users.erase(lu);
	}

	clientlist.erase(user->instance);

	this->EraseLogin(user);
	
	uuid_list.erase(user->uuid);
	user->CheckEmptyChannels();

        falert(NOTIFY_DEBUG, "User disconnected: %s (%s)", user->login.c_str(), user->GetRealHost().c_str());
}

void ClientManager::NotifyAll(BRLD_PROTOCOL protocol, const char* text, ...)
{
        std::string message;
        SCHEME(message, text, text);

        for (LocalList::iterator i = local_users.begin(); i != local_users.end(); ++i)
        {
               LocalUser* user = *i;
               
               if (user->registered == REG_OK)
               {
	               user->SendProtocol(protocol, Daemon::Format("%s", message.c_str()).c_str());
	       }
        }
}

void ClientManager::PublishAll(const char* text, ...)
{
	std::string message;
	SCHEME(message, text, text);
	ProtocolTrigger::Messages::Publish msg(ProtocolTrigger::Messages::Publish::no_replicate, Kernel->Clients->Global, Kernel->Config->GetServerName(), message);
	ProtocolTrigger::Event msgevent(Kernel->GetBRLDEvents().publish, msg);

	for (LocalList::const_iterator i = local_users.begin(); i != local_users.end(); ++i)
	{
		LocalUser* user = *i;
		user->Send(msgevent);
	}
}

void ClientManager::Flush(time_t current)
{
	for (LocalList::iterator i = local_users.begin(); i != local_users.end(); )
	{
		LocalUser* curr = *i;
		++i;
	
		if (curr->usercon.GetQueueSize())
		{
			curr->usercon.StreamData();
		}

		switch (curr->registered)
		{
			case REG_OK:
			{
				if ((current % 10) == 0)
				{
			//		VerifyPingTimeouts(curr);
				}
			}	
			
			break;

			case REG_LOGINUSER:
			{
				VerifyModules(curr);
                        }
                        
			break;

			default:
			{
				VerifyRegistrationTimeout(curr);
                        }
                         
			break;
		}
	}
}

sent_id ClientManager::next_sentid()
{
	if (++dispatch_sent_id == 0)
	{
		dispatch_sent_id = 1;
		
		for (LocalList::iterator i = local_users.begin(); i != local_users.end(); ++i)
		{
			LocalUser* user = *i;
			user->already_sent = 0;
		}
	}

	return dispatch_sent_id;
}


User* ClientManager::FindInstance(const std::string &login)
{
	if (!login.empty() && isdigit(*login.begin()))
	{
		return FindUUID(login);
	}

	return FindInstanceOnly(login);
}

User* ClientManager::FindInstanceOnly(const std::string &instance)
{
	UserMap::iterator iter = this->clientlist.find(instance);

	if (iter == this->clientlist.end())
	{
		return NULL;
	}

	return iter->second;
}

User *ClientManager::FindUUID(const std::string &uid)
{
	UserMap::iterator user = this->uuid_list.find(uid);

	if (user == this->uuid_list.end())
	{
		return NULL;
	}

	return user->second;
}

unsigned int ClientManager::CountLogin(const std::string& login)
{
        const UserVector& AllLogins = Kernel->Clients->FindLogin(login);
        unsigned int counter = 0;
     
        for (UserVector::const_iterator u = AllLogins.begin(); u != AllLogins.end(); ++u)
        {
            counter++;
        } 
     
        return counter;
}

void ClientManager::RemoveGroup(const std::shared_ptr<Group> group)
{
        UserVector users;

        const LoginHash& AllLogins = Kernel->Clients.GetLogins();

        for (LoginHash::const_iterator i = AllLogins.begin(); i != AllLogins.end(); ++i)
        {
                User* user = i->second;
                user->RemoveGroup(group);
        }
}

UserVector ClientManager::FindGroup(const std::shared_ptr<Group> group)
{
        UserVector users;

        const LoginHash& AllLogins = Kernel->Clients.GetLogins();

        for (LoginHash::const_iterator i = AllLogins.begin(); i != AllLogins.end(); ++i)
        {
                User* user = i->second;

                if (user->HasGroup(group))
                {
                        users.push_back(user);
                }
        }

        return users;
}

User* ClientManager::FirstLogin(const std::string& login, registration_state state)
{
        const LoginHash& AllLogins = Kernel->Clients.GetLogins();

        for (LoginHash::const_iterator i = AllLogins.begin(); i != AllLogins.end(); ++i)
        {
                User* const user = i->second;

                if (i->first == login && user->registered == state)
                {
                        return user;
                }
        }

        return NULL;
}

UserVector ClientManager::FindPrivs(const std::string& flag)
{
	UserVector UserList;
	
	bool all = false;
	
	if (flag.empty() || flag.length() > 1)
	{
		all = true;	
	}
	
        const LoginHash& AllLogins = Kernel->Clients.GetLogins();

        for (LoginHash::const_iterator i = AllLogins.begin(); i != AllLogins.end(); ++i)
        {
		User* user = i->second;
		
		if (all)
		{
			if (user->session->CanAdmin() || user->session->CanExecute() || user->session->CanManage())
			{
				UserList.push_back(user);
			}
			
			continue;	
		}
		
		if (flag[0] == 'r')
		{
			if (user->session->CanAdmin())
			{
				UserList.push_back(user);
			}
			
			continue;
		}
		
		if (flag[0] == 'e')
		{
			if (user->session->CanExecute())
			{
				UserList.push_back(user);
			}
			
			continue;
		}
		
		if (flag[0] == 'a')
		{
			if (user->session->CanManage())
			{
				UserList.push_back(user);
			}
			
			continue;
		}
        }
        
        return UserList;
}

unsigned int ClientManager::DisconnectAll(const std::string& login, const std::string& msg)
{
         unsigned int counter = 0;
       
         const UserVector& logins = Kernel->Clients->FindLogin(login);

         for (UserVector::const_iterator i = logins.begin(); i != logins.end(); ++i)
         {
               User* const user = *i;
              
               if (user->login != login)
               {
		   continue;
               }
       		
               Kernel->Clients->Disconnect(user, msg);
               counter++;
         }
       
         return counter;
}

void ClientManager::ExitLogins(const std::string& login, const std::string& reason)
{
	 const UserVector& foundl = this->FindLogin(login);	
	
	 for (UserVector::const_iterator i = foundl.begin(); i != foundl.end(); ++i)
	 {
		User* user = *i;
		this->Disconnect(user, reason);
	 }
}

void ClientManager::Part(User* skip, const std::string& login, const std::string& channel)
{
         Channel* const chan = Kernel->Channels->Find(channel);

         if (!chan)
         {
              return;
         }

         const UserVector& FoundLogins = Kernel->Clients->FindLogin(login);

         for (UserVector::const_iterator i = FoundLogins.begin(); i != FoundLogins.end(); ++i)
         {
                   User* const user = *i;

                   if (user == skip)
                   {
                           continue;
                   }

                   chan->PartUser(user);
         }
}

UserVector ClientManager::FindLogin(const std::string& login, registration_state state)
{
         UserVector users;
         const LoginHash& AllLogins = Kernel->Clients.GetLogins();
        
         for (LoginHash::const_iterator i = AllLogins.begin(); i != AllLogins.end(); ++i)
         {
                User* user = i->second;

                if (i->first == login && user->registered == state)
                {
                        users.push_back(user);
                }
         }

         return users;
}

void ClientManager::Join(User* skip, const std::string& login, const std::string& channel)
{
         const UserVector& FoundLogins = Kernel->Clients->FindLogin(login);

         for (UserVector::const_iterator i = FoundLogins.begin(); i != FoundLogins.end(); ++i)
         {
              User* const user = *i;

              if (user == skip)
              {
                   continue;
              }

              LocalUser* localuser = IS_LOCAL(user);

              if (localuser)
              {
                    Channel::JoinUser(false, localuser, channel, true);
              }
         }
}
