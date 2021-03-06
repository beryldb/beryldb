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

class Serverstats : public safecast<Serverstats>
{
   public:
			
      /* Accepted connections */
      
      unsigned long Accept;
		
      unsigned long Refused;
		
      unsigned long Unknown;
				
      unsigned long Collisions;

      unsigned long Connects;
      
      unsigned long Cached;
      
      timeval LastCPU;

      timespec LastSampled;

      Serverstats() : Accept(0), Refused(0), Unknown(0), Collisions(0), Connects(0), Cached(0)
      {
		
      }
};