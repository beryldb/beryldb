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

#include "beryl.h"
#include "core_geo.h"

class CoreModuleGeo : public Module
{
    private:
    
        CommandGeoAdd cmdgeoadd;
        CommandGeoGet cmdgeoget;
        CommandGeoDel cmdgeodel;
        CommandGFind cmdgfind;

    public:     
        
        CoreModuleGeo() : cmdgeoadd(this), cmdgeoget(this), cmdgeodel(this),
                          cmdgfind(this)
        {
        
        }
        
        Version GetDescription() 
        {
                return Version("Provides geo-related functions.", VF_BERYLDB|VF_CORE);
        }
};

MODULE_LOAD(CoreModuleGeo)
        