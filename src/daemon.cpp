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

#include <fstream>
#include <dirent.h>

#include "beryl.h"
#include "engine.h"
#include "exit.h"

const char* ExitMap[] =
{
           "No errors took place",
           "Core error",
           "Config file error",
           "Logfile error",
           "Unable to fork",
           "Bad commandline parameters",
           "Unable to write PID file",
           "Problem with SocketPool",
           "Refusing to start up as root",
           "Unable to load module during boot",
           "Received SIGTERM",
           "Database error",
           "SIG INT received",
           "Shutdown command received",
           "Unable to create threads",
           "Invalid command"
};

unsigned const char *locale_case_insensitive_map = brld_case_insensitive_map;

void Beryl::RandomSeed()
{
	timespec current = Kernel->TIME;

#if !defined ARC4RANDOM_BUF_OK
        srandom(current.tv_nsec ^ current.tv_sec);
#endif
}

void Beryl::Refresh()
{
#if defined CLOCK_GETTIME_OK

	clock_gettime(CLOCK_REALTIME, &TIME);

#else

	struct timeval tv;
	gettimeofday(&tv, NULL);

	TIME.tv_sec = tv.tv_sec;
	TIME.tv_nsec = tv.tv_usec * 1000;

#endif
}

bool Daemon::LoginValidator(const std::string& login)
{
	/* Basic length validators. */
	
	if (login.empty() || login.length() < 3 || login.length() > 15)
	{
		return false;
	}

	for (std::string::const_iterator i = login.begin(); i != login.end(); ++i)
	{
		if ((*i >= 'A') && (*i <= '}'))
		{
			continue;
		}

		if ((((*i >= '0') && (*i <= '9')) || (*i == '-')) && (i != login.begin()))
		{
			continue;
		}

		return false;
	}

	return true;
}

bool Daemon::KeyValidator(const std::string& key)
{
        if (key.empty())
        {
                return false;
        }

        for (std::string::const_iterator i = key.begin(); i != key.end(); ++i)
        {
		if (*i == '*' || *i == ':')
		{
			return false;
		}	
        }

        return true;
}

bool Daemon::AgentValidator(const std::string& agent)
{
	if (agent.empty() || agent.length() < 3 || agent.length() > 15)
	{
		return false;
	}

	for (std::string::const_iterator i = agent.begin(); i != agent.end(); ++i)
	{
		if ((*i >= 'A') && (*i <= '}'))
		{
			continue;
		}

		if (((*i >= '0') && (*i <= '9')) || (*i == '-') || (*i == '.'))
		{
			continue;
		}

		return false;
	}

	return true;
}

bool Daemon::DBValidator(const std::string& name)
{
        if (name.empty() || name.length() < 3 || name.length() > 15)
        {
                return false;
        }

        for (std::string::const_iterator i = name.begin(); i != name.end(); ++i)
        {
                if ((*i >= 'A') && (*i <= '}'))
                {
                        continue;
                }

                if (((*i >= '0') && (*i <= '9')) || (*i == '-') || (*i == '.'))
                {
                        continue;
                }

                return false;
        }

        return true;
}

bool Daemon::ValidHost(const std::string& host)
{
	if (host.empty() || host.length() > 96)
	{
		return false;
	}

	unsigned int dash_count  = 0;
	unsigned int dots_count  = 0;
	
	bool dots_counted        = false;
	
	const std::string::const_iterator hostend = host.end() - 1;
	
	for (std::string::const_iterator iter = host.begin(); iter != host.end(); ++iter)
	{
		unsigned char chr = static_cast<unsigned char>(*iter);

		if (chr == '.')
		{
			dots_count++;
			
			if (dots_counted || dash_count || iter == host.begin() || iter == hostend)
			{
				return false;
			}

			dots_counted = true;
			continue;
		}

		dots_counted = false;

		if (chr == '-')
		{
			if (dots_counted || dash_count >= 2 || iter == host.begin() || iter == hostend)
			{
				return false;
			}

			dash_count += 1;
			continue;
		}
		
		dash_count = 0;

		if ((chr >= '0' && chr <= '9') || (chr >= 'A' && chr <= 'Z') || (chr >= 'a' && chr <= 'z'))
		{
			continue;
		}

		return false;
	}

	return dots_count;
}

bool Daemon::Valid_SID(const std::string &str)
{
	return ((str.length() == 3) && isdigit(str[0]) && ((str[1] >= 'A' && str[1] <= 'Z') || isdigit(str[1])) && ((str[2] >= 'A' && str[2] <= 'Z') || isdigit(str[2])));
}

static const unsigned int num_duration[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 86400, 0, 0, 0, 3600, 0, 0, 0, 0, 60, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 604800, 0, 31557600, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 86400, 0, 0, 0, 3600, 0, 0, 0, 0, 60, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 604800, 0, 31557600, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

bool Daemon::Duration(const std::string& str, unsigned long& duration)
{
	unsigned long total = 0;
	unsigned long subtotal = 0;

	for (std::string::const_iterator i = str.begin(); i != str.end(); ++i)
	{
		if ((*i >= '0') && (*i <= '9'))
		{
			subtotal = (subtotal * 10) + (*i - '0');
		}
		else
		{
			unsigned int multiplier = num_duration[static_cast<unsigned char>(*i)];

			if (multiplier == 0)
			{
				return false;
			}

			total += subtotal * multiplier;

			subtotal = 0;
		}
	}

	duration = total + subtotal;
	return true;
}

unsigned long Daemon::Duration(const std::string& str)
{
	unsigned long out = 0;
	Daemon::Duration(str, out);
	return out;
}

bool Daemon::IsValidDuration(const std::string& duration)
{
	for (std::string::const_iterator i = duration.begin(); i != duration.end(); ++i)
	{
		unsigned char c = *i;

		if (((c >= '0') && (c <= '9')))
		{
			continue;
		}

		if (!num_duration[c])
		{
			return false;
		}
	}
	
	return true;
}

std::string Daemon::duration_as_string(time_t duration)
{
	if (duration == 0)
	{
		return "0s";
	}

	time_t years    =   duration / 31449600;
	time_t weeks    =   (duration / 604800) % 52;
	time_t days     =   (duration / 86400) % 7;
	time_t hours    =   (duration / 3600) % 24;
	time_t minutes  =   (duration / 60) % 60;
	time_t seconds  =   duration % 60;

	std::string formatted;

	if (years)
	{
		formatted = convto_string(years) + "y";
	}
	
	if (weeks)
	{
		formatted += convto_string(weeks) + "w";
	}
	
	if (days)
	{
		formatted += convto_string(days) + "d";
	}
	
	if (hours)
	{
		formatted += convto_string(hours) + "h";
	}
	
	if (minutes)
	{
		formatted += convto_string(minutes) + "m";
	}
	
	if (seconds)
	{
		formatted += convto_string(seconds) + "s";
	}

	return formatted;
}

void Dispatcher::SmartDiv(User* user, BRLD_PROTOCOL brld, const std::string& key, const std::string& msg, const std::string& div)
{
        if (user->agent == DEFAULT_EMERALD)
        {
		user->SendProtocol(brld, Daemon::Format("%s%s %s", key.c_str(), div.c_str(), msg.c_str()));
        }
        else
        {
                user->SendProtocol(brld, key, msg);
        }
}

void Dispatcher::JustEmerald(User* user, BRLD_PROTOCOL brld, const std::string& msg)
{
        if (user->agent != DEFAULT_EMERALD)
        {
                 return;
        }
        
        user->SendProtocol(brld, msg);
}

std::string Dispatcher::Repeat(const std::string& str, unsigned int times)
{	
  	std::string result;
  	
	for (unsigned int i = 0; i < times; i++)
	{
		result.append(str);	
	}
	
	return result;
}

void Dispatcher::JustAPI(User* user, BRLD_PROTOCOL brld)
{
        if (user->agent == DEFAULT_EMERALD)
        {
        	 return;
        }
        
        user->SendProtocol(brld);
}

void Dispatcher::CondList(User* user, BRLD_PROTOCOL brld, const std::string& one_api, const std::string& second_api, bool comillas)
{
        if (user->agent == DEFAULT_EMERALD)
        {
             if (!comillas)
             {
                     user->SendProtocol(brld, Daemon::Format("%-29s | %5s", one_api.c_str(), second_api.c_str()).c_str());
	     } 
	     else
	     {
                     user->SendProtocol(brld, Daemon::Format("%s \"%s\"", one_api.c_str(), second_api.c_str()).c_str());
	     }
        }
        else
        {
             user->SendProtocol(brld, one_api.c_str(), second_api.c_str());
        }
}

void Dispatcher::ListDepend(User* user, BRLD_PROTOCOL brld, const std::string& msg, const std::string& msg2)
{
        if (user->agent == DEFAULT_EMERALD)
        {
             user->SendProtocol(brld, msg);
        }
        else
        {
             user->SendProtocol(brld, msg2);
        }
}

static bool MatchInternal(const unsigned char* str, const unsigned char* mask, unsigned const char* map)
{
	unsigned char* cp = NULL;
	unsigned char* mp = NULL;
	unsigned char* base = (unsigned char*)str;
	unsigned char* wildcard = (unsigned char*)mask;

	while ((*base) && (*wildcard != '*'))
	{
		if ((map[*wildcard] != map[*base]) && (*wildcard != '?'))
		{
			return 0;
		}

		wildcard++;
		base++;
	}

	while (*base)
	{
		if (*wildcard == '*')
		{
			if (!*++wildcard)
			{
				return 1;
			}

			mp = wildcard;
			cp = base+1;
		}
		else
			if ((map[*wildcard] == map[*base]) || (*wildcard == '?'))
			{
				wildcard++;
				base++;
			}
			else
			{
				wildcard = mp;
				base = cp++;
			}
	}

	while (*wildcard == '*')
	{
		wildcard++;
	}

	return !*wildcard;
}

bool Daemon::Match(const std::string& str, const std::string& mask, unsigned const char* map)
{
	if (!map)
	{
		map = locale_case_insensitive_map;
	}

	return MatchInternal((const unsigned char*)str.c_str(), (const unsigned char*)mask.c_str(), map);
}

bool Daemon::Match(const char* str, const char* mask, unsigned const char* map)
{
	if (!map)
	{
		map = locale_case_insensitive_map;
	}

	return MatchInternal((const unsigned char*)str, (const unsigned char*)mask, map);
}

bool Daemon::MatchCompactIP(const std::string& str, const std::string& mask, unsigned const char* map)
{
	if (engine::sockets::MatchCompactIP(str, mask, true))
	{
		return true;
	}

	return Daemon::Match(str, mask, map);
}

bool Daemon::MatchCompactIP(const char* str, const char* mask, unsigned const char* map)
{
	if (engine::sockets::MatchCompactIP(str, mask, true))
	{
		return true;
	}

	return Daemon::Match(str, mask, map);
}

bool Daemon::MatchMask(const std::string& masks, const std::string& hostname, const std::string& ipaddr)
{
	engine::space_node_stream maske_list(masks);
	std::string mask;

	while (maske_list.items_extract(mask))
	{
		if (Daemon::Match(hostname, mask, ascii_case_insensitive) || Daemon::MatchCompactIP(ipaddr, mask, ascii_case_insensitive))
		{
			return true;
		}
	}

	return false;
}

FileLoader::FileLoader(const std::string& filename)
{
	Load(filename);
}

void FileLoader::Load(const std::string& filename)
{
	cached_config::const_iterator it = Kernel->Config->Files.find(filename);
	
	if (it != Kernel->Config->Files.end())
	{
		this->lines = it->second;
	}
	else
	{
		const std::string real_name = Kernel->Config->Paths->SetWDConfig(filename);
		lines.clear();

		std::ifstream stream(real_name.c_str());
	
		if (!stream.is_open())
		{
			throw KernelException(filename + " does not exist.");
		}

		std::string line;
		
		while (std::getline(stream, line))
		{
			lines.push_back(line);
			Size += line.size() + 2;
		}

		stream.close();
	}
}

std::string FileLoader::as_string() const
{
	std::string buffer;
	
	for (file_cache::const_iterator it = this->lines.begin(); it != this->lines.end(); ++it)
	{
		buffer.append(*it);
		buffer.append("\r\n");
	}
	
	return buffer;
}

std::string FileSystem::GetRealPath(const std::string& base, const std::string& fragment)
{
	if (fragment[0] == '/')
	{
		return fragment;
	}

	if (!fragment.compare(0, 2, "~/", 2))
	{
		const char* home_directory = getenv("HOME");

		if (home_directory && *home_directory)
		{
			return std::string(home_directory) + '/' + fragment.substr(2);
		}
	}

	return base + '/' + fragment;
}

bool FileSystem::Exists(const std::string& file)
{
	struct stat sb;

	if (stat(file.c_str(), &sb) == -1)
	{
		return false;
	}

	if ((sb.st_mode & S_IFDIR) > 0)
	{
		return false;
	}

	return !access(file.c_str(), F_OK);
}

bool FileSystem::AsFileList(const std::string& directory, std::vector<std::string>& entries, const std::string& match)
{
	DIR* library = opendir(directory.c_str());

	if (!library)
	{
		return false;
	}

	dirent* entry = NULL;
	
	while ((entry = readdir(library)))
	{
		if (Daemon::Match(entry->d_name, match, ascii_case_insensitive))
		{
			entries.push_back(entry->d_name);
		}
	}
	
	closedir(library);
	return true;
}

std::string FileSystem::GetName(const std::string& name)
{
	size_t pos = name.rfind('/');
	return pos == std::string::npos ? name : name.substr(++pos);
}

int FileSystem::RemoveDir(const char *path) 
{
	DIR *dopen = opendir(path);
	size_t path_len = strlen(path);
	int answer = -1;

	if (dopen) 
	{
		struct dirent *p;

		answer = 0;
		
		while (!answer && (p = readdir(dopen))) 
		{
				int r2 = -1;
				char *buffer;
				size_t len;

				if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				{
					continue;
				}

				len    = path_len + strlen(p->d_name) + 2; 
				buffer = (char*)malloc(len);

				if (buffer) 
				{
					struct stat statbuf;

					snprintf(buffer, len, "%s/%s", path, p->d_name);
					
					if (!stat(buffer, &statbuf)) 
					{
							if (S_ISDIR(statbuf.st_mode))
							{
								r2 = RemoveDir(buffer);
							}
							else
							{
								r2 = unlink(buffer);
							}
					}
					
					free(buffer);
				}
				
				answer = r2;
			}
			
			closedir(dopen);
	}

	if (!answer)
	{
		answer = rmdir(path);
	}

	return answer;
}

void Dispatcher::VectorFlush(bool comillas, const std::string& title, QueryBase* query)
{
        if (query->subresult == 1)
        {
                Dispatcher::JustAPI(query->user, BRLD_START_LIST);
                
                Dispatcher::JustEmerald(query->user, BRLD_START_LIST, Daemon::Format("%-30s ", title.c_str()));
                Dispatcher::JustEmerald(query->user, BRLD_START_LIST, Daemon::Format("%-30s", Dispatcher::Repeat("―", 10).c_str()));
                
                if (query->VecData.empty())
                {
                       Dispatcher::JustAPI(query->user, BRLD_END_LIST);
                       return;
                }
        }
        
        for (StringVector::iterator i = query->VecData.begin(); i != query->VecData.end(); ++i)
        {            
        	 std::string item;
        	 
                 if (comillas)
                 {
                 	item = "\"" + *i + "\"";
		 }
		 else
		 {
		 	item = *i;
		 }
		 
                 Dispatcher::ListDepend(query->user, BRLD_ITEM_LIST, Daemon::Format("%-30s", item.c_str()), Daemon::Format("%s", item.c_str()));
        }

        if (!query->partial)
        {
                Dispatcher::JustAPI(query->user, BRLD_END_LIST);                 
        }
	
}

void Dispatcher::MMapFlush(bool comillas, const std::string& title, const std::string& subtitle, QueryBase* query)
{
        if (query->subresult == 1)
        {
                Dispatcher::JustAPI(query->user, BRLD_START_LIST);
                Dispatcher::JustEmerald(query->user, BRLD_START_LIST, Daemon::Format("%-30s | %-10s", title.c_str(), subtitle.c_str()));
                Dispatcher::JustEmerald(query->user, BRLD_START_LIST, Daemon::Format("%-30s | %-10s", Dispatcher::Repeat("―", 30).c_str(), Dispatcher::Repeat("―", 30).c_str()));

               if (query->mmap.empty())
               {
                       Dispatcher::JustAPI(query->user, BRLD_END_LIST);
                       return;
               }
                
        }

        for (DualMMap::iterator i = query->mmap.begin(); i != query->mmap.end(); ++i)
        {
                 std::string ikey = i->first;
                 std::string item;
                 
                 if (comillas)
                 {
		      item  = "\"" + i->second + "\"";
		 }
		 else
		 {
		     item = i->second;
		 }
		 
                 Dispatcher::ListDepend(query->user, BRLD_ITEM_LIST, Daemon::Format("%-30s | %-10s", ikey.c_str(), item.c_str()), Daemon::Format("%s %s", ikey.c_str(), item.c_str()));
        }

        if (!query->partial)
        {
                Dispatcher::JustAPI(query->user, BRLD_END_LIST);
        }
}