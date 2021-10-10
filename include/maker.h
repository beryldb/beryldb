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

#include "beryl.h"
#include "engine.h"

/* 
 * Checks whether a given number is valid and, positive.
 * 
 * @parameters:
 *
 *         · string   : String to check.
 *
 * @return:
 *
 *       - bool
 *
 *         · true     : Valid number.
 *         · false    : Invalid number.
 */

inline bool CheckValidPos(User* user, const std::string& number)
{
       if (!is_number(number))
       {
                 user->SendProtocol(ERR_INPUT, MUST_BE_NUMERIC);
                 return false;
       }

       if (!is_positive_number(number))
       {
                user->SendProtocol(ERR_INPUT, MUST_BE_POSIT);
                return false;
       }
       
       return true;
}

/* 
 * Checks whether provided value is numeric.
 * 
 * @parameters:
 *
 *         · string	: Number to check.
 * 
 * @return:
 *
 *         · True	: Is numeric.
 */    
 
inline bool CheckValid(User* user, const std::string& number)
{
       if (!is_number(number))
       {
                 user->SendProtocol(ERR_INPUT, MUST_BE_NUMERIC);
                 return false;
       }
       
       return true;
}

/* 
 * Checks the format of a string (ie "string query").
 * 
 * @parameters:
 *
 *         · string	: String to check.
 *         · bool	: Whether this function notifies to the requesting user.
 *
 * @return:
 *
 *       - bool:
 *
 *          · true	: Valid query.
 *          · false	: Invalid query.
 */

inline bool CheckFormat(User* user, const std::string& value, bool notify = true)
{
        if (value.size() == 0)
        {
            if (notify)
            {
                 user->SendProtocol(ERR_INPUT, INVALID_TYPE);
            }

            return false;
        }

        if (value.size() == 1)
        {
             return true;
        }

        if (is_number(value, true))
        {
             return true;
        }

        if (is_correct(value))
        {
             return true;
        }

        if (notify)
        {
                user->SendProtocol(ERR_INPUT, INVALID_VALUE);
        }

        return false;
}

/* 
 * Checks whether a given key is valid.
 * 
 * @parameters:
 *
 *         · string	: Key to check.
 *
 * @return:
 *
 *         · true	: Valid key.
 */

inline bool CheckKey(User* user, const std::string& value, bool notify = true)
{
        if (!value.size())
        {
              if (notify)
              {
                  user->SendProtocol(ERR_INPUT, INVALID_KEY);
              }

              return false;
        }
        
        if (Kernel->Engine->ValidKey(value))
        {
              return true;
        }
        
        if (notify)
        {
              user->SendProtocol(ERR_INPUT, INVALID_KEY);
        }
        
        return false;
}

/* 
 * Returns limiting data for a query.
 * Keep in mind that this class should be only
 * handled by function GetLimits.
 */

class ExportAPI Limiter : public safecast<Limiter>
{
   friend Limiter GetLimits(User* user, unsigned int max, const CommandModel::Params& parameters);
    
   private:
   
      bool error;
      signed int limit;
      signed int offset;
  
  public:
  
      Limiter() : error(false), limit(0), offset(0)
      {
       
      }
      
      bool GetError()
      {
             return this->error;
      }
       
      signed int GetLimit()
      {
             return this->limit;
      }
      
      signed int GetOffset()
      {
             return this->offset;      
      }
};

/* 
 * Returns a vector containing limits.
 * 
 * @parameters:
 *
 *         · User       : Requesting user.
 *         · uint       : Max parameters.
 *         · Params     : Parameters as given by the original command.,
 *
 * @return:
 *
 *         · Limiter    : Returns { 0 } if invalid.
 */

inline Limiter GetLimits(User* user, unsigned int max, const CommandModel::Params& parameters)
{
       Limiter limiter;

       if (parameters.size() == (max - 1))
       {
             if (!is_zero_or_great_or_mone(parameters[(max - 2)]))
             {
                  user->SendProtocol(ERR_INPUT, MUST_BE_GREAT_ZERO);
                  limiter.error = true;
                  return limiter;
             }
       
             limiter.limit  = convto_num<signed int>(parameters[(max - 2)]); 
             limiter.offset = 0;
       }
       else if (parameters.size() == max)
       {
             if (!is_zero_or_great_or_mone(parameters[(max - 1)]) || !is_zero_or_great(parameters[(max - 2)]))
             {
                   user->SendProtocol(ERR_INPUT, MUST_BE_GREAT_ZERO);
                   limiter.error = true;
                   return limiter;
             }
       
             limiter.limit  = convto_num<signed int>(parameters[(max - 1)]); 
             limiter.offset = convto_num<signed int>(parameters[(max - 2)]);
       }
       else
       {
             limiter.limit  = -1;
             limiter.offset = 0;
       }
       
       return limiter;
}
