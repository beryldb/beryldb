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

#include <sys/epoll.h>
#include <sys/resource.h>

#include "beryl.h"

namespace
{
	int SocketHandler;
	std::vector<struct epoll_event> events(16);
}

void SocketPool::Start()
{
	get_max_fdesc();

	SocketHandler = epoll_create(128);
	
	if (SocketHandler == -1)
	{
		InitError();
	}
	
}

void SocketPool::SafeInit()
{
	
}

void SocketPool::CloseAll()
{
	Close(SocketHandler);
}

static unsigned SetMask(int req_mask)
{
	unsigned rv = 0;

	if (req_mask & (Q_REQ_POOL_READ | Q_REQ_POLL_WRITE | Q_SINGLE_WRITE))
	{
		if (req_mask & (Q_REQ_POOL_READ | Q_FAST_READ))
		{
			rv |= EPOLLIN;
		}
		
		if (req_mask & (Q_REQ_POLL_WRITE | Q_FAST_WRITE | Q_SINGLE_WRITE))
		{
			rv |= EPOLLOUT;
		}
	}
	else
	{
		rv = EPOLLET;
		
		if (req_mask & (Q_FAST_READ | Q_EDGE_READ))
		{
			rv |= EPOLLIN;
		}
		
		if (req_mask & (Q_FAST_WRITE | Q_EDGE_WRITE))
		{
			rv |= EPOLLOUT;
		}
	}
	
	return rv;
}

bool SocketPool::AddDescriptor(EventHandler* ehandler, int req_mask)
{
	int fd = ehandler->GetDescriptor();

	if (fd < 0)
	{
		return false;
	}

	if (!SocketPool::AttachFileDescRef(ehandler))
	{
		return false;
	}

	struct epoll_event ep_ev;
	memset(&ep_ev, 0, sizeof(ep_ev));
	ep_ev.events = SetMask(req_mask);
	ep_ev.data.ptr = static_cast<void*>(ehandler);
	int i = epoll_ctl(SocketHandler, EPOLL_CTL_ADD, fd, &ep_ev);

	if (i < 0)
	{
		return false;
	}

	ehandler->SetReqMask(req_mask);
	ResizeDouble(events);

	return true;
}

void SocketPool::OnMaskReq(EventHandler* ehandler, int old_mask, int new_mask)
{
	unsigned PreviousEvents = SetMask(old_mask);
	unsigned NewEvents = SetMask(new_mask);

	if (PreviousEvents != NewEvents)
	{
		struct epoll_event ep_ev;
		memset(&ep_ev, 0, sizeof(ep_ev));
		ep_ev.events = NewEvents;
		ep_ev.data.ptr = static_cast<void*>(ehandler);
		epoll_ctl(SocketHandler, EPOLL_CTL_MOD, ehandler->GetDescriptor(), &ep_ev);
	}
}

void SocketPool::DeleteDescriptor(EventHandler* ehandler)
{
	int fd = ehandler->GetDescriptor();

	if (fd < 0)
	{
		return;
	}

	struct epoll_event ev;
	epoll_ctl(SocketHandler, EPOLL_CTL_DEL, fd, &ev);
	SocketPool::DeleteFileDescRef(ehandler);
}

int SocketPool::Events()
{
	int i = epoll_wait(SocketHandler, &events[0], events.size(), 3);
        Kernel->Now();
	
	for (int j = 0; j < i; j++)
	{
		const epoll_event ep_ev = events[j];

		EventHandler* const ehandler = static_cast<EventHandler*>(ep_ev.data.ptr);
		const int fd = ehandler->GetDescriptor();
		
		if (fd < 0)
		{
			continue;
		}

		if (ep_ev.events & EPOLLHUP)
		{
			ehandler->OnManageError(0);
			continue;
		}

		if (ep_ev.events & EPOLLERR)
		{
			socklen_t codesize = sizeof(int);
			int errcode;
			
			if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &codesize) < 0)
			{
				errcode = errno;
			}
			
			ehandler->OnManageError(errcode);
			continue;
		}

		int mask = ehandler->GetEventMask();

		if (ep_ev.events & EPOLLIN)
		{
			mask &= ~Q_READ_WILL_BLOCK;
		}
		
		if (ep_ev.events & EPOLLOUT)
		{
			mask &= ~Q_WRITE_BLOCK;
			
			if (mask & Q_SINGLE_WRITE)
			{
				int nm = mask & ~Q_SINGLE_WRITE;
				OnMaskReq(ehandler, mask, nm);
				mask = nm;
			}
		}
		
		ehandler->SetReqMask(mask);
		
		if (ep_ev.events & EPOLLIN)
		{
			ehandler->OnPendingRead();
			
			if (ehandler != GetReference(fd))
			{
				continue;
			}
		}
		
		if (ep_ev.events & EPOLLOUT)
		{
			ehandler->OnPendingWrites();
		}
	}
	
	return i;
}
