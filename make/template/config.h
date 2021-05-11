/*
 * BerylDB - A modular database.
 * http://www.beryldb.com
 *
 * Copyright (C) 2015-2021 Carlos F. Ferry <cferry@beryldb.com>
 * 
 * This file is part of BerylDB. BerylDB is free software: you can
 * redistribute it and/or modify it under the terms of the BSD License
 * version 3.
 *
 * More information about our licensing can be found at https://docs.beryl.dev
 */

#pragma once

/* Max. length of a line of text */

const int INPUT_LIMIT = 65530;

/* Linux distro */

#define LINUX_DISTRO @LINUX_DISTRO@

/* Enables testsuite. */

#define ENABLE_TESTOFFICE

/* Time to wait before starting accepting requests. */

const int PRELOOP_WAIT = 1000000;

/* Max client target for a loop command. */

const int MAX_DEST = 100;

/* Default configuration file */

#define DEFAULT_CONFIG "beryldb.conf"

/* Number of processors available. */

#define CORE_COUNT @CORE_COUNT@

/* Default buffer size. */

const unsigned int BUFFERSIZE = 65536;

/* The branch version that is shown to unprivileged users. */

#define BRANCH "BerylDB-@VERSION_MAJOR@"

/* The full version that is shown to privileged users. */

#define VERSION "BerylDB-@VERSION_FULL@"

/* Determines whether this version of BerylDB is older than the requested version. */

#define VERSION_BEFORE(MAJOR, MINOR) (((@VERSION_MAJOR@ << 8) | @VERSION_MINOR@) < ((MAJOR << 8) | (MINOR)))

/* Determines whether this version of BerylDB is equal to or newer than the requested version. */

#define VERSION_SINCE(MAJOR, MINOR) (((@VERSION_MAJOR@ << 8) | @VERSION_MINOR@) >= ((MAJOR << 8) | (MINOR)))

/* The default location that config files are stored in. */

#define CONFIG_PATH "@CONFIG_DIR@"

/* The default location that data files are stored in. */

#define DATA_PATH "@DATA_DIR@"

/* Default database path. */

#define DB_PATH "@DB_DIR@"

/* The default location that log files are stored in. */

#define LOG_PATH "@LOG_DIR@"

/* The default location that module files are stored in. */

#define MODULE_PATH "@MODULE_DIR@"

/* The default location that runtime files are stored in. */

#define RUNTIME_PATH "@RUNTIME_DIR@"

/* The URL of the BerylDB docs site. */

#define DOCS "https://docs.beryl.dev"

%target include/config.h

/* Whether the clock_gettime() function was available at compile time. */

%define CLOCK_GETTIME_OK

