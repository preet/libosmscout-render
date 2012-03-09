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
