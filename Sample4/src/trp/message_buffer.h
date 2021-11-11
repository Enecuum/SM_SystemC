#ifndef __MESS_BUFFER_H__
#define __MESS_BUFFER_H__

#include "inc.h"



using namespace std;



namespace P2P_MODEL
{
    typedef chord_message              buffer_element;
    typedef chord_message&             buffer_element_reference;
    typedef chord_message*             buffer_element_pointer;
    #define t_container                list 
    #define t_buffer_container(a)      t_container<a>
    typedef t_container<chord_message> buffer_container;

    enum buffer_type {
        BUFF_CONFIG = 0,
        BUFF_TIMER,
        BUFF_APPTXDATA,
        BUFF_RX_MESS,

        BUFF_TX_JOIN,
        BUFF_TX_NOTIFY,
        BUFF_TX_ACK,
        BUFF_TX_SUCCESSOR,
        BUFF_TX_FIND_SUCCESSOR,
        BUFF_TX_PREDECESSOR,
        BUFF_TX_FIND_PREDECESSOR,
        BUFF_TX_FWD_BROADCAST,
        BUFF_TX_FWD_MULTICAST,
        BUFF_TX_FWD_SINGLE,
        BUFF_TX_BROADCAST,
        BUFF_TX_MULTICAST,
        BUFF_TX_SINGLE,
        
        MAX_BUFF_TYPE,
        BUFF_UNKNOWN
    };

    template <class T>
    class message_buffer {
    private:
        list<T> messages;
        int maxDeep;
        int priority;
        bool immediate;
        int maxSize;
        int messCounter;
        buffer_type type;       
        sc_time lastCallTime;
        

    public:
        message_buffer() {
            clear();
        }

        ~message_buffer() {            
        }

        message_buffer(const message_buffer& src) {
            *this = src;
        }


        void clearMessages() {
            messages.clear();            
            messCounter = 0;
            lastCallTime = SC_ZERO_TIME;
        }

        void set(const buffer_type type, const int maxDeep, const int maxSize, const bool immediate, const int priority) {
            this->type = type;
            this->maxDeep = maxDeep;
            this->priority = priority;
            this->immediate = immediate;
            this->maxSize = maxSize;
        }

        bool push(const T& r) {
            if (messages.size() < maxSize) {
                messages.push_back(r);
                return true;
            }
            else {
                cout << "BUFFER OVERFLOW" << toStr() << std::endl;
                messages.erase(messages.begin());
                messages.push_back(r);
                return false;
            }            
        }

        uint size() const {
            return (uint) messages.size();
        }    
        

    public:
        uint buffType() const {
            return type;
        }

        T* mess(const size_t index) {
            auto it = messages.begin();
            if (index < messages.size()) {
                advance(it, index);
                return &(*it);
            }
            return nullptr;
        }

        T& firstMessByImmediate() {
            static T r;
            auto it = firstMessIteratorByImmediate();
            if (it == messages.end())
                return r;
            return *it;
        }

        T* firstMessPointerByImmediate() {
            auto it = firstMessIteratorByImmediate();
            if (it == messages.end())
                return nullptr;
            return &(*it);
        }

        void eraseMess(const typename t_buffer_container(T)::iterator& it) {
            messages.erase(it);
        }

        void eraseFirstMess() {
            if (messages.size() > 0)
                eraseMess(messages.begin());
        }

        void eraseMess(const uint messType, const uint retryMessType, const uint retryMessID) {
            bool exist;
            typename t_buffer_container(T)::iterator it = find1Mess(exist, messType, retryMessType, retryMessID);
            if (exist == true)
                eraseMess(it);
        }

        void eraseMess(const uint messType) {
            bool exist;
            typename t_buffer_container(T)::iterator it = find1Mess(exist, messType);
            if (exist == true)
                erase(it);
        }

        typename t_buffer_container(T)::iterator find1Mess(bool& exist, const uint messType, const uint retryMessType, const uint retryMessID) {
            vector<uint> retryMessTypes(1, retryMessType);
            return find1Mess(exist, messType, retryMessTypes, retryMessID);
        }

        typename t_buffer_container(T)::iterator find1Mess(bool& exist, uint messType) {
            vector<uint> retryMessTypes;
            return find1Mess(exist, messType, retryMessTypes, retryMessID);
        }

        typename t_buffer_container(T)::iterator find1Mess(bool& exist, const uint messType, const vector<uint>& retryMessTypes, const uint retryMessID) {
            for (auto it = messages.begin(); it != messages.end(); ++it) {
                chord_message& m = (*it);
                if (m.type == messType) {
                    if (retryMessTypes.size() == 0) {
                        exist = true;
                        return it;
                    }
                    else {
                        if (m.retryMess != nullptr) {
                            for (uint i = 0; i < retryMessTypes.size(); ++i) {
                                if ((m.retryMess->type == retryMessTypes[i]) && (m.retryMess->messageID == retryMessID)) {
                                    exist = true;
                                    return it;
                                }
                            }
                        }
                    }
                }
            }
            exist = false;
            return messages.end();
        }


        void clearMessCounter() {
            messCounter = 0;
        }


        bool operator < (const message_buffer& right) const {
            //priority "1" is the highest priority to service requests
            // ...
            //priority "5" is the  lowest priority to service requests
            //if (immediate != right.immediate) {
            //    if (!(right.immediate == true)) return true;
            //    else return false;
            //}
            //else
                return (priority < right.priority);
        }

        message_buffer& operator= (const message_buffer& src) {
            if (this == &src)
                return *this;
            this->type        = src.type;
            this->messages    = src.messages;
            this->maxDeep     = src.maxDeep;
            this->priority    = src.priority;
            this->immediate   = src.immediate;
            this->maxSize     = src.maxSize;
            this->messCounter = src.messCounter;
            return *this;
        }

        string toStr() const {
            string str;
            str.clear();
            string typeStr;
            switch (type) {
                case BUFF_CONFIG:                  typeStr = "BUFF_CONFIG"; break;
                case BUFF_TIMER:                   typeStr = "BUFF_TIMER"; break;
                case BUFF_APPTXDATA:               typeStr = "BUFF_APPTXDATA"; break;
                case BUFF_RX_MESS:                 typeStr = "BUFF_RX_MESS"; break;

                case BUFF_TX_JOIN:                 typeStr = "BUFF_TX_JOIN"; break;
                case BUFF_TX_NOTIFY:               typeStr = "BUFF_TX_NOTIFY"; break;
                case BUFF_TX_ACK:                  typeStr = "BUFF_TX_ACK"; break;
                case BUFF_TX_SUCCESSOR:            typeStr = "BUFF_TX_SUCCESSOR"; break;
                case BUFF_TX_FIND_SUCCESSOR:       typeStr = "BUFF_TX_FIND_SUCCESSOR"; break;
                case BUFF_TX_PREDECESSOR:          typeStr = "BUFF_TX_PREDECESSOR"; break;
                case BUFF_TX_FIND_PREDECESSOR:     typeStr = "BUFF_TX_FIND_PREDECESSOR"; break;
                case BUFF_TX_FWD_BROADCAST:        typeStr = "BUFF_TX_FWD_BROADCAST"; break;
                case BUFF_TX_FWD_MULTICAST:        typeStr = "BUFF_TX_FWD_MULTICAST"; break;
                case BUFF_TX_FWD_SINGLE:           typeStr = "BUFF_TX_FWD_SINGLE"; break;
                case BUFF_TX_BROADCAST:            typeStr = "BUFF_TX_BROADCAST"; break;
                case BUFF_TX_MULTICAST:            typeStr = "BUFF_TX_MULTICAST"; break;
                case BUFF_TX_SINGLE:               typeStr = "BUFF_TX_SINGLE"; break;
                default:                           typeStr = "BUFF_UNKNOWN"; break;
            }
            str = typeStr + "\timmediate\t" + string(immediate == true ? "true" : "false") + "\tpri\t" + to_string(priority);
            return str;
        }

    private:
        void clear() {
            messages.clear();
            type = BUFF_UNKNOWN;
            maxDeep = 0;
            priority = 0;
            immediate = false;
            maxSize = 0;
            messCounter = 0;
            lastCallTime = SC_ZERO_TIME;
        }

        typename t_buffer_container(T)::iterator firstMessIteratorByImmediate() {
            typename t_buffer_container(T)::iterator it = messages.end();

            if (messages.size() > 0)
            {
                if (sc_time_stamp() != lastCallTime)
                    messCounter = 0;

                if (immediate == true) {
                    it = messages.begin();
                    lastCallTime = sc_time_stamp();
                    //++messCounter;
                }
                else {
                    if ((messCounter < maxDeep) && (messCounter < maxSize)) {
                        it = messages.begin();
                        lastCallTime = sc_time_stamp();
                        ++messCounter;
                    }
                }
            }
            return it;
        }
    };

}
#endif