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

class ExportAPI UIDHandler : public safecast<UIDHandler>
{
  private:
	
	/* Holds current uid. */
	
	std::string LastUID;

	/* Finds next uid. */
	
	void FindNext(unsigned int pos);

 public:
	
	static const unsigned int UUID_LIMIT = 9;

	/* Initializes UID handler. */
	
	void Initialize();

	/* Returns UID created by FindNext() */
	
	std::string GetUID();
	
	/* Creats UUID string. */

	static std::string Create();
};