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

class ExportAPI ListHandler
{
  private:
    
        ListMap mhandler;

        HANDLER_MSG LastMsg;
     
  public:
  
        /* Constructor */
         
        ListHandler();
         
        /* Static constructor. */
         
        static std::shared_ptr<ListHandler> Create(const std::string& load);
     
        void Dump();

        /* 
         * Obtains the average size of a given list.
         * 
         * @return:
 	 *
         *         · double	: Avg value.
         */    
         
        double GetSMA();
        
        /* 
         * Checks whether a entire list if numeric.
         * 
         * @return:
 	 *
         *         · True	: All elements are numeric.
         */    

        bool IsNumeric();
         
        /* Removes all elements. */
         
        void Erase();

        /* Removes first element in list. */    
         
        void PopFront();

        /* 
         * Determines whether a given key exists.
         * 
         * @parameters:
	 *
	 *         · string	: Key to verify.
	 * 
         * @return:
 	 *
         *         · True	: Key is present.
         */    
         
        bool Exist(const std::string& key);

        /* 
         * Counts repeats in a given list.
         * 
         * @parameters:
	 *
	 *         · string	: Item to lookup.
	 * 
         * @return:
 	 *
         *         · uint	: Times a given item is repeated.
         */    
         
        unsigned int Repeats(const std::string& word);

        /* 
         * Returns a string provided a given pos.
         * 
         * @parameters:
	 *
	 *         · uint	: Position to find.
	 * 
         * @return:
 	 *
         *         · string	: Item found in given position.
         */
                     
        std::string Index(unsigned int pos);
        
        /*  Removes last element in mhandler. */

        void PopBack();

        /* 
         * Returns front element from list.
	 * 
         * @return:
 	 *
         *         · string	: Front element as string.
         */    
         
        std::string Front();
         
        /* 
         * Removes all matching keys for given element.
         * 
         * @parameters:
	 *
	 *         · string	: Key to remove.
         */             
         
        void PopAll(const std::string& key);
     
         /* Returns mhandler as string. */
         
        std::string as_string();

        /* 
         * Adds a new item to the end of the list (push_back).
         * 
         * @parameters:
	 *
	 *         · string	: Key to add.
         */         
         
        void Add(const std::string& key);

        /* 
         * Removes a key from a list.
         * 
         * @parameters:
	 *
	 *         · string	: Key to remove.
         */         
         
        void Remove(const std::string& key);

        /* 
         * Counts items in the mhandler list.
         * 
         * @return:
 	 *
         *         · uint	: List size.
         */    
              
        unsigned int Count()
        {
                this->LastMsg = HANDLER_MSG_OK;
                return this->mhandler.size();
        }
        
        /* Reverses a list. */
        
        void Reverse()
        {
                this->LastMsg = HANDLER_MSG_OK;                
                this->mhandler.reverse();
        }
         
        /* 
         * Resizes a given list.
         * 
         * @parameters:
	 *
	 *         · uint	: Shrink to size.
	 * 
         */    
         
        void Resize(unsigned int size)
        {
               this->LastMsg = HANDLER_MSG_OK;
               this->mhandler.resize(size);
        }

         /* Sorts list. */
         
        void Sort()
        {
               this->LastMsg = HANDLER_MSG_OK;               
               this->mhandler.sort();
        }

        /* 
         * Finds an item in list and returns a vector.
         * 
         * @parameters:
	 *
	 *         · string	: Wildcard to lookup.
	 * 
         * @return:
 	 *
         *         · vector	: String vector containing all matched keys.
         */              
         
        std::vector<std::string> Find(const std::string& key);

        /* 
         * Returns last item in listen, and then removes it.
	 * 
         * @return:
 	 *
         *         · string	: Last item in provided item.
         */    
         
        std::string RPOP();

        /* 
         * Returns last element in list.
	 * 
         * @return:
 	 *
         *         · string	: Last element in list.
         */    
         
        std::string Back();

        /* 
         * Obtains front (first) element in list.
         * 
         * @return:
 	 *
         *         · string	: Front element.
         */    
         
        std::string FPOP();
         
        /* 
         * Returns lowest element in list.
         * Keep in mind that this list must be numeric in order
         * to be ran.
	 * 
         * @return:
 	 *
         *         · double	: Lowest element.
         */             
         
        double GetLow();
        
        /* 
         * Obtains highest number in list.
	 * 
         * @return:
 	 *
         *         · double: Highest number.
         */            
         
        double GetHigh();
        
         
        /* 
         * Returns a copy of this->mhandler.
         * 
         * @return:
 	 *
         *         · ListMap	: Entire list.
         */         
         
        ListMap& GetList()
        {
                return this->mhandler;
        }

        /* 
         * Fetch last returned msg from a list operation.
         *
         * @return:
 	 *
         *         · HANDLER_MSG	: Last msg.
         */    
         
        HANDLER_MSG GetLast()
        {
             return this->LastMsg;
        }
        
};

