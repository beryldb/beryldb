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

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <cstdio>
#include <sstream>
#include <unistd.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>

#include "beryl.h"
#include "colors.h"
#include "stats.h"
#include "engine.h"
#include "exit.h"
#include "extras.h"

Daemon::Daemon() :    main_threadid(std::this_thread::get_id()),
                      PI(&DefaultProtocolInterface),
                      ValidChannel(&ChannelValidator), 
                      GenRandom(&DefaultGenRandom), 
                      ValidKey(&KeyValidator), 
                      ValidLogin(&LoginValidator), 
                      IsAgent(&AgentValidator),
                      IsDatabase(&DBValidator),
                      IsGroup(&GroupValidator)
{

}

Daemon::~Daemon()
{
      this->PI = NULL;
}

void Daemon::print_newline(unsigned int loops)
{
        for (unsigned int i = 0; i < loops; i++)
        {
            std::cout << std::endl;
        }
}

std::string Daemon::Welcome(std::string& msg)
{
        const std::string& casted = msg;
        return Daemon::Welcome(casted);
}

std::string Daemon::Welcome(const std::string& msg)
{
        std::stringstream welcome(static_cast<std::stringstream&&>(std::stringstream() << engine::color::bold << msg << engine::color::reset));
        return welcome.str();
}

void Daemon::CheckRoot()
{
	if (getegid() != 0 && geteuid() != 0)
	{
	    return;
        }

        bprint(INFO, "You should never run BerylDB as root. You are open to attacks that may gain access to your server.");
		
	if (isatty(fileno(stdout)))
	{
                bprint(ERROR, "BerylDB is starting in 20 secs. Use --asroot to disable this waiting time.");
    		sleep(20);
	}
	else
	{
                bprint(ERROR, "If you are required to run BerylDB as root, be sure to run it using the --asroot argument"); 				
		Kernel->Exit(EXIT_CODE_ROOT);
	}
}

std::string Daemon::GetRealPath(const char* path)
{
      char configuration_path[PATH_MAX + 1];

      if (realpath(path, configuration_path))
      {
           return configuration_path;
      }

      return path;
}

bool Daemon::SameChars(const std::string& str)
{
      for (unsigned int i = 1; i < str.length(); i++)
      {
           if (str[i] != str[0])
           {
               return false;
           }
      }
 
      return true;
}

bool Daemon::ChannelValidator(const std::string& chname)
{
        if (chname.empty() || chname.length() > 15)
        {
                return false;
        }

        if (chname[0] != '#' || chname == "#" || (chname[1] == '#' && Daemon::SameChars(chname)))
        {
                return false;
        }

        for (std::string::const_iterator i = chname.begin()+1; i != chname.end(); ++i)
        {
                switch (*i)
                {
                        case ' ':
                        case ',':
                        case 7:
                                return false;
                }
        }

        return true;
}

bool Daemon::GroupValidator(const std::string& gname)
{
        if (gname.empty() || gname.length() > 15)
        {
                return false;
        }
        
        if (!isalpha(gname))
        {
               return false;
        }

        for (std::string::const_iterator i = gname.begin()+1; i != gname.end(); ++i)
        {
                switch (*i)
                {
                        case ' ':
                        case ',':
                        case 7:
                                return false;
                }
        }

        return true;
}

void Daemon::DeletePID()
{
     const std::string rawfile = Kernel->Config->PID;
     std::string FileName = Kernel->Config->Paths->SetWDRuntime(rawfile.empty() ? "beryldb.pid" : rawfile);
     std::remove(FileName.c_str());
}

void Daemon::SavePID(bool exitonfail)
{
        if (!Kernel->Config->usercmd.writepid)
        {
                slog("STARTUP", LOG_DEFAULT, "--nopid specified on command line; PID file not written.");
                return;
        }

        const std::string& rawfile = Kernel->Config->PID;
        const std::string& FileName = Kernel->Config->Paths->SetWDRuntime(rawfile.empty() ? "beryldb.pid" : rawfile);
        std::ofstream outfile(FileName.c_str());

        if (outfile.is_open())
        {
                outfile << getpid();
                outfile.close();
        }
        else
        {
                if (exitonfail)
                {
                        bprint(ERROR, "Unable to write PID file: %s", FileName.c_str());
                }
                
                slog("STARTUP", LOG_DEFAULT, "Failed to write PID-file '%s'%s", FileName.c_str(), (exitonfail ? ", exiting." : ""));
                
                if (exitonfail)
                {
                        Kernel->Exit(EXIT_CODE_PID);
                }
        }
}

std::string Daemon::Format(va_list& vaList, const char* TempString)
{
	static std::vector<char> Buffer(1024);

	while (true)
	{
		va_list dst;
		va_copy(dst, vaList);

		int vsnret = vsnprintf(&Buffer[0], Buffer.size(), TempString, dst);
		va_end(dst);

		if (vsnret > 0 && static_cast<unsigned>(vsnret) < Buffer.size())
		{
			break;
		}

		Buffer.resize(Buffer.size() * 2);
	}

	return std::string(&Buffer[0]);
}

std::string Daemon::Format(const char* TempString, ...)
{
	std::string newstring;
	SCHEME(newstring, TempString, TempString);
	return newstring;
}

std::string Daemon::generate_random_str(unsigned int length, bool printable)
{
	std::vector<char> str(length);
	GenRandom(&str[0], length);

	if (printable)
	{
		for (size_t i = 0; i < length; i++)
		{
			str[i] = 0x3F + (str[i] & 0x3F);
                }
        }
                
	return std::string(&str[0], str.size());
}

unsigned long Daemon::generate_random_int(unsigned long max)
{
	unsigned long rv;
	GenRandom((char*)&rv, sizeof(rv));
	return rv % max;
}

void Daemon::DefaultGenRandom(char* output, size_t max)
{
	for (unsigned int i = 0; i < max; ++i)
	{
		output[i] = random();
        }
}

void Daemon::SnapshotStats()
{
        static rusage ru;

        if (getrusage(RUSAGE_SELF, &ru) == -1)
        {
                return; 
        }

        Kernel->Stats->LastSampled.tv_sec = Kernel->Now();
        Kernel->Stats->LastSampled.tv_nsec = Kernel->TimeStamp();
        Kernel->Stats->LastCPU = ru.ru_utime;
}

void Daemon::RunAs()
{
        config_rule* security = Kernel->Config->GetConf("security");

        const std::string SetGroup = security->as_string("runasgroup");

        if (!SetGroup.empty())
        {
                errno = 0;
        
                if (setgroups(0, NULL) == -1)
                {
                        bprint(ERROR, "setgroups() failed: %s", strerror(errno));
                        exit(EXIT_CODE_CONFIG);
                }

                struct group* groupname = getgrnam(SetGroup.c_str());
        
                if (!groupname)
                {
                        bprint(ERROR, "getgrnam(%s) failed: %s", SetGroup.c_str(), strerror(errno));
                        exit(EXIT_CODE_CONFIG);
                }

                if (setgid(groupname->gr_gid) == -1)
                {
                        bprint(ERROR, "setgid(%d) failed: %s", groupname->gr_gid, strerror(errno));
                        exit(EXIT_CODE_CONFIG);
                }
        }

        const std::string SetUser = security->as_string("runasuser");

        if (!SetUser.empty())
        {
                errno = 0;
                struct passwd* as_user = getpwnam(SetUser.c_str());
        
                if (!as_user)
                {
                        bprint(ERROR, "getpwnam(%s) failed: %s", SetUser.c_str(), strerror(errno));
                        exit(EXIT_CODE_CONFIG);
                }

                if (setuid(as_user->pw_uid) == -1)
                {
                        bprint(ERROR, "setuid(%d) failed: %s", as_user->pw_uid, strerror(errno));
                        exit(EXIT_CODE_CONFIG);
                }
        }
}

bool Daemon::ConfigFileExists(std::string& path)
{
        if (FileSystem::Exists(path))
        {
                return true;
        }

        return false;
}


void Beryl::Signalizers()
{
        signal(SIGALRM, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGXFSZ, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);
        signal(SIGUSR2, SIG_IGN);

        signal(SIGINT, Beryl::Signalizer);
        signal(SIGTERM, Beryl::Signalizer);
}

void Beryl::VoidSignalManager(int)
{
        exit(EXIT_CODE_OK);
}

void Beryl::DaemonFork()
{
        signal(SIGTERM, Beryl::VoidSignalManager);

        errno = 0;
        int childpid = fork();

        if (childpid < 0)
        {
                bprint(ERROR, "Unable to fork: %s", strerror(errno));
                Kernel->Exit(EXIT_CODE_FORK);
        }
        else if (childpid > 0)
        {

                while (kill(childpid, 0) != -1)
                {
                        sleep(1);
                }

                exit(EXIT_CODE_OK);
        }
        else
        {
                setsid();
                signal(SIGTERM, Beryl::Signalizer);
                SocketPool::SafeInit();
        }

}

void Daemon::SetCoreLimits()
{
        errno = 0;
        rlimit rl;

        if (getrlimit(RLIMIT_CORE, &rl) == -1)
        {
                bprint(ERROR, "Unable to increase dump size: %s", strerror(errno));
                return;
        }

        rl.rlim_cur = rl.rlim_max;

        if (setrlimit(RLIMIT_CORE, &rl) == -1)
        {
                bprint(ERROR, "Unable to increase dump size: %s", strerror(errno));
        }
}

std::string Daemon::HumanEpochTime(time_t curtime, const char* format, bool utc)
{
	struct tm* timeinfo = utc ? gmtime(&curtime) : localtime(&curtime);

	if (!timeinfo)
	{
		curtime = 0;
		timeinfo = localtime(&curtime);
	}

	if (timeinfo->tm_year + 1900 > 9999)
	{
		timeinfo->tm_year = 9999 - 1900;
        }
	else if (timeinfo->tm_year + 1900 < 1000)
	{
		timeinfo->tm_year = 0;
        }

	if (!format)
	{
		format = "%a %b %d %Y %H:%M:%S";
        }

	char buffer[512];
	
	if (!strftime(buffer, sizeof(buffer), format, timeinfo))
	{
		buffer[0] = '\0';
        }

	return buffer;
}

bool Daemon::TimingSafeCompare(const std::string& one, const std::string& two)
{
	if (one.length() != two.length())
	{
		return false;
        }

	unsigned int diff = 0;
	
	for (std::string::const_iterator i = one.begin(), j = two.begin(); i != one.end(); ++i, ++j)
	{
		unsigned char a = static_cast<unsigned char>(*i);
		unsigned char b = static_cast<unsigned char>(*j);
		diff |= a ^ b;
	}

	return (diff == 0);
}

void Daemon::iprintb(int number, const char *fmt, ...)
{
       std::string buff;
       SCHEME(buff, fmt, fmt);
       this->iprintb(number, buff);
}

void Daemon::iprintb(int number, const std::string& buff)
{
       if (!Kernel->Config->printbdebug)
       {
               return;
       }

       std::cout << "[ " << engine::color::green << number << engine::color::reset << " ] " << buff << engine::color::reset << "\r\t" << std::endl;
}



void Daemon::printb(const int type, const char *fmt, ...)
{
       std::string buff;
       SCHEME(buff, fmt, fmt);
       this->printb(type, buff);
}

void Daemon::printb(const int type, const std::string& buff)
{
       if (!Kernel->Config->printbdebug)
       {
               return;
       }

       switch (type)
       {
            case DONE:
                   
                    std::cout << "[ " << engine::color::green << PROCESS_OK << engine::color::reset << " ] " << buff << engine::color::reset << "\r\t" << std::endl;
                    break;

            case INFO:
                   
                    std::cout << "[ " << engine::color::blue << "INFO" << engine::color::reset << " ] " << buff  << engine::color::reset << "\r\t" << std::endl;
                    break;

            case ERROR:
                   
                    std::cout << "[ " << engine::color::red << "ERROR" << engine::color::reset << " ] " << buff << engine::color::reset << "\r\t" << std::endl;
                    break;
                    
            case WARNING:

                    std::cout << "[ " << engine::color::purple << "WARNING" << engine::color::reset << " ] " << buff << engine::color::reset << std::endl;
                    break;
                    
            case IDONE:
                    
                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[" << TIMESTR << "]" << "[" << engine::color::green << "DONE" << engine::color::reset << "] " << buff << engine::color::reset << std::endl;
                    }
                    
                    break;

            case IERROR:
                    
                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[" << TIMESTR << "]" << "[" << engine::color::red << "ERROR" << engine::color::reset << "] " << buff << engine::color::reset << std::endl;
                    }
                    
                    break;

            case IINFO:
                    
                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[" << TIMESTR << "]" << "[" << engine::color::blue << "INFO" << engine::color::reset << "] " << buff << engine::color::reset << std::endl;
                    }
                    
                    break;


            case FDONE:

                    
                    {
                            std::cout << "[ " << engine::color::green << "DONE" << engine::color::reset << " ]" << buff << engine::color::reset << "                      \t\r" <<  std::flush;

                    }
                    
                    break;

            case FINFO:

                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[ " << engine::color::blue << "INFO" << engine::color::reset << " ]" << "[" << TIMESTR << "] " << buff << engine::color::reset << "                       \t\r" <<  std::flush;
                    }

                    break;

            case FERROR:

                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[ " << engine::color::red << "ERROR" << engine::color::reset << " ]" << "[" << TIMESTR << "] " << buff << engine::color::reset << "                        \t\r" <<  std::flush;
                    }


                    break;

            case FWARNING:


                    {
                            std::string TIMESTR = Daemon::HumanEpochTime(Kernel->Now(), "%m/%d/%Y|%H:%M:%S");
                            std::cout << "[ " << engine::color::purple << "WARNING" << engine::color::reset << " ] " << "[" << TIMESTR << "] " << buff << engine::color::reset << "                   \t\r" <<  std::flush;
                    }

                    break;

            default:
                   
                    std::cout << buff << std::endl;
       }
}


bool Daemon::CheckRange(User* user, const std::string& value, const std::string& reason, int min, int max)
{
     int to_num = convto_num<int>(value);
     
     if (to_num >= min && to_num <= max)
     {
             return true;
     }
     
     user->SendProtocol(ERR_INPUT2, ERR_INVALID_RANGE, reason.c_str());
     return false;
}

std::string Daemon::Uptime(const std::string& msg, unsigned int up)
{
      std::string format;
      
      if (up > 172800)
      {
           format = Daemon::Format("%s%u days, %.2u:%.2u:%.2u", msg.c_str(), up / 86400, (up / 3600) % 24, (up / 60) % 60, up % 60);
      }
      else if (up > 86400)
      {
           format = Daemon::Format("%s%u day, %.2u:%.2u:%.2u", msg.c_str(), up / 86400, (up / 3600) % 24, (up / 60) % 60, up % 60);
      }
      else
      {
           format = Daemon::Format("%s%.2u:%.2u:%.2u", msg.c_str(), (up / 3600) % 24, (up / 60) % 60, up % 60);
      }
      
      return format;
}

void Dispatcher::TellThat(std::string& who, const std::string& msg, int rpl)
{
        User* user = Kernel->Clients->FindInstance(who);

        if (!user)
        {
                return;
        }

        user->SendProtocol(rpl, msg);
}

bool Dispatcher::CheckIterator(QueryBase* query)
{
       if ((query->user && query->user->IsQuitting()) || !Kernel->Store->Flusher->Status() || query->database->IsClosing())
       {
                query->access_set(DBL_INTERRUPT);
                return false;
       }
       
       return true;
}

void Dispatcher::Simple(User* user, const std::string& msg, const std::string& msg2)
{
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("| %-16s| %-29s|", msg.c_str(), msg2.c_str()));
        Dispatcher::JustEmerald(user, BRLD_START_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
}

void Dispatcher::SimpleEnd(User* user)
{
        Dispatcher::JustEmerald(user, BRLD_END_LIST, Daemon::Format("+%-16s+%-29s+", Dispatcher::Repeat("-", 17).c_str(), Dispatcher::Repeat("-", 30).c_str()));
}