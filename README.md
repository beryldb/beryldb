<div align="center">
![Logo](https://static.beryl.dev/smaller.png)
</div>

# BerylDB, a lightweight database.

<a target="_blank" href="https://github.com/beryldb/beryldb/actions"><img src="https://github.com/beryldb/beryldb/workflows/Linux%20build/badge.svg"></a>
<a target="_blank" href="https://github.com/beryldb/beryldb/actions"><img src="https://github.com/beryldb/beryldb/workflows/macOS%20Build/badge.svg"></a>
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Mailing List](https://img.shields.io/badge/email-google%20groups-4285F4 "beryldb@googlegroups.com")](https://groups.google.com/g/beryldb)
[![Twitter](https://img.shields.io/twitter/follow/beryldb?color=%23179CF0&logo=twitter&style=flat-square "@beryldb on Twitter")](https://twitter.com/beryldb)
[![Discord Server](https://badgen.net/badge/icon/discord?icon=discord&label)](https://discord.gg/23f6w9sgAd)
<br>

This README is a brief introduction to BerylDB. For extended information, you
can visit our documentation site at [docs.beryl.dev](https://docs.beryl.dev/).

## What is BerylDB?

BerylDB is a data structure server. Our database provides access to structures via a 
wide range of commands, which are later queued, processed, and dispatched. 
The server is written in C++ and is powered by [RocksDB](https://github.com/facebook/rocksdb).

With BerylDB, users have access to lists, maps, keys, and channel
subscriptions for their clients. 
Currently, the server is available for GNU/Linux, FreeBSD and macOS.

* Check BerylDB's [full list of commands](https://docs.beryl.dev/commands/).
* We are on [Twitter](https://twitter.com/beryldb).

A robust attention to performance and code cleanliness is an ongoing
priority when developing BerylDB. Our server aims at being crash-free while utilizing
minimum resources.

![Data](https://static.beryl.dev/model.png)

## Why use it?

BerylDB simplifies your system by allowing you to write fewer lines of code to store, retrieve, and manage data in your apps. 
For instance, if your app relies on sessions to keep users connected,
you may use BerylDB's key-value data storage. Then remove expiring keys with Beryl's automatic expire managing system.

## Installation

We recommend building BerylDB from the source. A guide on how to do this is available on [the BerylDB docs site](https://docs.beryl.dev/getstarted/).
BerylDB's development branches are available in [our GitHub repository](https://github.com/beryldb/beryldb).

NOTE: When running 'make', you should use the '-j4' argument, which allows you to compile 
using 4 cores. Feel free to change this number according to your CPU.

* Installing dependencies in **Debian** based systems:

```
$ sudo apt-get install librocksdb-dev build-essential
```

* Installing dependencies in **Redhat** based systems:

```
$ dnf install @development-tools
$ yum install git rocksdb-devel.x86_64
```

* Installing dependencies in **macOS**:

```
$ brew install rocksdb 
```

Clone BerylDB's latest stable version:

```
$ git clone --branch 1.0 https://github.com/beryldb/beryldb.git 
$ cd beryldb/
```

You may now configure BerylDB following these quick steps: 

```
$ ./configure
$ make -j4 install
```

BerylDB is compiled in verbose level by default. If you prefer to build with minimal
information, you can use:

```
$ make -j4 install VERBOSE=0
```

## Running BerylDB

To run BerylDB using the default configuration, type:

```
$ ./run/beryldb start
```

**NOTE**: BerylDB runs in the background by default. If you wish to avoid
forking, you can run the server using nofork:

```
$ ./run/beryldb start --nofork
```

Great! You may now run your very first BerylDB command using
[beryl-cli](https://github.com/beryldb/beryldb-cli).

Keep in mind that BerylDB uses port **6378** by default, so be sure to
maintain that port unused before running the server.

## Drivers

Drivers are used to interact with a given BerylDB instance. 
If you are familiar with BerylDB, you may notice that driver's functions
typically have the same name that its parent function.

* [PHP](https://github.com/beryldb/php-beryl) - Compatible with PHP >= 7.3
* [Node.js](https://github.com/beryldb/node-beryl)  - Compatible with node >= 15

## First queries

You may want to learn more about BerylDB and run your first queries using
beryl-cli:

* **[SET](https://docs.beryl.dev/commands/set.html)** is used to define a key. 
* **[GET](https://docs.beryl.dev/commands/get.html)** is used to retrieve a key.
* **[DEL](https://docs.beryl.dev/commands/del.html)** is what you need when removing a key.
* use **[EXISTS](https://docs.beryl.dev/commands/exists.html)** to verify whether a given key exists.
* **[STRLEN](https://docs.beryl.dev/commands/strlen.html)** is used to obtain a key's length.

```
beryl> SET hello "world"
OK
beryl> GET hello
"world"
beryl> STRLEN hello
5
beryl> ISMATCH hello "worl?"
1
beryl> DEL hello
OK
beryl> EXISTS hello
0
```

You can also run the **ls** command to obtain a counter in all structures:

```
beryl> ls
+-----------------+----------------------+
| Type            | Count                |
+-----------------+----------------------+
| KEYS            | 5                    |
| MAPS            | 7                    |
| LISTS           | 0                    |
| GEOS            | 12                   |
| MULTIMAPS       | 0                    |
| VECTORS         | 9                    |
| EXPIRES         | 1                    |
| FUTURES         | 0                    |
+-----------------+----------------------+
```

To search all keys:

```
beryl> search *
+-----------------+----------------------+
| Key             | Value                |
+-----------------+----------------------+
| hello           | "world"              |
| test            | "entry"              |
+-----------------+----------------------+
```

Take a look at all BerylDB's commands [here](https://docs.beryl.dev/commands/).

## Coremodules and Modules

BerylDB is mainly divided into two components: core modules and modules. These
can be found in these directories:

* `src/coremodules`: Contains BerylDB's core modules. These are required to
  		     run the server properly.

* `src/modules`: Contains optional modules. BerylDB can funcion without these.

Core modules include those components that are required in order to run basic
functionalities. For instance, [core_keys](https://github.com/beryldb/beryldb/tree/unstable/src/coremods/core_keys)
handles the SET and GET commands.

In the other hand, modules are optional components developed either by the Beryl team
or by third party developers. 

Feel free to code and submit your own modules.

## Discussions

In order to maintain a documentation that is easy to follow. We try to maintain documentation that is easy to
follow and try to make discussions understandable for everyone. Our code is actively changing and thus
having a brief discussion board is preferred. Join our [Google group](https://groups.google.com/g/beryldb) 
If you would like to learn more about Beryl's development process.

## Source code organization

Beryl's source code is in the `src/` directory. The Makefile, which is
created after configuring Beryl, is located in the root directory.

Inside the root directory, you will find:

* `src`: Contains BerylDB's implementation, written in C++.
* `include`: Contains BerylDB's headers.
* `etc`: Libraries developed by 3rd party organizations.
* `make`: Contains several Perl-based functions to compile BerylDB.

NOTE: BerylDB has changed a bit. Some functions and file names may have
changed. Hence, BerylDB's documentation may be closer to the ``1.0`` branch.
Nonetheless, the core structure is the same, tested extensively.

## Contributing

We are always welcoming new members. If you wish to start contributing code to the 
Beryl project in any form, such as in the form of pull requests via Github, 
a code snippet, or a patch, you will need to agree to release your work under the terms of the
BSD license.

## Join our community 👋

We invite people from different backgrounds 🌈👨❤️ :raising_hand::older_man:  

If you are just getting started as programmer, there are several ways that you can
collaborate. There is no need to be a senior programmer. At BerylDB, we
are problem solvers and welcome people having this vision 👍

### How do I get involved?
 
 - 🎓 Check our pending issues, or create your own.
 - 🌵 Contribute to our Drivers ([Node.js](https://github.com/beryldb/node-beryl), [PHP](https://github.com/beryldb/php-beryl), Python).
 - 🙋 Become a QA: Test our software and report back ([Check our Google group](https://groups.google.com/g/beryldb)).
 - 💬 Get Involved and join our [Discord server](https://discord.gg/GRCEuMdYRt).

## External Links

* [Documentation](https://docs.beryl.dev)
* [GitHub](https://github.com/beryldb/beryldb)
* [Support/Discord](https://discord.gg/23f6w9sgAd)
* [Twitter](https://twitter.com/beryldb)


