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

/* Predefined exit codes */

enum EXIT_CODES
{
	EXIT_CODE_OK 		= 	0,		
	EXIT_CODE_CORE 		= 	1,			
	EXIT_CODE_CONFIG 	= 	2,			
	EXIT_CODE_LOG 		= 	3,			
	EXIT_CODE_FORK 		= 	4,			
	EXIT_CODE_ARGV 		= 	5,			
	EXIT_CODE_PID 		= 	6,			
	EXIT_CODE_SOCKETSTREAM 	= 	7,		
	EXIT_CODE_ROOT 		= 	8,			
	EXIT_CODE_MODULE 	= 	9,			
	EXIT_CODE_SIGTERM 	= 	10,		
	EXIT_CODE_DATABASE 	= 	11,
	EXIT_CODE_SIGINT 	= 	12,
	EXIT_CODE_SHUTDOWN 	= 	13,
	EXIT_CODE_THREADS 	= 	14,
	EXIT_CODE_COMMAND	=	15
};

/*
 * The ExitMap array is used to map EXIT_CODES enums with
 * a message. These map-enum associations can later be used to display an 
 * a proper exiting message.
 */

extern const char * ExitMap[];
