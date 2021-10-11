# BerylDB, a lightweight database.

<a target="_blank" href="https://github.com/beryldb/beryldb/actions"><img src="https://github.com/beryldb/beryldb/workflows/Linux%20build/badge.svg"></a>
<a target="_blank" href="https://github.com/beryldb/beryldb/actions"><img src="https://github.com/beryldb/beryldb/workflows/macOS%20Build/badge.svg"></a>
[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Mailing List](https://img.shields.io/badge/email-google%20groups-4285F4 "beryldb@googlegroups.com")](https://groups.google.com/g/beryldb)
[![Twitter](https://img.shields.io/twitter/follow/beryldb?color=%23179CF0&logo=twitter&style=flat-square "@beryldb on Twitter")](https://twitter.com/beryldb)
[![Discord Server](https://img.shields.io/discord/823028202318200912?color=7289da&logo=discord "Discord Server")](https://discord.gg/GRCEuMdYRt)
<br>

This README is a brief introduction to Beryl. For extended information, you
can visit our documentation site at [docs.beryl.dev](https://docs.beryl.dev/).

## Join our community 👋

We invite people from different backgrounds 🌈👨❤️ :raising_hand::older_man:  

If you are just getting started as programmer, there are several ways that you can
collaborate. There is no need to be a senior programmer. At BerylDB, we
are problem solvers and welcome people having this vision 👍

### How do I get involved?
 
 - 🎓 Check our pending issues, or create your own.
 - 🌵 Contribute to our APIs (JS, [PHP](https://github.com/beryldb/php-beryl), Python).
 - 🙋 Become a QA: Test our software and report back ([Check our Google group](https://groups.google.com/g/beryldb)).
 - 💬 Get Involved and join our [Discord server](https://discord.gg/GRCEuMdYRt).

![Logo](https://docs.beryl.dev/img/smaller.png??)

## What is BerylDB?

Beryl is a data structure server. Our database provides access to structures via a 
wide range of commands, which are later queued, processed, and dispatched. 
The server is written in C++ and is powered by [RocksDB](https://github.com/facebook/rocksdb).

With Beryl, users have access to lists, maps, keys, and channel
subscriptions for their clients. 
Currently, the server is available for GNU/Linux, FreeBSD and macOS.

* Check Beryl's [full list of commands](https://docs.beryl.dev/using/commands/).
* We are on [Twitter](https://twitter.com/beryldb).

A robust attention to performance and code cleanliness is an ongoing
priority when developing BerylDB. Our server aims at being crash-free while utilizing
minimum resources.

## Why use it?

Beryl simplifies your system by allowing you to write fewer lines of code to store, retrieve, and manage data in your apps. 
For instance, if your app relies on sessions to keep users connected,
you may use Beryl's key-value data storage. Then remove expiring keys with Beryl's automatic expire managing system.

## Installation

We recommend building Beryl from the source. A guide on how to do this is available on [the Beryl docs site](https://docs.beryl.dev/using/installation/).
Beryl's development branches are available in [our GitHub repository](https://github.com/beryldb/beryldb).

NOTE: When running 'make', you should use the '-j4' argument, which allows you to compile 
using 4 cores. Feel free to change this number according to your CPU.

* Installing dependencies in **Debian** based systems:

```bash
sudo apt-get install librocksdb-dev build-essential
```

* Installing dependencies in **Redhat** based systems:

```bash
dnf install @development-tools
yum install git rocksdb-devel.x86_64
```

* Installing dependencies in **macOS**:

```bash
brew install rocksdb 
```

Clone Beryl's latest stable version:

```bash
git clone --branch 1.0 https://github.com/beryldb/beryldb.git 
cd beryldb/
```

You may now configure BerylDB following these quick steps: 

```bash
./configure
make -j4 install
```

Beryl is compiled in verbose level by default. If you prefer to build with minimal
information, you can use:

```console
make -j4 install VERBOSE=0
```

## Running Beryl

To run Beryl using the default configuration, type:

```console
./run/beryldb start
```

**NOTE**: Beryl runs in the background by default. If you wish to avoid
forking, you can run the server using nofork:

```console
./run/beryldb start --nofork
```

Great! You may now run your very first BerylDB command using
[Beryl-cli](https://github.com/beryldb/beryldb-cli).

Keep in mind that BerylDB uses port **6378** by default, so be sure to
maintain that port unused before running the server.

## APIs

APIs are used to interact with a BerylDB data-structure server. You can
typically execute the same exact command as if you were connecting from
the CLI.

* Check our [PHP API](https://github.com/beryldb/php-beryl).

## First queries

You may want to learn more about Beryl and run your first queries using
Beryl-cli:

* **set** is used to define a key. 
* **get** is used to retrieve a key.
* use **del** to remove a key.
* use **exists** to verify whether a given key exists.
* **strlen** is used to obtain a key's length.

```bash
beryl> set hello "world"
OK
beryl> get hello
"world"
beryl> strlen hello
5
beryl> ismatch hello "worl?"
1
beryl> del hello
OK
beryl> exists hello
0
```

You can also run the **ls** command to obtain a counter in all structures:

```bash
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

```bash
beryl> search *
+-----------------+----------------------+
| Key             | Value                |
+-----------------+----------------------+
| hello           | world                |
| test            | value                |
+-----------------+----------------------+
```

Take a look at all Beryl's commands [here](https://docs.beryl.dev/using/commands/).

## Coremodules and Modules

Beryl is mainly divided into two components: core modules and modules. These
can be found in these directories:

* `src/coremodules`: Contains Beryl's core modules. These are required to
  		     run the server properly.

* `src/modules`: Contains optional modules. Beryl can funcion without these.

Core modules include those components that are required in order to run basic
functionalities. For instance, [core_keys](https://github.com/beryldb/beryldb/tree/unstable/src/coremods/core_keys)
handles the SET and GET commands.

In the other hand, modules are optional components developed either by the Beryl team
or by third party developers. A good example of this can be [Hop](https://github.com/beryldb/beryldb/blob/unstable/src/modules/m_hop.cpp).
Hop allows users to unsubscribe and subscribe to a channel in just one command. 

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

* `src`: Contains Beryl's implementation, written in C++.
* `include`: Contains Beryl's headers.
* `etc`: Libraries developed by 3rd party organizations.
* `make`: Contains several Perl-based functions to compile BerylDB.

NOTE: Beryl has changed a bit. Some functions and file names may have
changed. Hence, Beryl's documentation may be closer to the ``stable`` branch.
Nonetheless, the core structure is the same, tested extensively.

## Contributing

We are always welcoming new members. If you wish to start contributing code to the 
Beryl project in any form, such as in the form of pull requests via Github, 
a code snippet, or a patch, you will need to agree to release your work under the terms of the
BSD license.


## External Links

* [Documentation](https://docs.beryl.dev)
* [GitHub](https://github.com/beryldb/beryldb)
* [Support/Discord](https://discord.gg/GRCEuMdYRt)
* [Twitter](https://twitter.com/beryldb)


