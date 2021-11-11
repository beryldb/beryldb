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
#include "engine.h"

bool ModuleHandler::Load(const std::string& name, bool nextloop)
{
	if (name.find('/') != std::string::npos)
	{
		this->last_known_error = "Bad path format: " + name;
		return false;
	}

	const std::string filename = FullModName(name);
	const std::string ModuleFile = Kernel->Config->Paths->SetWDModule(filename);

	if (!FileSystem::Exists(ModuleFile))
	{
		last_known_error = NOT_FOUND;
                bprint(ERROR, "%s", last_known_error.c_str());
		return false;
	}

	if (Modules.find(filename) != Modules.end())
	{
		last_known_error = "MODULE_LOADED";
	        bprint(ERROR, "%s", last_known_error.c_str());
		return false;
	}

	Module* loading = NULL;
	DLLHandler* handler = new DLLHandler(ModuleFile.c_str());
	ServiceList NewServices;
	
	if (!nextloop)
	{	
		this->new_services = &NewServices;
	}

	try
	{
		loading = handler->StartMod();
		this->new_services = NULL;

		if (loading)
		{
			loading->SourceFile = filename;
			loading->DLL = handler;
			Modules[filename] = loading;
			const char* version = handler->GetDescription();

			if (!version)
			{
				version = "unknown";
			}

			if (!nextloop)
			{
				config_status confstatus;
				AttachAll(loading);
				AttachServices(NewServices);
				
				loading->Initialize();
				loading->ConfigReading(confstatus);
				Version v = loading->GetDescription();
			}
		}
		else
		{
			last_known_error = "Unable to load " + filename + ": " + handler->LastError();
	                bprint(ERROR, "%s", last_known_error.c_str());
			delete handler;
			return false;
		}
	}
	catch (KernelException& modexcept)
	{
		this->new_services = NULL;

		if (loading)
		{
			SmartUnload(loading);
			Kernel->Reducer->Add(handler);
		}
		else
		{
			delete handler;
		}
		
		last_known_error = "Unable to load " + filename + ": " + modexcept.get_reason();
                bprint(ERROR, "%s", last_known_error.c_str());
		return false;
	}

	if (nextloop)
	{
		return true;
	}

	NOTIFY_MODS(OnLoadModule, (loading));
	PrioritizeAttachs();
	return true;
}

void ModuleHandler::LoadCore(std::map<std::string, ServiceList>& ServiceMap)
{
	unsigned int counter = 0;

	std::vector<std::string> files;
	
	if (!FileSystem::AsFileList(Kernel->Config->Paths->Module, files, "core_*.so"))
	{
		bprint(ERROR, "Failed to retrieve core modules list.");
		Kernel->Exit(EXIT_CODE_MODULE);
	}

	for (std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); ++i)
	{
		counter++;
		const std::string& name = *i;
		this->new_services = &ServiceMap[name];

		if (!Load(name, true))
		{
			bprint(ERROR, "%s", this->LastError().c_str());
			Kernel->Exit(EXIT_CODE_MODULE);
		}
		else 
		{
			//bprint(DONE, "Core module loaded: %s", name.c_str());
		}
	}
	
        iprint(counter, "Core modules loaded."); 
}
