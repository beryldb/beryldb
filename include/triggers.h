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

namespace Numeric
{
	class SendProtocolSink;
	class SendRemoteProtocolSink;

	template <char Sep, bool SendEmpty, typename Box>
	class GenericBuilder;

	template <char Sep = ',', bool SendEmpty = false>
	class Builder;

	template <unsigned int NumStaticParams, bool SendEmpty, typename Box>
	class GenericParamBuilder;

	template <unsigned int NumStaticParams, bool SendEmpty = false>
	class ParamBuilder;
}

class Numeric::SendProtocolSink
{
	LocalUser* const user;

 public:

	SendProtocolSink(LocalUser* u) : user(u)
	{

	}

	void operator()(Numeric& numeric) const
	{
		user->SendProtocol(numeric);
	}
};

class Numeric::SendRemoteProtocolSink
{
	User* const user;

 public:

	SendRemoteProtocolSink(User* u) : user(u)
	{

	}

	void operator()(Numeric& numeric) const
	{
		user->SendRemoteProtocol(numeric);
	}
};

template <char Sep, bool SendEmpty, typename Box>
class Numeric::GenericBuilder
{
	Box box;
	Numeric numeric;
	const std::string::size_type max;

	bool HasRoom(const std::string::size_type additional) const
	{
		return (numeric.GetParams().back().size() + additional <= max);
	}

 public:

	GenericBuilder(Box s, unsigned int num, bool addparam = true, size_t additionalsize = 0)
		: box(s)
		, numeric(num)
		, max(INPUT_LIMIT - Kernel->Config->GetServerName().size() - additionalsize - 10)
	{
		if (addparam)
		{
			numeric.push(std::string());
		}
	}

	Numeric& GetNumeric() 
	{ 
		return numeric; 
	}

	void Add(const std::string& entry)
	{
		if (!HasRoom(entry.size()))
		{
			Flush();
		}

		numeric.GetParams().back().append(entry).push_back(Sep);
	}

	void Add(const std::string& entry1, const std::string& entry2)
	{
		if (!HasRoom(entry1.size() + entry2.size()))
		{
			Flush();
		}

		numeric.GetParams().back().append(entry1).append(entry2).push_back(Sep);
	}

	void Flush()
	{
		std::string& data = numeric.GetParams().back();

		if (IsEmpty())
		{
			if (!SendEmpty)
				return;
		}
		else
		{
			data.erase(data.size()-1);
		}

		box(numeric);
		data.clear();
	}

	bool IsEmpty() const { return (numeric.GetParams().back().empty()); }
};

template <char Sep, bool SendEmpty>
class Numeric::Builder : public GenericBuilder<Sep, SendEmpty, SendProtocolSink>
{
 public:

	Builder(LocalUser* user, unsigned int num, bool addparam = true, size_t additionalsize = 0)
		: ::Numeric::GenericBuilder<Sep, SendEmpty, SendProtocolSink>(SendProtocolSink(user), num, addparam, additionalsize + user->instance.size())
	{
		
	}
};

template <unsigned int NumStaticParams, bool SendEmpty, typename Box>
class Numeric::GenericParamBuilder
{
	Box box;
	Numeric numeric;
	std::string::size_type currlen;
	std::string::size_type max;

	bool HasRoom(const std::string::size_type additional) const
	{
		return (currlen + additional <= max);
	}

 public:
	GenericParamBuilder(Box s, unsigned int num, size_t additionalsize)
		: box(s)
		, numeric(num)
		, currlen(0)
		, max(INPUT_LIMIT - Kernel->Config->GetServerName().size() - additionalsize - 10)
	{

	}

	void AddStatic(const std::string& entry)
	{
		max -= (entry.length() + 1);
		numeric.GetParams().push_back(entry);
	}

	void Add(const std::string& entry)
	{
		if (!HasRoom(entry.size()))
		{
			Flush();
		}

		currlen += entry.size() + 1;
		numeric.GetParams().push_back(entry);
	}

	void Flush()
	{
		if ((!SendEmpty) && (IsEmpty()))
		{
			return;
		}

		box(numeric);
		currlen = 0;
		numeric.GetParams().erase(numeric.GetParams().begin() + NumStaticParams, numeric.GetParams().end());
	}

	bool IsEmpty() const 
	{ 
		return (numeric.GetParams().size() <= NumStaticParams); 
	}
};

template <unsigned int NumStaticParams, bool SendEmpty>
class Numeric::ParamBuilder : public GenericParamBuilder<NumStaticParams, SendEmpty, SendProtocolSink>
{
 public:
 
	ParamBuilder(LocalUser* user, unsigned int num)
		: ::Numeric::GenericParamBuilder<NumStaticParams, SendEmpty, SendProtocolSink>(SendProtocolSink(user), num, user->instance.size())
	{
	
	}
};

namespace Protocols
{
	class NoChannel;
	class NoInstance;
}

class Protocols::NoChannel : public Numeric::Numeric
{
 public:
 
	NoChannel(const std::string& chan) : Numeric(ERR_NO_CHAN)
	{
		push(chan.empty() ? "*" : chan);
		push(PROCESS_NULL);
	}
};

class Protocols::NoInstance : public Numeric::Numeric
{
 public:
 
	NoInstance(const std::string& instance) : Numeric(ERR_NO_INSTANCE)
	{
		push(instance.empty() ? "*" : instance);
		push(PROCESS_NULL);
	}
};
