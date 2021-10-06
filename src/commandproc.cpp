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
#include "managers/user.h"
#include "extras.h"
#include "engine.h"
#include "maker.h"

bool Daemon::PassCompare(Expandable* ex, const std::string& data, const std::string& input, const std::string& hashtype)
{
	ModuleResult res;
	UNTIL_RESULT(OnPassCompare, res, (ex, data, input, hashtype));

	if (res == MOD_RES_OK)
	{
		return true;
	}

	if (res == MOD_RES_STOP)
	{
		return false;
	}

	if (!hashtype.empty() && !stdhelpers::string::equalsci(hashtype, "plaintext"))
	{
		return false;
	}

	return Daemon::TimingSafeCompare(data, input);
}

bool CommandHandler::HasLoop(User* user, Command* handler, const CommandModel::Params& parameters, unsigned int splithere, int extra, bool usemax)
{
	if (splithere >= parameters.size())
	{
		return false;
	}

	if (parameters[splithere].find(',') == std::string::npos)
	{
		return false;
	}

	brld::flat_set<std::string, engine::insensitive_swo> dupes;
	
	bool check_dupes = (extra < 0);

	engine::comma_node_stream items1(parameters[splithere]);
	engine::comma_node_stream items2(extra >= 0 ? parameters[extra] : "", true);
	std::string item;
	unsigned int max = 0;
	LocalUser* localuser = IS_LOCAL(user);
	
	CommandModel::Params splitparams(parameters);
	
	while (items1.items_extract(item) && (!usemax || max++ < MAX_DEST))
	{
		if ((!check_dupes) || (dupes.insert(item).second))
		{
			splitparams[splithere] = item;

			if (extra >= 0)
			{
				items2.items_extract(item);
				splitparams[extra] = item;
			}

			COMMAND_RESULT result = handler->Handle(user, splitparams);

			if (localuser)
			{
				item.clear();
				NOTIFY_MODS(OnPostCommand, (handler, splitparams, localuser, result, true));
			}
		}
	}

	return true;
}

Command* CommandHandler::GetBase(const std::string &commandname)
{
	CommandMap::iterator n = CommandList.find(commandname);
	
	if (n != CommandList.end())
	{
		return n->second;
	}

	return NULL;
}

COMMAND_RESULT CommandHandler::CallHandler(const std::string& commandname, const CommandModel::Params& parameters, User* user, Command** cmd)
{
	CommandMap::iterator n = CommandList.find(commandname);

	if (n != CommandList.end())
	{
		if ((!parameters.empty()) && (parameters.back().empty()) && (!n->second->last_empty_ok))
		{
			return INVALID;
		}

		if (parameters.size() >= n->second->min_params)
		{
			bool cready = false;

			if (IS_LOCAL(user) && n->second->flags)
			{
				if (user->CanPerform(n->second->flags))
				{
					cready = true;
				}
			}
			else
			{
				
				cready = true;
			}

			if (cready)
			{
				if (cmd)
				{
					*cmd = n->second;
				}

				ProtocolTrigger::TagMap tags;
				return n->second->Handle(user, CommandModel::Params(parameters, tags));
			}
		}
	}

	return INVALID;
}

void CommandHandler::Execute(LocalUser* user, std::string& command, CommandModel::Params& cmd_params)
{
	if (user->Paused)
	{
		/* PONG can still be sent if user is paused. */
		
		if (command != "PONG")
		{
			return;
		}
	}

	Command* handler = GetBase(command);

	if (!handler)
	{
		ModuleResult MOD_RESULT;
		UNTIL_RESULT(OnPreCommand, MOD_RESULT, (command, cmd_params, user, false));
		
		if (MOD_RESULT == MOD_RES_STOP)
		{
                         NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                         return;
		}

		handler = GetBase(command);
	
		if (!handler)
		{
		        /* Reports not found command to monitoring clients. */
		        
		        Kernel->Monitor->Push(user->instance, command, MONITOR_DEBUG, cmd_params);
		
			if (user->registered == REG_OK)
			{
				user->SendProtocol(ERR_INPUT, NOT_FOUND);
			}

			Kernel->Stats->Unknown++;
                        NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                        return;
		}
	}

	if (handler->max_params && cmd_params.size() > handler->max_params)
	{
		const CommandModel::Params::iterator lastbase = cmd_params.begin() + (handler->max_params - 1);
		const CommandModel::Params::iterator firstexcess = lastbase + 1;

		for (CommandModel::Params::const_iterator i = firstexcess; i != cmd_params.end(); ++i)
		{
			lastbase->push_back(' ');
			lastbase->append(*i);
		}
		
		cmd_params.erase(firstexcess, cmd_params.end());
	}

	ModuleResult MOD_RESULT;
	UNTIL_RESULT(OnPreCommand, MOD_RESULT, (command, cmd_params, user, false));
	
	Kernel->Monitor->Push(user->instance, command, MONITOR_DEFAULT, cmd_params);

	if (MOD_RESULT == MOD_RES_STOP)
	{
                 NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                 return;
	}
	
	/* Touch base is updated, as long as command is not PONG or PUBLISH */
	
	if (command != "PONG" && command != "PUBLISH")
	{
		user->touchbase = Kernel->Now();
	}
	
	user->next_ping_check = Kernel->Now() + PING_INTVL;

	/* Verifies whether command requires special flags. */
	
	bool is_manager = false;
	
	if (handler->flags)
	{
		if (!user->CanPerform(handler->flags))
		{
			user->SendProtocol(ERR_INPUT2, ERR_NO_FLAGS, ACCESS_DENIED);
                        NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                        return;
		}
		else
		{
			is_manager = true;
		}
	}
	
	bool skip = false;
	
	if (!is_manager && handler->group)
	{
		if (!user->InGroup(handler->group))
		{
                        user->SendProtocol(ERR_INPUT2, ERR_NO_FLAGS, ACCESS_DENIED);
                        NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                        return;
		}
		else
		{
			skip = true;
		}	
	}
	
	if (!is_manager && !skip && handler->groups.size() > 0)
	{
		for (std::vector<unsigned char>::iterator i = handler->groups.begin(); i != handler->groups.end(); ++i)
		{
			if (!user->InGroup(handler->group))			
			{
        	                user->SendProtocol(ERR_INPUT2, ERR_NO_FLAGS, ACCESS_DENIED);
	                        NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                	        return;
			}
		}
	}
	
	if ((!cmd_params.empty()) && (cmd_params.back().empty()) && (!handler->last_empty_ok))
	{
		cmd_params.pop_back();
	}

	/* Verifies whether user provided required parameters. */
	
	if (cmd_params.size() < handler->min_params)
	{
		handler->MissingParameters(user, cmd_params);
                NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
                return;
	}

	if ((user->registered != REG_OK) && (!handler->pre_reg_ok))
	{
		handler->NotLogged(user, cmd_params);
                NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
	}
	else
	{
		UNTIL_RESULT(OnPreCommand, MOD_RESULT, (command, cmd_params, user, true));

		if (MOD_RESULT == MOD_RES_STOP)
		{
			NOTIFY_MODS(OnCommandBlocked, (command, cmd_params, user));
			return;
		}

                if (!handler->alias.empty())
                {
                        const std::string& CMD = handler->alias;
                        Kernel->Commander->Queue->Add(user, CMD, cmd_params);
                        return;
                }

		if (cmd_params.size())
		{
			if (handler->check_key >= 0)
			{
				if (cmd_params.size() >= (unsigned int)handler->check_key)
				{
                        		if (!CheckKey(user, cmd_params[handler->check_key], true))
                        		{
                                		return;
					}
				}
			}

        	        if (handler->check_num)
	                {
				if (cmd_params.size() >= (unsigned int)handler->check_num)
                	        {	
                	        	if (!is_number(cmd_params[handler->check_num], true))
					{
						user->SendProtocol(ERR_INPUT, MUST_BE_NUMERIC);
						return;
					}
                                } 
                        }
                
                        if (handler->check_value)
                        {
                        	if (!CheckFormat(user, cmd_params.back()))
                        	{
                         	      	 return;
				} 
			}
			
			if (handler->check_login >= 0)
			{
                                if (cmd_params.size() >= (unsigned int)handler->check_login)
                                {
                                	if (handler->check_exists)
                                	{
						const std::string& exists = UserHelper::Find(cmd_params[handler->check_login], "created");
					
					        if (exists.empty())
					        {
					                user->SendProtocol(ERR_INPUT, PROCESS_FALSE);
					                return;
						}
                                	}
                                	
                                	if (cmd_params[handler->check_login].length() < 3 || cmd_params[handler->check_login].length() > 15)
                                	{
                                		user->SendProtocol(ERR_INPUT, USER_MIMMAX_LENGTH);
                                		return;
   					}
                                
					if (!Kernel->Engine->ValidLogin(cmd_params[handler->check_login]))
					{
						user->SendProtocol(ERR_INPUT, INVALID_UNAME);
						return;
					}
				
					if (handler->check_root)
					{
						if (cmd_params[handler->check_login] == ROOT_USER)
						{
							user->SendProtocol(ERR_INPUT, PROCESS_ERROR);
							return;
						}
					}
				}
			}

        	        if (handler->check_hash >= 0)
        	        {
                        	if (cmd_params.size() >= (unsigned int)handler->check_hash)
                        	{
                                	if (!CheckKey(user, cmd_params[handler->check_hash], true))
                                	{
                                       		 return;
                                       	}
				}
		      }
		      
		      if (handler->run_conf)
		      {
  				Limiter Conf = GetLimits(user, handler->max_params, cmd_params);
				
				if (Conf->GetError())
				{
					return;
				}
				else
				{
					handler->offset = Conf->GetOffset();
					handler->limit  = Conf->GetLimit();
				}
		      }
                }

		COMMAND_RESULT result = handler->Handle(user, cmd_params);
		NOTIFY_MODS(OnPostCommand, (handler, cmd_params, user, result, false));
	}
}

void CommandHandler::Remove(Command* x)
{
	CommandMap::iterator n = CommandList.find(x->name);

	if (n != CommandList.end() && n->second == x)
	{
		CommandList.erase(n);
	}
}

void CommandHandler::ProcessBuffer(LocalUser* user, const std::string& buffer)
{
	ProtocolTrigger::ParseOutput parseoutput;

	if (!user->serializer->Parse(user, buffer, parseoutput))
	{
		return;
	}

        std::string& command = parseoutput.cmd;
        std::transform(command.begin(), command.end(), command.begin(), ::toupper);

	CommandModel::Params parameters(parseoutput.params, parseoutput.tags);
  
        this->Queue->Add(user, command, parameters);
}

bool CommandHandler::AddCommand(Command *cmd)
{
	if (this->CommandList.find(cmd->name) == this->CommandList.end())
	{
		this->CommandList[cmd->name] = cmd;
		return true;
	}
	
	return false;
}

CommandHandler::CommandHandler()
{

}

std::string CommandHandler::TranslateUIDs(const std::vector<InterpretationType>& to, const CommandModel::Params& source, bool prefix_final, CommandModel* custom_translator)
{
	std::vector<InterpretationType>::const_iterator types = to.begin();
	std::string dest;

	for (unsigned int i = 0; i < source.size(); i++)
	{
		InterpretationType t = TR_TEXT;

		if (types != to.end())
		{
			t = *types;
			types++;
		}

		bool last = (i == (source.size() - 1));

		if (prefix_final && last)
		{
			dest.push_back(':');
		}

		TranslateSingleParam(t, source[i], dest, custom_translator, i);

		if (!last)
		{
			dest.push_back(' ');
		}
	}

	return dest;
}

void CommandHandler::TranslateSingleParam(InterpretationType to, const std::string& item, std::string& dest, CommandModel* custom_translator, unsigned int paramnumber)
{
	switch (to)
	{
		case TR_INSTANCE:
		{
			User* user = Kernel->Clients->FindInstance(item);
			
			if (user)
			{
				dest.append(user->uuid);
			}
			else
			{
				dest.append(item);
			}
			
			break;
		}
		
		case TR_CUSTOM:
		{
			if (custom_translator)
			{
				std::string translated = item;
				custom_translator->EncodeParameter(translated, paramnumber);
				dest.append(translated);
			}
			
		    break;
		}
		
		default:
			
			dest.append(item);
		break;
	}
}

CommandQueue::CommandQueue()
{

}

void CommandQueue::Add(LocalUser* user, const std::string& command, CommandModel::Params& cmd_params)
{
	if (!user || user->IsQuitting())
        {
             return;
        }

        PendingCMD adding(user, cmd_params, command);

        if (command == "PONG")
        {       
                user->PendingList.push_back(adding);
                return;
        }

        if (command == "MULTIRESET")
        {
		user->PendingMulti.clear();
		user->MultiRunning = false;
	}
	else
	{        
	        if (user->Multi && command == "MULTI")
	        {
                	user->SendProtocol(ERR_INPUT2, ERR_MULTI, PROCESS_ERROR);
                	return;
		}

	        if (user->Multi && command != "MRUN")
	        {
	      		user->PendingMulti.push_back(adding);
	      		user->SendProtocol(BRLD_OK, "QUEUED");
	      		return;
		}
		
		if (command == "MRUN" && !user->Multi)
		{
			user->SendProtocol(ERR_INPUT2, ERR_MULTI, PROCESS_ERROR);
			return;
	        }
	}
        
        user->PendingList.push_back(adding);
}

void CommandQueue::Reset()
{
       const ClientManager::LocalList& clients = Kernel->Clients->GetLocals();

       for (ClientManager::LocalList::const_iterator u = clients.begin(); u != clients.end(); ++u)
       {
                  LocalUser* user = *u;

                  if (!user)
                  {
                        continue;
                  }
                  
                  user->PendingList.clear();
	}
}

void CommandQueue::Flush()
{
       if (!Kernel->Ready)
       {
        	return;
       }
       
       const ClientManager::LocalList& users = Kernel->Clients->GetLocals();
       
       if (!users.size())
       {
               return;
       }
      
       for (ClientManager::LocalList::const_iterator u = users.begin(); u != users.end(); ++u)
       {
               LocalUser* user = *u;

               if (user == NULL)
               {
                     continue;
               }

               if (user->IsQuitting())
               {
               	     user->PendingList.clear();
               }
               
               if (!user->PendingList.size())
               {
                     continue;
               }
               
               PendingCMD event = user->PendingList.front();

               /* PONGS are allowed at any time when processing queries, even when locked. */

               if (event.command == "PONG")
               {
                       user->PendingList.pop_front();
                       Kernel->Commander->Execute(user, event.command, event.cmd_params);
                       return;
               }
                  
               if (user->IsLocked())
               {
                 	continue; 
               }
               
               if (user->Multi && event.command == "MRUN")
               {
	               	user->MultiRunning = true;
	               	user->Multi = false;
	       }
	       
	       if (user->MultiRunning)
	       {        
  		      if (!user->PendingMulti.size())
                      {
                           user->MultiRunning = false;
                           continue;
                      }

	       	      PendingCMD m_event = user->PendingMulti.front();
	       	      Kernel->Commander->Execute(user, m_event.command, m_event.cmd_params);
	              user->PendingMulti.pop_front();
	              continue;
	        }
               
	       user->PendingList.pop_front();
               Kernel->Commander->Execute(user, event.command, event.cmd_params);
        }
}
