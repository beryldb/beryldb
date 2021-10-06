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

class Beryl;
class LiveSocket;
class Channel;
class ChannelManager;
class Command;
class config_status;
class config_rule;
class Expandable;
class GlobalUser;
class CoreDatabase;
class Database;
class QueueProvider;
class LocalUser;
class Subscription;
class Module;
class ProtocolServer;
class QueryBase;
class RemoteUser;
class Server;
class Configuration;
class User;
class ModuleResult;
class UserDatabase;
class DataThread;
class DataFlush;
class StoreManager;
class CoreManager;
class Database;
class SessionManager;
class Notifier;
class CommandQueue;
class Group;
class GroupManager;
class MonitorHandler;
class Daemon;
class Serverstats;

struct PendingCMD;

#ifdef ENABLE_TESTOFFICE

    class TestOffice;
    
#endif

struct ConfigConnect;

