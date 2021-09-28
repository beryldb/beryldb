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

class ExportAPI GroupManager 
{
     private:
        
         /* A map of all groups */
          
         GroupMap GroupList;
        
     public:
         
         /* Constructor */
                  
         GroupManager();

         /* Destructor */
                  
         ~GroupManager();

        /* 
         * Adds a new group to GroupList.
         * 
         * @parameters:
	 *
	 *         · name: Group name.
	 *         · flags: Flags this group contains.
	 * 
         * @return:
 	 *
         *         · True: Group added successfuly.
         */             
         
         bool Add(const std::string& name, const std::string& flags);

        /* 
         * Finds a group within GroupList
         * 
         * @parameters:
	 *
	 *         · string	: Group name to find.
	 * 
         * @return:
 	 *
         *         · Group	: Group pointer, returns NULL if not 
         *                        found.
         */             
         
         std::shared_ptr<Group> Find(const std::string& name);

        /* 
         * Removes a group from the GroupList.
         * 
         * @parameters:
	 *
	 *         · string	: Name to add.
	 * 
         * @return:
 	 *
         *         · True: Group removed.
         *         · False: Unable to remove.
         */    
         
         bool Delete(const std::string& name);

        /* 
         * Resets flags for this group.
         * 
         * @parameters:
	 *
	 *         · string	: Name of group to reset.
	 * 
         * @return:
 	 *
         *        · boolean
         *
         *          - True	: Flags updated.
         *          - False     : Unable to update.
         */    
         
         bool ResetFlags(const std::string& name);

        /* 
         * Returns GroupList.
         * 
         * @return:
 	 *
         *         · GroupMap    : Groups.
         */             
         
         GroupMap& GetList()
         {
              return this->GroupList;
         }
         
         /* Resets all groups. */
         
         void Reset();
};

class ExportAPI Group
{
     friend class GroupManager;
     
     private:
 
         /* Time at which this group was added. */
         
         time_t created;
         
         /* Groups' name. */
         
         std::string name;
         
         /* Flags as expressed in a simple string */
         
         std::string rawflags;
         
     public:

         bool can_do_all;

         bool can_keys;

         bool can_list;

         bool can_geo;

         bool can_vector;

         bool can_expire;

         bool can_future;

         bool can_map;

         bool can_mmap;

         bool can_publish_chan;

         bool can_publish_clients;

         bool can_hints;

         bool can_dual_exec;
 
         /* Constructor */
    
         Group();

         /* Destructor */
    
         ~Group();

        /* 
         * Updates flag.
         * 
         * @parameters:
	 *
	 *         · string	: Flag to append.
	 * 
         * @return:
 	 *
         *         · True       : Updated successfuly.
         */    
         
         bool UpdateFlags(const std::string& flags);
         
        /* 
         * Inits a group by setting all variables to 
         * false.
         *
         * This function should be called right after 
         * creating a new group.
         */             
         
         void Init();

        /* 
         * Checks whether a given flag can be run.
         * 
         * @parameters:
	 *
	 *         · char	: Flag to check.
	 * 
         * @return:
 	 *
         *         · True	: Can be executed.
         *         · False	: Unable to execute.
         */    
         
         bool CanDo(unsigned char flag);
          
        /* 
         * Returns rawflags.
         * 
         * @return:
 	 *
         *         · string: flags.
         */    
         
         std::string GetFlags()
         {
              return this->rawflags;
         }

        /* 
         * Returns group's name.
         * 
         * @return:
 	 *
         *         · string	: Current name.
         */             
         
         std::string GetName()
         {
              return this->name;
         }
};


