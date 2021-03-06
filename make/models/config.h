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

#include <string>

/* Max databases constant. */

const unsigned int MAX_DATABASES	=	5;

/* Items to display in each loop. */

const unsigned int ITER_LIMIT 		= 	50;

/* Registry types. */

const std::vector<std::string> TypeRegs = 
{ 
    INT_LIST, 
    INT_KEY, 
    INT_GEO, 
    INT_MAP, 
    INT_MMAP, 
    INT_VECTOR, 
    INT_EXPIRE, 
    INT_FUTURE 
};

/* Core database */

const std::string CORE_DB		=	"core";

/* Root user */

const std::string ROOT_USER		=	"root";

/* Default emerald client. */

const std::string DEFAULT_EMERALD 	= 	"emerald";

/* Max. length of a line of text */

const unsigned int INPUT_LIMIT 		= 	65530;

/* Time to wait before starting accepting requests. */

const int PRELOOP_WAIT 			= 	1000000;

/* Max client target for a loop command. */

const int MAX_DEST 			= 	100;

/* Default configuration file */

const std::string DEFAULT_CONFIG 	= 	"beryldb.conf";

/* The default location that config files are stored in. */

const std::string CONFIG_PATH 		= 	"@CONFIG_DIR@";

/* Base path */

const std::string BASE_PATH 		= 	"@CORE_DIR@";

/* The default location that data files are stored in. */

const std::string DATA_PATH 		= 	"@DATA_DIR@";

/* Default database path. */

const std::string DB_PATH	 	= 	"@DB_DIR@";

/* The default location that log files are stored in. */

const std::string LOG_PATH 		= 	"@LOG_DIR@";

/* The default location that module files are stored in. */

const std::string MODULE_PATH 		= 	"@MODULE_DIR@";

/* The default location that runtime files are stored in. */

const std::string RUNTIME_PATH 		= 	"@RUNTIME_DIR@";

/* The URL of the BerylDB docs site. */

const std::string DOCS_URL 		= 	"https://docs.beryl.dev";

/* Number of processors available. */

const unsigned int CORE_COUNT 		= 	@CORE_COUNT@;

/* Default buffer size. */

const unsigned int BUFFERSIZE 		= 	65536;

/* The branch version that is shown to unprivileged users. */

const std::string BRANCH 		= 	"BerylDB-@VERSION_MAJOR@";

/* The full version that is shown to privileged users. */

#define VERSION "BerylDB-@VERSION_FULL@"

/* Determines whether this version of BerylDB is older than the requested version. */

#define VERSION_BEFORE(MAJOR, MINOR) (((@VERSION_MAJOR@ << 8) | @VERSION_MINOR@) < ((MAJOR << 8) | (MINOR)))

/* Determines whether this version of BerylDB is equal to or newer than the requested version. */

#define VERSION_SINCE(MAJOR, MINOR) (((@VERSION_MAJOR@ << 8) | @VERSION_MINOR@) >= ((MAJOR << 8) | (MINOR)))

%target include/config.h

/* Whether the clock_gettime() function was available at compile time. */

%define CLOCK_GETTIME_OK

