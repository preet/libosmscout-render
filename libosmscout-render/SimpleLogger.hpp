/*
    libosmscout-render

    Copyright (C) 2012, Preet Desai

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef OSMSCOUTRENDER_SIMPLELOGGER_HPP
#define OSMSCOUTRENDER_SIMPLELOGGER_HPP

#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>

namespace osmscout
{
    class SimpleLogger
    {
    public:
        SimpleLogger(std::vector<std::string> *myStreamHistory)
        {
            m_streamhistory = myStreamHistory;
            m_stringstream.precision(8);
        }

        ~SimpleLogger()
        {
            // add the latest log message to stream history
            std::string myMessage = m_stringstream.str();
            m_streamhistory->push_back(myMessage);

            //temp
//            std::cout << myMessage << std::endl;

            // keep the log size of previous errors to a
            // reasonable number (500 lines seems reasonable)
            if(m_streamhistory->size() > 500)
            {
                std::vector<std::string>::iterator it =
                        m_streamhistory->begin();

                m_streamhistory->erase(it);
            }
        }

        std::ostream & GetStream()
        {   return m_stringstream;   }

    private:
        std::stringstream m_stringstream;
        std::vector<std::string> *m_streamhistory;
    };
}

#define OSRDEBUG (SimpleLogger(&m_listMessages).GetStream())

#endif
