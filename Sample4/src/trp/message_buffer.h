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
    typedef t_container<buffer_element> buffer_container;

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
        BUFF_MEMORY,        
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

        bool push(const T& r, const bool toBack = true) {
            bool pushWithoutErase = true;
            if (messages.size() >= maxSize) {
                pushWithoutErase = false;
                messages.erase(messages.begin());
            }
                        
            if (toBack)
                messages.push_back(r);
            else
                messages.push_front(r);
            return pushWithoutErase;
        }

        uint size() const {
            return (uint) messages.size();
        }    
        

    public:
        uint buffType() const {
            return type;
        }
        
        T mess(const size_t index) {
            auto it = messIterator(index) 
            if (it != messages.end())
                return T();
            return *it;
        }                
        
        T* messPointer(const size_t index) {
            auto it = messIterator(index);                 
            if (it != messages.end())                
                return &(*it);
            return nullptr;
        }

        typename t_buffer_container(T)::iterator messIterator(const size_t index) {
            auto it = messages.begin();
            if (index < messages.size()) {
                advance(it, index);
                return it;
            }
            return messages.end();
        }

        T firstMessByImmediate() {
            auto it = firstMessIteratorByImmediate();
            if (it != messages.end())
                return *it;
            return T();
        }

        T* firstMessPointerByImmediate() {
            auto it = firstMessIteratorByImmediate();
            if (it != messages.end())
                return &(*it);
            return nullptr;
        }

        bool eraseFirstMess() {
            if (messages.size() > 0) {
                eraseMess(messages.begin());
                return true;
            }
            return false;
        }

        bool eraseMess(const typename t_buffer_container(T)::iterator& it) {
            if (it != messages.end()) {
                messages.erase(it);
                return true;
            }
            return false;
        }

        bool eraseMess(const uint messType) {
            bool exist;
            vector<uint> retryMessTypes;
            typename t_buffer_container(T)::iterator it = find1Mess(exist, messType, retryMessTypes, 0);
            if (exist == true) 
                return eraseMess(it);
            return false;
        }


        bool eraseMess(const uint messType, const uint retryMessType, const uint retryMessID) {
            bool exist;
            vector<uint> retryMessTypes(1, retryMessType);
            typename t_buffer_container(T)::iterator it = find1Mess(exist, messType, retryMessTypes, retryMessID);
            if (exist == true)
                return eraseMess(it);
            return false;
        }

        bool eraseAllMess(const vector<uint>& messTypes, const vector<uint>& retryMessTypes, const vector<uint>& retryMessIDs) {
            bool exist;            
            vector<typename t_buffer_container(T)::iterator> all = findAllMess(exist, messTypes, retryMessTypes, retryMessIDs);
            if (exist == true) {
                for (uint i = 0; i < all.size(); ++i) 
                    eraseMess(all[i]);                
            }
            return exist;
        }


        typename t_buffer_container(T)::iterator find1Mess(bool& exist, const uint messType) {
            vector<uint> retryMessTypes;
            vector<uint> messTypes(1, messType);    
            vector<typename t_buffer_container(T)::iterator> all = findAllMess(exist, messTypes, vector<uint>(),0);            
            if (all.size() > 0)
                return *(all.begin());
            return (typename t_buffer_container(T)::iterator());
        }

        typename t_buffer_container(T)::iterator find1Mess(bool& exist, const uint messType, const vector<uint>& retryMessTypes, const uint retryMessID) {
            vector<uint> messTypes(1, messType);
            vector<uint> retryMessIDs(1, retryMessID);
            vector<typename t_buffer_container(T)::iterator> all = findAllMess(exist, messTypes, retryMessTypes, retryMessIDs);
            if (all.size() > 0)
                return *(all.begin());

            return (typename t_buffer_container(T)::iterator());
        }



        vector<typename t_buffer_container(T)::iterator> findAllMess(bool& exist, const vector<uint>& messTypes, const vector<uint>& retryMessTypes, const vector<uint>& retryMessIDs) {
            exist = false;
            vector<t_buffer_container(T)::iterator> resIt;
            for (auto it = messages.begin(); it != messages.end(); ++it) {
                chord_message& currMess = (*it);
                for (uint i1 = 0; i1 < messTypes.size(); ++i1) {
                    if (currMess.type == messTypes[i1]) {
                        if (retryMessTypes.size() == 0) {
                            exist = true;
                            resIt.push_back(it);                            
                        }
                        else {
                            for (uint i2 = 0; i2 < retryMessTypes.size(); ++i2) {
                                if (currMess.retryMess.type == retryMessTypes[i2]) {
                                    if (retryMessIDs.size() == 0) {
                                        exist = true;
                                        resIt.push_back(it);
                                    }
                                    else {
                                        for (uint i3 = 0; i3 < retryMessIDs.size(); ++i3) {
                                            if (currMess.retryMess.messageID == retryMessIDs[i3]) {
                                                exist = true;
                                                resIt.push_back(it);                                        
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return resIt;
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
                case BUFF_MEMORY:                  typeStr = "BUFF_MEMORY"; break;
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