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

#include <thread>

#include "protocol.h"
#include "helpers.h"
#include "monitor.h"
#include "cstruct.h"
#include "filehandler.h"
#include "exit.h"

class ExportAPI Daemon : public safecast<Daemon>
{
    friend class Beryl;

    private:

        /* 
         * Records statistical information.
         * This function is called during every loop.
         */    
         
        void SnapshotStats();
        
        /* 
         * Run as another user.
         * This function reads configuration file looking for the 'runasuser' tag.
         * It will attempt to run as given user if available.
         */
             
        void RunAs();
        
        /* 
         * Checks whether configuration file exists.
         * 
         * @parameters:
	 *
	 *         · string	: Path to check.
	 * 
         * @return:
 	 *
 	 *        - bool
         *         · True       : File located.
         *         · False      : Unable to locate files.
         */            

        static bool ConfigFileExists(std::string& path);
        
        void SetCoreLimits();
        
    public:
    
    	/* Main thread */
    	
    	std::thread::id main_threadid;
        
        /* Constructor */
        
        Daemon();  
        
        /* Destructor */
        
        ~Daemon();

        ProtocolInterface DefaultProtocolInterface;
        
        /* Protocol interface. */
        
        ProtocolInterface* PI = NULL;

        /* Prints a new line */
                
        static void print_newline(unsigned int loops);

        /* 
         * Returns message in bold.
         * 
         * @parameters:
	 *
	 *         · string    : Text to turn bold.
	 * 
         * @return:
 	 *
         *         · string    : Message in bold.
         */    
        
        static std::string Welcome(const std::string& msg);

        static std::string Welcome(std::string& msg);
        
        /* Deletes PID file. */
        
        void DeletePID();

        /* 
         * Saves PID.
         * 
         * @parameters:
         *
         *         · exitonfail: Will Exit Beryl if unable to write.
         */

        void SavePID(bool exitonfail = true);

        STR1::function<bool(const std::string&)> ValidChannel;

        /* 
         * Validates a channel.
         * 
         * @parameters:
	 *
	 *         · channel: Channel to validate.
	 * 
         * @return:
 	 *
         *         · True: Valid channel.
         *         · False: Unable to validate channel.
         */    
         
        static bool ChannelValidator(const std::string& channel);

        /* 
         * Generates random string.
         * 
         * @parameters:
	 *         
	 *         · output: Where to return.
	 *         · max: String limit.
         */    
         
        static void DefaultGenRandom(char* output, size_t max);
        
        STR1::function<void(char*, size_t)> GenRandom;

        /* 
         * Checks for a valid key.
         * 
         * @parameters:
         *
         *         · string	:  Key to check.
         * 
         * @return:
         *
         *         - bool
         *            · True : Valid Key.
         *            · False : invalid.
         */

        static bool KeyValidator(const std::string& key);
        
        STR1::function<bool(const std::string&)> ValidKey;

        /* 
         * Validates provided login.
         * 
         * @parameters:
	 *
	 *         · string   : login to check.
	 * 
         * @return:
 	 *
         *         · True: Login has a correct format.
         *         · False: Poorly-formated login.
         */            

        static bool LoginValidator(const std::string& login);

        STR1::function<bool(const std::string&)> ValidLogin;

        /* 
         * Checks whether a valid agent has been provided.
         * 
         * @parameters:
	 *
	 *         · string	:  Agent to check.
	 * 
         * @return:
 	 *
 	 *        - bool 
         *         · True       : Valid agent.
         *         · False      : Invalid agent.
         */    
        
        static bool AgentValidator(const std::string& agent);

        STR1::function<bool(const std::string&)> IsAgent;

        /* 
         * Checks whether provided database name is valid.
         * 
         * @parameters:
	 *
	 *         · string	:	Database name to check.
	 * 
         * @return:
 	 *
         *         - bool	
         *            · true    : Valid database.
         *            · false	: Invalid name.
         */    
        
        static bool DBValidator(const std::string& name);

        STR1::function<bool(const std::string&)> IsDatabase;

        /* 
         * Validates a given group name.
         *  
         * @parameters:
	 *
	 *         · string   : Name to validate.
	 *  
         * @return:
 	 *
         *         · bool     : Whether provided name is valid.
         */    
         
        static bool GroupValidator(const std::string& gname);

        STR1::function<bool(const std::string&)> IsGroup;
        
        /* 
         * Determines whether a host has the proper format, for instance
         * server1.beryl.dev.
         * 
         * @parameters:
	 *
	 *         · host         : Host to check.
	 * 
         * @return:
 	 *
         *         · bool/True    : Valid host.
         */            
         
        static bool ValidHost(const std::string& host);
        
        static bool Valid_SID(const std::string& sid);
        
        std::string generate_random_str(unsigned int length, bool printable = true);
        
        /* 
         * Generates a random int.
         * 
         * @parameters:
	 *
	 *         · ulong   : Max number.
	 * 
         * @return:
 	 *
         *         · ulong   : Random number.
         */            
         
        unsigned long generate_random_int(unsigned long max);

        /* 
         * Checks if BerylDB had been started as root.
         *
         * @process:
         *
         *       · This function will exit if started using root, unless
         *         --asoot argument was provided.
         */

        void CheckRoot();

        /* 
         * Returns real path for a given string.
         * 
         * @parameters:
	 *
	 *         · path: Path to check.
	 * 
         * @return:
 	 *
         *         · string   : String formatted.
         */    
         
        std::string GetRealPath(const char* path);

        /* 
         * Formats an string.
         * 
         * @parameters:
	 *
	 *         · char*    :  printf like string to format.
	 * 
         * @return:
 	 *
         *         · string   : Formatted string.
         */    
       
        static std::string Format(const char* TempString, ...) BERYL_PRINTF(1, 2);
        
        static std::string Format(va_list& vaList, const char* TempString) BERYL_PRINTF(2, 0);

        /* 
         * Converts a given time_t timestamp into human-readable time.
         * 
         * @parameters:
	 *
	 *         · time_t   : Time to convert.
	 *         · char     : Optional format to use.
	 *         · bool     : Whether using utc or not.
	 * 
         * @return:
 	 *
         *         · string: Formated string.
         */    
         
        static std::string HumanEpochTime(time_t curtime, const char* format = NULL, bool utc = false);

        static bool TimingSafeCompare(const std::string& one, const std::string& two);

        /* 
         * Matches an string with a given mask.
         * 
         * @parameters:
	 *
	 *         · string	: String to check.
	 *         · string	: Mask to check against.
	 * 
         * @return:
 	 *
 	 *        - bool
         *         · True       : Provided wilcard Matches.
         *         · False      : No matching.
         */    
        
        static bool Match(const std::string& str, const std::string& mask, unsigned const char* map = NULL);
        
        static bool Match(const char* str, const char* mask, unsigned const char* map = NULL);
        
        static bool MatchCompactIP(const std::string& str, const std::string& mask, unsigned const char* map = NULL);

        static bool MatchCompactIP(const char* str, const char* mask, unsigned const char* map = NULL);
        
        static bool MatchMask(const std::string& masks, const std::string& hostname, const std::string& ipaddr);
        
        static unsigned long Duration(const std::string& str);
        
        static bool Duration(const std::string& str, unsigned long& duration);
        
        static bool IsValidDuration(const std::string& str);
        
        static std::string duration_as_string(time_t duration);

        static bool CheckRange(User* user, const std::string& value, const std::string& reason, int min, int max);
        
        bool PassCompare(Expandable* ex, const std::string& data, const std::string& input, const std::string& hashtype);

        /* 
         * Prints a colored-formated string.
         * 
         * @parameters:
	 *
	 *         · int        : String type (INFO, ERROR, DONE).
	 *         · string     : Buffer to print.
         */    
         
         void printb(const int type, const std::string& buff);

         void printb(const int type, const char *fmt, ...) BERYL_PRINTF(3, 4);
         
        /* 
         * Prints an colored notification with a counter.
         * 
         * @parameters:
	 *
	 *         · int	: Counter to print.
	 *         · string	: Buffer to print.
         */            
        
         void iprintb(int number, const std::string& buff);

         void iprintb(int number, const char *fmt, ...) BERYL_PRINTF(3, 4);
        
        /* 
         * Formats uptime into a string.
         * 
         * @parameters:
	 *
	 *         · string	: Message to incorporate.
	 *         · uint	: Uptime in seconds.
	 * 
         * @return:
 	 *
         *         · string	: Formatted string.
         */    
        
        static std::string Uptime(const std::string& msg, unsigned int up);

        /* 
         * Checks whether all characeters are the same.
         * 
         * @parameters:
	 *
	 *         · string  : String to check.
	 * 
         * @return:
 	 *
         *         · bool    : All chars are the same or not.
         */            
         
        static bool SameChars(const std::string& str);
};

class ExportAPI Dispatcher : public safecast<Dispatcher>
{
   public:
    
        static void SmartDiv(User* user, BRLD_PROTOCOL brld, const std::string& key, const std::string& msg, const std::string& div);

     
        /* 
         * Finds an user and sends a msg.
         * 
         * @parameters:
	 *
	 *         · string   : User to find.
	 *         · string   : Msg to send.
         */         
         
        static void TellThat(std::string& who, const std::string& msg, int rpl);      
     
        /* 
         * JustAPI transmits given data only if end-user is connected
         * via an API (not Emerald).
         *
         * @parameters:
	 *
	 *         · user   : User to send data to.
	 *         · brld   : Protocol.
         */    
        
        static void JustAPI(User* user, BRLD_PROTOCOL brld);
     
        /*  
         * Formats an item inside a list.
         * 
         * @parameters:
	 *
	 *         · User    : User to dispatch data to.
	 *         · brld    : Protocol to send.
	 *         · string  : First string.
	 *         · string  : Second string.
	 *         · bool    : Whether to add comillas or not.
         */    
         
        static void CondList(User* user, BRLD_PROTOCOL brld, const std::string& one_api, const std::string& second_api, bool comillas = false);
        
        /* 
         * Just flush provided data if destination client is emerald.
         * 
         * @parameters:
	 *
	 *         · string   : Message to send.
         */            
         
        static void JustEmerald(User* user, BRLD_PROTOCOL brld, const std::string& msg);

        /* 
         * Flushes data in a vector format.
         * 
         * @parameters:
	 *
	 *         · bool      : Whether to add comillas to our results.
	 *         · string    : Title that a given returning title has.
	 *         · QueryBase : Original query. 
         */            
         
        static void VectorFlush(bool comillas, const std::string& title, QueryBase* query);

        /* 
         * Flushes data in a multimap  format.
         * 
         * @parameters:
         *
         *         · bool      : Whether to add comillas to our results.
         *         · string    : Title that a given returning title has.
         *         · string    : Subtitle to utilize.
         *         · QueryBase : Original query. 
         */  
         
        static void MMapFlush(bool comillas, const std::string& title, const std::string& subtitle, QueryBase* query);
        
        /* 
         * This function is used when creating a string that cotains repeated 
         * data.
         * 
         * @parameters:
	 *
	 *         · string	: String to generate.
	 *  	   · times	: Times to repeat given strnig.
	 * 
         * @return:
 	 *
         *         · string	: String repeating.
         */            
         
        static std::string Repeat(const std::string& str, unsigned int times);

        static bool CheckIterator(QueryBase* query);
        
        static void ListDepend(User* user, BRLD_PROTOCOL brld, const std::string& msg, const std::string& msg2);

        static void Simple(User* user, const std::string& msg, const std::string& msg2);
        
        static void SimpleEnd(User* user);
        
        
};
