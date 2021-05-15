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

enum BRLD_PROTOCOL
{
	/* Beryldb internal protocol codes. */
	
	BRLD_CURRENT_TIME  		=		100,
	BRLD_SERVERNAME  		=		101,
	BRLD_CREATED  			=		102,
        BRLD_PONG                       =               103,
        BRLD_PUBLISH                    =               104,
        BRLD_ALERT                      =               105,
        BRLD_LOCAL_TIME                 =               106,
	BRLD_MYINFO 			=		107,
	BRLD_CONNECTED  	    	=		108, 
        BRLD_PING                       =               110,

        BRLD_LINKS                      =               111,
        BRLD_ENDOFLINKS                 =               112,
        BRLD_END_OF_SLIST               =               113,
        BRLD_INFO                       =               114,
	BRLD_MAP    		        = 		115, 
	BRLD_ENDMAP             	= 		116, 
	BRLD_MAPUSERS                   = 		117, 
	BRLD_STATUS                     = 		118,
	
	BRLD_TOTAL_VIEW                 = 		120,
	BRLD_LUSEROP                    = 		121,
	BRLD_CHANS_VIEW               	= 		122,
	BRLD_LOCAL_VIEW                 = 		123,
	BRLD_LOCAL_PEAK                 = 		124,
	BRLD_GLOBAL_VIEW                = 		125,
	BRLD_CLIENTSERVER               = 		126,
	BRLD_ENDOFCLIENT                = 		127,
	BRLD_FINGER_LIST                = 		128,
	BRLD_CHANNELCREATED             = 		130,
	 
        BRLD_MODLIST 			= 		131,
        BRLD_END_OF_MODLIST 		= 		132,
	BRLD_VERSION                    = 		133,
	BRLD_SLIST_REPLY                = 		134,
        BRLD_SYNTAX                     = 		135, 
        
        BRLD_FINGER_END                 = 		140,
        BRLD_DBQUERY_OK                 = 		141,
        BRLD_ERROR              	=       	142,
	BRLD_ENDOFINFO                  = 		143,
	BRLD_JOIN			=		145,
	BRLD_QUIT			=		146,
	BRLD_PART			=		147,
	BRLD_VIS_HOST          		= 		148,
	BRLD_UPTIME			=		149,
	BRLD_END_STATUS			=		150,
	
	BRLD_CPU_USE 			=		151,
	BRLD_ALLOCATION			=		152,
	BRLD_SIGNALS			=		153,
	BRLD_PFAULTS			=		154,
	BRLD_SWAPS			=		155,
	BRLD_CSW			=		156,
	BRLD_CPU_NOW			=		157,
	BRLD_QUITTING			=		158,
	BRLD_USER_ADD			=		159,
	BRLD_DB_ADD			=		160,
	
	BRLD_NEW_USE			=		161,
	BRLD_COMMANDS 			= 		162,
	BRLD_COMMANDSEND 		= 		163,
	BRLD_FLUSH			=		164,
	BRLD_CURRENT_USE		=		165,
	BRLD_ADD_FLAGS			=		166,
	BRLD_LOGIN_CHPASS		=		167,
	BRLD_LOGIN_DEL			=		168,
	BRLD_FLUSHING			=		169,
	BRLD_EXPIRE_ADD			=		170,
	
	BRLD_TTL			=		171,
	BRLD_PERSIST			=		172,
	BRLD_EXP_COUNT			=		173,
        BRLD_SHARED_MEM                 =       	174, 
        BRLD_COUNT			=		175,
        BRLD_USER_LIST			=		176,
        BRLD_USER_LIST_END		=		177,
        BRLD_VIEW_INFO			=		178,
        BRLD_EXPIRE_ITEM		=		179,
        BRLD_EXPIRE_END			=		180,
        
        BRLD_EXPIRE_BEGIN		=		181,
        BRLD_INFO_EXP_DEL		=		182,
        BRLD_EXP_DELETED		=		183,
        BRLD_FINGER_BEGIN		=		184,
	BRLD_LOGIN_FLAGS		=		185,
	BRLD_YOUR_FLAGS			=		186,
	BRLD_BEGIN_OF_MODLIST		=		187,
	BRLD_HSEARCH_BEGIN		=		188,
	BRLD_HSEARCH_END		=		189,
	BRLD_HSEARCH_ITEM		=		190,
	
	BRLD_HKEYS_END			=		191,
	BRLD_HKEYS_BEGIN		=		192,
	BRLD_HKEYS_ITEM			=		193,
	BRLD_FIND_BEGIN			=		194,
	BRLD_FIND_END			=		195,
	BRLD_FIND_ITEM			=		196,
	BRLD_FLUSH_DONE			=		197,
	BRLD_CONF_DEFAULTS		=		198,
	BRLD_CONF_VALUE			=		199,
	BRLD_CONF_OK			=		200,
	
	BRLD_CURRENT_DIR		=		201,
	BRLD_CURRENT_CONF		=		202,
	BRLD_CURRENT_LOG		=		203,
	BRLD_FLAG_INFO			=		204,
	BRLD_EXISTS			=		205,
	BRLD_LENGTH			=		206,
	BRLD_IN_CACHE			=		207,
	BRLD_RESET_CACHE		=		208,
	BRLD_DB_RESET			=		209,	
	BRLD_DB_RESET_TIMED		=		210,
	BRLD_LOGINS_BEGIN		=		211,
	BRLD_LOGINS_END			=		212,
	BRLD_LOGINS_LIST		=		213,
	BRLD_INSTANCE			=		214,
	BRLD_SUBS_LIST			=		215,
	BRLD_SUBS_LIST_END		=		216,
	BRLD_SUBS_LIST_BEGIN		=		217,
	BRLD_MOD_UNLOAD			=		218,
	BRLD_MOD_LOADED			=		219,
	BRLD_SHUTDOWN			=		220,
	BRLD_RESTART			=		221,
	BRLD_TYPE_RAND			=		222,
	BRLD_AGENT_SET			=		223,
	BRLD_SFLUSH			=		225,
	BRLD_SFLUSHED			=		225,
	BRLD_PARTALL			=		226,
	BRLD_LPOS 			=		227,
	BRLD_SEARCH_ITEM		=		228,
	BRLD_SEARCH_END			=		229,
	BRLD_SEARCH_BEGIN		=		230,
	BRLD_HASH			=		231,
	BRLD_LSEARCH_BEGIN		=		232,
	BRLD_LSEARCH_ITEM		=		233,
	BRLD_LSEARCH_END		=		234,
	BRLD_LFIND_ITEM			=		235,
	BRLD_LFIND_END			=		236,
	BRLD_LFIND_BEGIN		=		237,
	BRLD_PREPARE_OK			=		238,
	BRLD_HQUERY_SENT		=		239,
	BRLD_HOVER_OK			=		240,
	BRLD_HLIMITS_OK			=		241,
	BRLD_HPARAM_OK			=		242,
	BRLD_END_MASS_JOIN		=		243,
	BRLD_MASS_JOIN			=		244,
	
	/* Error protocol codes. */
	
	ERR_NO_INSTANCE               	= 		500,
	ERR_NOSUCHSERVER                = 		501,
	ERR_NO_CHAN               	= 		502, 
	ERR_UNLOAD_MOD          	=               503, 
	ERR_NORECIPIENT                 = 		505,
	ERR_PUB_NOT_PROVIDED            = 		506,
	ERR_DBQUERY_FAILED              = 		507,
	ERR_CANNOTSENDTOCHAN            = 		508,
	ERR_EXCESS_SUBS             	= 		509,
	ERR_NOSUCHSERVICE               = 		510,
	 
	ERR_CMD_NOFND              	= 		511,
	ERR_EMPTY_LOGIN                 = 		512,
	ERR_INVALID_LOGIN            	= 		513,
	ERR_INSTANCE_NOT_SUBS           = 		514,
	ERR_NOT_SUBSCRIBED              = 		515,
	ERR_CANTSETLOGIN                = 		516, 
        ERR_INVALID_CHAN                = 		517,
        ERR_CHANFULL                    =		518,
        ERR_LOGIN_NO_EXISTS		=		519,

	ERR_NOTREGISTERED               =	 	520,
	ERR_NEEDMOREPARAMS		=		521,
	ERR_AGENT_IS_SET           	= 		522,
	ERR_CHANOPRIVSNEEDED            = 		527, 
	ERR_USERSDONTMATCH              = 		529,
	ERR_CANTSENDTOINSTANCE          = 		530, 
	ERR_LOGIN_EXISTS		=		531,
	ERR_BAD_AGENT			=		534,
	ERR_DB_EXISTS			=		535,
	ERR_WRONG_PASS			=		536,
	ERR_MUST_BE_POS_INT		=		537,
	ERR_FLUSH			=		538,
	ERR_NO_FLAGS			=		539,
	ERR_PROTECTED_LOGIN		=		540,
	
	ERR_INVALID_PASS		=		541,
	ERR_INVALID_PARAM		=		542,
	ERR_EXPIRE			=		543,
	ERR_NOT_EXPIRE			=		544,
	ERR_PERSIST			=		545,
	ERR_WRONG_SYNTAX		=		546,
	ERR_NOT_ACCEPTING_C		=		547,
	ERR_USE				=		548,
	ERR_INVALID_RANGE		=		549,
	ERR_UNABLE_FLUSH		=		550,
	
	ERR_CONFIG_NOT_FOUND		=		551,
        ERR_MUST_BE_DOUBLE              =               552,
        ERR_MUST_BE_INT                 =               553,
        ERR_UNABLE_DBRESET		=		554,
        ERR_NO_ORIGIN			=		555,
        ERR_EXCEED_BUFFER        	=		556,
        ERR_LOGIN_NO_FLAGS		=		557,
        ERR_SFLUSH			=		558,
        ERR_PARTALL			=		559,
	ERR_UNKNOWN_HASH		=		560,
	ERR_HASH_NOT_FOUND		=		561,
	ERR_NO_HQUERY			=		562,
	ERR_GREAT_ZERO			=		563,
	ERR_HUNABLE_RESET		=		564,
	ERR_WRONG_HPARAMS		=		565
};
