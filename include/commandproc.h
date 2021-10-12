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

struct PendingCMD
{
    public:
    
        LocalUser* user;
        CommandModel::Params cmd_params;
        std::string command;
        
        /* Constructor, sets variables. */
        
        PendingCMD(LocalUser* usr, const CommandModel::Params cmd_paramsarams, const std::string& cmd) : user(usr), cmd_params(cmd_paramsarams), command(cmd)
        {
        
        }
};

class ExportAPI CommandQueue : public safecast<CommandQueue>
{
   public:

         /* Constructor */
         
         CommandQueue();
      
        /* 
         * Adds a new command to the pending queue.
         * 
         * @parameters:
	 *
	 *         · user	: User that is requesting this new command.
	 *         · string	: Command requested.
	 *         · cmd_params : Command's parameters.
         */          

        void Add(LocalUser* user, const std::string& command, CommandModel::Params& cmd_params);

        /* 
         * Runs pending commands. 
         * This function is called from mainloop and runs constantly.
         */    
	
	bool Flush();

        /* Resets pending flushes. */
        
        void Reset();
};

class ExportAPI CommandHandler : public safecast<CommandHandler>
{ 
    friend class CommandQueue;
	
    public:

	 typedef STR1::unordered_map<std::string, Command*, engine::insensitive, engine::str_hash_comparison> CommandMap;

    private:

	 void Execute(LocalUser* user, std::string& command, CommandModel::Params& parameters);

	 CommandMap CommandList;

    public:

	 CommandQueue Queue;
		
	 /* Constructor */
	 
	 CommandHandler();

	 const CommandMap& GetCommands() const 
	 { 
		return CommandList; 
	 }

	 COMMAND_RESULT CallHandler(const std::string& commandname, const CommandModel::Params& parameters, User* user, Command** cmd = NULL);

 	 Command* GetBase(const std::string &commandname);

	 static bool HasLoop(User* user, Command* handler, const CommandModel::Params& parameters, unsigned int splithere, int extra = -1, bool usemax = true);

	 void ProcessBuffer(LocalUser* user, const std::string& buffer);

        /* 
         * Adds a command to the command handler.
         * 
         * @parameters:
	 *
	 *         · Command	: Command to add.
	 * 
         * @return:
 	 *
 	 *       - bool
 	 *
         *         · True	: Command added.
         *         · False	: Unable to add command.
         */    
         
	 bool AddCommand(Command *cmd);

        /* 
         * Removes a command.
         * 
         * @parameters:
	 *
	 *         · Command    : Command class to remove.
         */    
         
	 void Remove(Command* x);

	 static void TranslateSingleParam(InterpretationType to, const std::string& item, std::string& dest, CommandModel* custom_translator = NULL, unsigned int paramnumber = 0);

	 static std::string TranslateUIDs(const std::vector<InterpretationType>& to, const CommandModel::Params& source, bool prefix_final = false, CommandModel* custom_translator = NULL);
};

