#include <algorithm>
#include <iostream>
#include <map>
#include <queue>
#include <utility>
#include <climits>
#include <functional>
#include <iomanip>
#include <stack>
#include <vector>
#include <sstream>

using namespace std;

#define SET(func_name,type,var_name,_var_name) void func_name(type _var_name) { var_name = _var_name ;} 
#define GET(func_name,type,var_name) type func_name() const { return var_name ;}

class header;
class payload;
class packet;
class node;
class event;
class link; // new

// for simplicity, we use a const int to simulate the delay
// if you want to simulate the more details, you should revise it to be a class
const unsigned int ONE_HOP_DELAY = 10;
const unsigned int BROCAST_ID = UINT_MAX;

map <int, int> ans_old_table[3000], ans_new_table[3000], truth_table[3000];

// BROCAST_ID means that all neighbors are receivers; UINT_MAX is the maximum value of unsigned int
class test{
    public:
    unsigned int input_time;
    unsigned int input_SrcID;
    unsigned int input_DstID;
}input_information[3000];

class dat{
    public:
    vector<pair<int, int> > arr;
}link_old[100000],link_new[100000];

class header {
    public:
        virtual ~header() {}

        SET(setSrcID, unsigned int , srcID, _srcID);
        SET(setDstID, unsigned int , dstID, _dstID);
        SET(setPreID, unsigned int , preID, _preID);
        SET(setNexID, unsigned int , nexID, _nexID);
        GET(getSrcID, unsigned int , srcID);
        GET(getDstID, unsigned int , dstID);
        GET(getPreID, unsigned int , preID);
        GET(getNexID, unsigned int , nexID);
        
        virtual string type() = 0;
        
        // factory concept: generate a header
        class header_generator {
                // lock the copy constructor
                header_generator(header_generator &){}
                // store all possible types of header
                static map<string,header_generator*> prototypes;
            protected:
                // allow derived class to use it
                header_generator() {}
                // after you create a new header type, please register the factory of this header type by this function
                void register_header_type(header_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your header
                virtual header* generate() = 0 ;
            public:
                // you have to implement your own type() to return your header type
        	    virtual string type() = 0 ;
        	    // this function is used to generate any type of header derived
        	    static header * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such header type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered header types: " << endl;
            	    for (map<string,header::header_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~header_generator(){};
        };
        
    protected:
        header():srcID(BROCAST_ID),dstID(BROCAST_ID),preID(BROCAST_ID),nexID(BROCAST_ID){} // this constructor cannot be directly called by users

    private:
        unsigned int srcID;
        unsigned int dstID;
        unsigned int preID;
        unsigned int nexID;
        header(header&){} // this constructor cannot be directly called by users
};
map<string,header::header_generator*> header::header_generator::prototypes;

class SDN_data_header : public header{
        SDN_data_header(SDN_data_header&){} // cannot be called by users
        
    protected:
        SDN_data_header(){} // this constructor cannot be directly called by users

    public:
        ~SDN_data_header(){}
        string type() { return "SDN_data_header"; }

        class SDN_data_header_generator;
        friend class SDN_data_header_generator;
        // SDN_data_header_generator is derived from header_generator to generate a header
        class SDN_data_header_generator : public header_generator{
                static SDN_data_header_generator sample;
                // this constructor is only for sample to register this header type
                SDN_data_header_generator() { /*cout << "SDN_data_header registered" << endl;*/ register_header_type(&sample); }
            protected:
                virtual header * generate(){
                    // cout << "SDN_data_header generated" << endl;
                    return new SDN_data_header; 
                }
            public:
                virtual string type() { return "SDN_data_header";}
                ~SDN_data_header_generator(){}
        
        };
};
SDN_data_header::SDN_data_header_generator SDN_data_header::SDN_data_header_generator::sample;

class SDN_ctrl_header : public header{
        SDN_ctrl_header(SDN_ctrl_header&){} // cannot be called by users
        
    protected:
        SDN_ctrl_header(){} // this constructor cannot be directly called by users

    public:
        ~SDN_ctrl_header(){}
        string type() { return "SDN_ctrl_header"; }

        class SDN_ctrl_header_generator;
        friend class SDN_ctrl_header_generator;
        // SDN_ctrl_header_generator is derived from header_generator to generate a header
        class SDN_ctrl_header_generator : public header_generator{
                static SDN_ctrl_header_generator sample;
                // this constructor is only for sample to register this header type
                SDN_ctrl_header_generator() { /*cout << "SDN_ctrl_header registered" << endl;*/ register_header_type(&sample); }
            protected:
                virtual header * generate(){
                    // cout << "SDN_ctrl_header generated" << endl;
                    return new SDN_ctrl_header; 
                }
            public:
                virtual string type() { return "SDN_ctrl_header";}
                ~SDN_ctrl_header_generator(){}
        
        };
};
SDN_ctrl_header::SDN_ctrl_header_generator SDN_ctrl_header::SDN_ctrl_header_generator::sample;


class payload {
        payload(payload&){} // this constructor cannot be directly called by users
    protected:
        payload(){}
    public:
        virtual ~payload(){}
        virtual string type() = 0;
        
        class payload_generator {
                // lock the copy constructor
                payload_generator(payload_generator &){}
                // store all possible types of header
                static map<string,payload_generator*> prototypes;
            protected:
                // allow derived class to use it
                payload_generator() {}
                // after you create a new payload type, please register the factory of this payload type by this function
                void register_payload_type(payload_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your payload
                virtual payload* generate() = 0;
            public:
                // you have to implement your own type() to return your header type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of header derived
        	    static payload * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such payload type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered payload types: " << endl;
            	    for (map<string,payload::payload_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~payload_generator(){};
        };
};
map<string,payload::payload_generator*> payload::payload_generator::prototypes;


class SDN_data_payload : public payload {
        SDN_data_payload(SDN_data_payload&){}
        
        string msg;
        
    protected:
        SDN_data_payload(){} // this constructor cannot be directly called by users
    public:
        ~SDN_data_payload(){}
        
        SET(setMsg,string,msg,_msg);
        GET(getMsg,string,msg);
        
        string type() { return "SDN_data_payload"; }
        
        class SDN_data_payload_generator;
        friend class SDN_data_payload_generator;
        // SDN_data_payload is derived from payload_generator to generate a payload
        class SDN_data_payload_generator : public payload_generator{
                static SDN_data_payload_generator sample;
                // this constructor is only for sample to register this payload type
                SDN_data_payload_generator() { /*cout << "SDN_data_payload registered" << endl;*/ register_payload_type(&sample); }
            protected:
                virtual payload * generate(){ 
                    // cout << "SDN_data_payload generated" << endl;
                    return new SDN_data_payload; 
                }
            public:
                virtual string type() { return "SDN_data_payload";}
                ~SDN_data_payload_generator(){}
        };
};
SDN_data_payload::SDN_data_payload_generator SDN_data_payload::SDN_data_payload_generator::sample;

class SDN_ctrl_payload : public payload {
        SDN_ctrl_payload(SDN_ctrl_payload&){}
        
        string msg;
        
    protected:
        SDN_ctrl_payload(){} // this constructor cannot be directly called by users
    public:
        ~SDN_ctrl_payload(){}
        
        SET(setMsg,string,msg,_msg);
        GET(getMsg,string,msg);
        
        string type() { return "SDN_ctrl_payload"; }
        
        class SDN_ctrl_payload_generator;
        friend class SDN_ctrl_payload_generator;
        // SDN_data_payload is derived from payload_generator to generate a payload
        class SDN_ctrl_payload_generator : public payload_generator{
                static SDN_ctrl_payload_generator sample;
                // this constructor is only for sample to register this payload type
                SDN_ctrl_payload_generator() { /*cout << "SDN_ctrl_payload registered" << endl;*/ register_payload_type(&sample); }
            protected:
                virtual payload * generate(){ 
                    // cout << "SDN_ctrl_payload generated" << endl;
                    return new SDN_ctrl_payload; 
                }
            public:
                virtual string type() { return "SDN_ctrl_payload";}
                ~SDN_ctrl_payload_generator(){}
        };
};
SDN_ctrl_payload::SDN_ctrl_payload_generator SDN_ctrl_payload::SDN_ctrl_payload_generator::sample;


class packet{
        // a packet usually contains a header and a payload
        header *hdr;
        payload *pld;
        unsigned int p_id;
        static unsigned int last_packet_id ; 
        
        packet(packet &) {}
        static int live_packet_num ;
    protected:
        // these constructors cannot be directly called by users
        packet(): hdr(nullptr), pld(nullptr) { p_id=last_packet_id++; live_packet_num ++; }
        packet(string _hdr, string _pld, bool rep = false, unsigned int rep_id = 0) {
            if (! rep ) // a duplicated packet does not have a new packet id
                p_id = last_packet_id ++;
            else
                p_id = rep_id;
            hdr = header::header_generator::generate(_hdr); 
            pld = payload::payload_generator::generate(_pld); 
            live_packet_num ++;
        }
    public:
        virtual ~packet(){ 
            // cout << "packet destructor begin" << endl;
            if (hdr != nullptr) 
                delete hdr; 
            if (pld != nullptr) 
                delete pld; 
            live_packet_num --;
            // cout << "packet destructor end" << endl;
        }
        
        SET(setHeader,header*,hdr,_hdr);
        GET(getHeader,header*,hdr);
        SET(setPayload,payload*,pld,_pld);
        GET(getPayload,payload*,pld);
        GET(getPacketID,unsigned int,p_id);
        
        static void discard ( packet* &p ) {
            // cout << "checking" << endl;
            if (p != nullptr) {
                // cout << "discarding" << endl;
                // cout << p->type() << endl;
                delete p;
                // cout << "discarded" << endl;
            }
            p = nullptr;
            // cout << "checked" << endl;
        }
        virtual string type () = 0;
        
        static int getLivePacketNum () { return live_packet_num; }
        
        class packet_generator;
        friend class packet_generator;
        class packet_generator {
                // lock the copy constructor
                packet_generator(packet_generator &){}
                // store all possible types of packet
                static map<string,packet_generator*> prototypes;
            protected:
                // allow derived class to use it
                packet_generator() {}
                // after you create a new packet type, please register the factory of this payload type by this function
                void register_packet_type(packet_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your payload
                virtual packet* generate ( packet *p = nullptr) = 0;
            public:
                // you have to implement your own type() to return your packet type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of packet derived
        	    static packet * generate (string type) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            			return prototypes[type]->generate(); // generate it!!
            		}
            		std::cerr << "no such packet type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static packet * replicate (packet *p) {
            	    if(prototypes.find(p->type()) != prototypes.end()){ // if this type derived exists 
            			return prototypes[p->type()]->generate(p); // generate it!!
            		}
            		std::cerr << "no such packet type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered packet types: " << endl;
            	    for (map<string,packet::packet_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~packet_generator(){};
        };
};
map<string,packet::packet_generator*> packet::packet_generator::prototypes;
unsigned int packet::last_packet_id = 0 ;
int packet::live_packet_num = 0;


// this packet is used to tell the destination the msg
class SDN_data_packet: public packet {
        SDN_data_packet(SDN_data_packet &) {}
        
    protected:
        SDN_data_packet(){} // this constructor cannot be directly called by users
        SDN_data_packet(packet*p): packet(p->getHeader()->type(), p->getPayload()->type(), true, p->getPacketID()) {
            *(dynamic_cast<SDN_data_header*>(this->getHeader())) = *(dynamic_cast<SDN_data_header*> (p->getHeader()));
            *(dynamic_cast<SDN_data_payload*>(this->getPayload())) = *(dynamic_cast<SDN_data_payload*> (p->getPayload()));
            //DFS_path = (dynamic_cast<SDN_data_header*>(p))->DFS_path;
            //isVisited = (dynamic_cast<SDN_data_header*>(p))->isVisited;
        } // for duplicate
        SDN_data_packet(string _h, string _p): packet(_h,_p) {}
        
    public:
        virtual ~SDN_data_packet(){}
        string type() { return "SDN_data_packet"; }
        
        class SDN_data_packet_generator;
        friend class SDN_data_packet_generator;
        // SDN_data_packet is derived from packet_generator to generate a pub packet
        class SDN_data_packet_generator : public packet_generator{
                static SDN_data_packet_generator sample;
                // this constructor is only for sample to register this packet type
                SDN_data_packet_generator() { /*cout << "SDN_data_packet registered" << endl;*/ register_packet_type(&sample); }
            protected:
                virtual packet *generate (packet *p = nullptr){
                    // cout << "SDN_data_packet generated" << endl;
                    if ( nullptr == p )
                        return new SDN_data_packet("SDN_data_header","SDN_data_payload"); 
                    else
                        return new SDN_data_packet(p); // duplicate
                }
            public:
                virtual string type() { return "SDN_data_packet";}
                ~SDN_data_packet_generator(){}
        };
};
SDN_data_packet::SDN_data_packet_generator SDN_data_packet::SDN_data_packet_generator::sample;

class SDN_ctrl_packet: public packet {
        SDN_ctrl_packet(SDN_ctrl_packet &) {}
        
    protected:
        SDN_ctrl_packet(){} // this constructor cannot be directly called by users
        SDN_ctrl_packet(packet*p): packet(p->getHeader()->type(), p->getPayload()->type(), true, p->getPacketID()) {
            *(dynamic_cast<SDN_data_header*>(this->getHeader())) = *(dynamic_cast<SDN_data_header*> (p->getHeader()));
            *(dynamic_cast<SDN_data_payload*>(this->getPayload())) = *(dynamic_cast<SDN_data_payload*> (p->getPayload()));
            //DFS_path = (dynamic_cast<SDN_ctrl_header*>(p))->DFS_path;
            //isVisited = (dynamic_cast<SDN_ctrl_header*>(p))->isVisited;
        } // for duplicate
        SDN_ctrl_packet(string _h, string _p): packet(_h,_p) {}
        
    public:
        virtual ~SDN_ctrl_packet(){}
        string type() { return "SDN_ctrl_packet"; }
        
        class SDN_ctrl_packet_generator;
        friend class SDN_ctrl_packet_generator;
        // SDN_ctrl_packet is derived from packet_generator to generate a pub packet
        class SDN_ctrl_packet_generator : public packet_generator{
                static SDN_ctrl_packet_generator sample;
                // this constructor is only for sample to register this packet type
                SDN_ctrl_packet_generator() { /*cout << "SDN_ctrl_packet registered" << endl;*/ register_packet_type(&sample); }
            protected:
                virtual packet *generate (packet *p = nullptr){
                    // cout << "SDN_ctrl_packet generated" << endl;
                    if ( nullptr == p )
                        return new SDN_ctrl_packet("SDN_ctrl_header","SDN_ctrl_payload"); 
                    else
                        return new SDN_ctrl_packet(p); // duplicate
                }
            public:
                virtual string type() { return "SDN_ctrl_packet";}
                ~SDN_ctrl_packet_generator(){}
        };
};
SDN_ctrl_packet::SDN_ctrl_packet_generator SDN_ctrl_packet::SDN_ctrl_packet_generator::sample;

class node {
        // all nodes created in the program
        static map<unsigned int, node*> id_node_table;
        
        unsigned int id;
        map<unsigned int,bool> phy_neighbors;
        
    protected:
        node(node&){} // this constructor should not be used
        node(){} // this constructor should not be used
        node(unsigned int _id): id(_id) { id_node_table[_id] = this; }
    public:
        virtual ~node() { // erase the node
            id_node_table.erase (id) ;
        }
        virtual string type() = 0; // please define it in your derived node class
        
        void add_phy_neighbor (unsigned int _id, string link_type = "simple_link"); // we only add a directed link from id to _id
        void del_phy_neighbor (unsigned int _id); // we only delete a directed link from id to _id
        
        // you can use the function to get the node's neigbhors at this time
        // but in the project 3, you are not allowed to use this function 
        const map<unsigned int,bool> & getPhyNeighbors () { 
            return phy_neighbors;
        }
        
        
        void recv (packet *p) {
            packet *tp = p;
            recv_handler(tp); 
            packet::discard(p); 
        } // the packet will be directly deleted after the handler
        void send (packet *p);
        
        // receive the packet and do something; this is a pure virtual function
        virtual void recv_handler(packet *p) = 0;
        void send_handler(packet *P);
        
        static node * id_to_node (unsigned int _id) { return ((id_node_table.find(_id)!=id_node_table.end()) ? id_node_table[_id]: nullptr) ; }
        GET(getNodeID,unsigned int,id);
        
        static void del_node (unsigned int _id) {
            if (id_node_table.find(_id) != id_node_table.end())
                id_node_table.erase(_id);
        }
        static unsigned int getNodeNum () { return id_node_table.size(); }

        class node_generator {
                // lock the copy constructor
                node_generator(node_generator &){}
                // store all possible types of node
                static map<string,node_generator*> prototypes;
            protected:
                // allow derived class to use it
                node_generator() {}
                // after you create a new node type, please register the factory of this node type by this function
                void register_node_type(node_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your node
                virtual node* generate(unsigned int _id) = 0;
            public:
                // you have to implement your own type() to return your node type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of node derived
        	    static node * generate (string type, unsigned int _id) {
        	        if(id_node_table.find(_id)!=id_node_table.end()){
        	            std::cerr << "duplicate node id" << std::endl; // node id is duplicated
        	            return nullptr;
        	        }
        	        else if ( BROCAST_ID == _id ) {
        	            cerr << "BROCAST_ID cannot be used" << endl;
        	            return nullptr;
        	        }
            		else if(prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            		    node * created_node = prototypes[type]->generate(_id);
            			return created_node; // generate it!!
            		}
            		std::cerr << "no such node type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered node types: " << endl;
            	    for (map<string,node::node_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~node_generator(){};
        };
};
map<string,node::node_generator*> node::node_generator::prototypes;
map<unsigned int,node*> node::id_node_table;


class mycomp {
    bool reverse;
    
    public:
        mycomp(const bool& revparam = false) { reverse=revparam ; }
        bool operator() (const event* lhs, const event* rhs) const;
};

class event {
        event(event*&){} // this constructor cannot be directly called by users
        static priority_queue < event*, vector < event* >, mycomp > events;
        static unsigned int cur_time; // timer
        static unsigned int end_time;
        
        unsigned int trigger_time;
        
        // get the next event
        static event * get_next_event() ;
        static void add_event (event *e) { events.push(e); }
        static hash<string> event_seq;
        
    protected:
        event(){} // it should not be used
        event(unsigned int _trigger_time): trigger_time(_trigger_time) {}
    public:
        virtual void trigger()=0;
        virtual ~event(){}

        virtual unsigned int event_priority() const = 0;
        unsigned int get_hash_value(string string_for_hash) const {
            unsigned int priority = event_seq (string_for_hash);
            return priority;
        }
        
        static void flush_events (); // only for debug
        
        GET(getTriggerTime,unsigned int,trigger_time);
        
        static void start_simulate( unsigned int _end_time ); // the function is used to start the simulation
        
        static unsigned int getCurTime() { return cur_time ; }
        static void getCurTime(unsigned int _cur_time) { cur_time = _cur_time; } 
        // static unsigned int getEndTime() { return end_time ; }
        // static void getEndTime(unsigned int _end_time) { end_time = _end_time; }
        
        virtual void print () const = 0; // the function is used to print the event information

        class event_generator{
                // lock the copy constructor
                event_generator(event_generator &){}
                // store all possible types of event
                static map<string,event_generator*> prototypes;
            protected:
                // allow derived class to use it
                event_generator() {}
                // after you create a new event type, please register the factory of this event type by this function
                void register_event_type(event_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your event
                virtual event* generate(unsigned int _trigger_time, void * data) = 0;
            public:
                // you have to implement your own type() to return your event type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of event derived
        	    static event * generate (string type, unsigned int _trigger_time, void * data) {
            		if(prototypes.find(type) != prototypes.end()){ // if this type derived exists
            		    event * e = prototypes[type]->generate(_trigger_time, data);
            		    add_event(e);
            		    return e; // generate it!!
            		}
            		std::cerr << "no such event type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered event types: " << endl;
            	    for (map<string,event::event_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~event_generator(){}
        };
};
map<string,event::event_generator*> event::event_generator::prototypes;
priority_queue < event*, vector< event* >, mycomp > event::events;
hash<string> event::event_seq;

unsigned int event::cur_time = 0;
unsigned int event::end_time = 0;

void event::flush_events()
{ 
    cout << "**flush begin" << endl;
    while ( ! events.empty() ) {
        cout << setw(11) << events.top()->trigger_time << ": " << setw(11) << events.top()->event_priority() << endl;
        delete events.top();
        events.pop();
    }
    cout << "**flush end" << endl;
}
event * event::get_next_event() {
    if(events.empty()) 
        return nullptr; 
    event * e = events.top();
    events.pop(); 
    // cout << events.size() << " events remains" << endl;
    return e; 
}
void event::start_simulate(unsigned int _end_time) {
    if (_end_time<0) {
        cerr << "you should give a possitive value of _end_time" << endl;
        return;
    }
    end_time = _end_time;
    event *e; 
    e = event::get_next_event ();
    while ( e != nullptr && e->trigger_time <= end_time ) {
        if ( cur_time <= e->trigger_time )
            cur_time = e->trigger_time;
        else {
            cerr << "cur_time = " << cur_time << ", event trigger_time = " << e->trigger_time << endl;
            break;
        }

        // cout << "event trigger_time = " << e->trigger_time << endl;
        e->print(); // for log
        // cout << " event begin" << endl;
        e->trigger();
        // cout << " event end" << endl;
        delete e;
        e = event::get_next_event ();
    }
    // cout << "no more event" << endl;
}

bool mycomp::operator() (const event* lhs, const event* rhs) const {
    // cout << lhs->getTriggerTime() << ", " << rhs->getTriggerTime() << endl;
    // cout << lhs->type() << ", " << rhs->type() << endl;
    unsigned int lhs_pri = lhs->event_priority();
    unsigned int rhs_pri = rhs->event_priority();
    // cout << "lhs hash = " << lhs_pri << endl;
    // cout << "rhs hash = " << rhs_pri << endl;
    
    if (reverse) 
        return ((lhs->getTriggerTime()) == (rhs->getTriggerTime())) ? (lhs_pri < rhs_pri): ((lhs->getTriggerTime()) < (rhs->getTriggerTime()));
    else 
        return ((lhs->getTriggerTime()) == (rhs->getTriggerTime())) ? (lhs_pri > rhs_pri): ((lhs->getTriggerTime()) > (rhs->getTriggerTime()));
}

class recv_event: public event {
    public:
        class recv_data; // forward declaration
            
    private:
        recv_event(recv_event&) {} // this constructor cannot be used
        recv_event() {} // we don't allow users to new a recv_event by themselv
        unsigned int senderID; // the sender
        unsigned int receiverID; // the receiver 
        packet *pkt; // the packet
        
    protected:
        // this constructor cannot be directly called by users; only by generator
        recv_event(unsigned int _trigger_time, void *data): event(_trigger_time), senderID(BROCAST_ID), receiverID(BROCAST_ID), pkt(nullptr){
            recv_data * data_ptr = (recv_data*) data;
            senderID = data_ptr->s_id;
            receiverID = data_ptr->r_id;
            pkt = data_ptr->_pkt;
        } 
        
    public:
        virtual ~recv_event(){}
        // recv_event will trigger the recv function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class recv_event_generator;
        friend class recv_event_generator;
        // recv_event is derived from event_generator to generate a event
        class recv_event_generator : public event_generator{
                static recv_event_generator sample;
                // this constructor is only for sample to register this event type
                recv_event_generator() { /*cout << "recv_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "recv_event generated" << endl; 
                    return new recv_event(_trigger_time, data); 
                }
                
            public:
                virtual string type() { return "recv_event";}
                ~recv_event_generator(){}
        };
        // this class is used to initialize the recv_event
        class recv_data{
            public:
                unsigned int s_id;
                unsigned int r_id;
                packet *_pkt;
        };
        
        void print () const;
};
recv_event::recv_event_generator recv_event::recv_event_generator::sample;

void recv_event::trigger() {
    if (pkt == nullptr) {
        cerr << "recv_event error: no pkt!" << endl; 
        return ; 
    }
    else if (node::id_to_node(receiverID) == nullptr){
        cerr << "recv_event error: no node " << receiverID << "!" << endl;
        delete pkt; return ;
    }
    node::id_to_node(receiverID)->recv(pkt); 
}
unsigned int recv_event::event_priority() const {
    string string_for_hash;
    string_for_hash = to_string(getTriggerTime()) + to_string(senderID) + to_string (receiverID) + to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the recv_event::print() function is used for log file
void recv_event::print () const {
    cout << "time "          << setw(11) << event::getCurTime() 
         << "   recID "      << setw(11) << receiverID 
         << "   pktID"       << setw(11) << pkt->getPacketID()
         << "   srcID "      << setw(11) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         << endl;
    // cout << pkt->type()
    //      << "   time "       << setw(11) << event::getCurTime() 
    //      << "   recID "      << setw(11) << receiverID 
    //      << "   pktID"       << setw(11) << pkt->getPacketID()
    //      << "   srcID "      << setw(11) << pkt->getHeader()->getSrcID() 
    //      << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
    //      << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
    //      << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
    //      << endl;
}

class send_event: public event {
    public:
        class send_data; // forward declaration
            
    private:
        send_event (send_event &){}
        send_event (){} // we don't allow users to new a recv_event by themselves
        // this constructor cannot be directly called by users; only by generator
        unsigned int senderID; // the sender
        unsigned int receiverID; // the receiver 
        packet *pkt; // the packet
    
    protected:
        send_event (unsigned int _trigger_time, void *data): event(_trigger_time), senderID(BROCAST_ID), receiverID(BROCAST_ID), pkt(nullptr){
            send_data * data_ptr = (send_data*) data;
            senderID = data_ptr->s_id;
            receiverID = data_ptr->r_id;
            pkt = data_ptr->_pkt;
        } 
        
    public:
        virtual ~send_event(){}
        // send_event will trigger the send function
        virtual void trigger();
        
        unsigned int event_priority() const;
        
        class send_event_generator;
        friend class send_event_generator;
        // send_event is derived from event_generator to generate a event
        class send_event_generator : public event_generator{
                static send_event_generator sample;
                // this constructor is only for sample to register this event type
                send_event_generator() { /*cout << "send_event registered" << endl;*/ register_event_type(&sample); }
            protected:
                virtual event * generate(unsigned int _trigger_time, void *data){ 
                    // cout << "send_event generated" << endl; 
                    return new send_event(_trigger_time, data); 
                }
            
            public:
                virtual string type() { return "send_event";}
                ~send_event_generator(){}
        };
        // this class is used to initialize the send_event
        class send_data{
            public:
                unsigned int s_id;
                unsigned int r_id;
                packet *_pkt;
        };
        
        void print () const;
};
send_event::send_event_generator send_event::send_event_generator::sample;

void send_event::trigger() {
    if (pkt == nullptr) {
        cerr << "send_event error: no pkt!" << endl; 
        return ; 
    }
    else if (node::id_to_node(senderID) == nullptr){
        cerr << "send_event error: no node " << senderID << "!" << endl;
        delete pkt; return ;
    }
    node::id_to_node(senderID)->send(pkt);
}
unsigned int send_event::event_priority() const {
    string string_for_hash;
    string_for_hash = to_string(getTriggerTime()) + to_string(senderID) + to_string (receiverID) + to_string (pkt->getPacketID());
    return get_hash_value(string_for_hash);
}
// the send_event::print() function is used for log file
void send_event::print () const {
    cout << "time "          << setw(11) << event::getCurTime() 
         << "   senID "      << setw(11) << senderID 
         << "   pktID"       << setw(11) << pkt->getPacketID()
         << "   srcID "      << setw(11) << pkt->getHeader()->getSrcID() 
         << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
         << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
         << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
         << "   "            << pkt->type()
         //<< "   msg"         << setw(11) << dynamic_cast<SDN_data_payload*>(pkt->getPayload())->getMsg()
         << endl;
    // cout << pkt->type()
    //      << "   time "       << setw(11) << event::getCurTime() 
    //      << "   senID "      << setw(11) << senderID
    //      << "   pktID"       << setw(11) << pkt->getPacketID()
    //      << "   srcID "      << setw(11) << pkt->getHeader()->getSrcID() 
    //      << "   dstID"       << setw(11) << pkt->getHeader()->getDstID() 
    //      << "   preID"       << setw(11) << pkt->getHeader()->getPreID()
    //      << "   nexID"       << setw(11) << pkt->getHeader()->getNexID()
    //      << endl;
}


class link {
        // all links created in the program
        static map< pair<unsigned int,unsigned int>, link*> id_id_link_table;
        
        unsigned int id1; // from
        unsigned int id2; // to
        
    protected:
        link(link&){} // this constructor should not be used
        link(){} // this constructor should not be used
        link(unsigned int _id1, unsigned int _id2): id1(_id1), id2(_id2) { id_id_link_table[pair<unsigned int,unsigned int>(id1,id2)] = this; }

    public:
        virtual ~link() { 
            id_id_link_table.erase (pair<unsigned int,unsigned int>(id1,id2)); // erase the link
        }
        
        static link * id_id_to_link (unsigned int _id1, unsigned int _id2) { 
            return ((id_id_link_table.find(pair<unsigned int,unsigned int>(_id1,_id2))!=id_id_link_table.end()) ? id_id_link_table[pair<unsigned,unsigned>(_id1,_id2)]: nullptr) ; 
        }

        virtual double getLatency() = 0; // you must implement your own latency
        
        static void del_link (unsigned int _id1, unsigned int _id2) {
            pair<unsigned int, unsigned int> temp;
            if (id_id_link_table.find(temp)!=id_id_link_table.end())
                id_id_link_table.erase(temp); 
        }

        static unsigned int getLinkNum () { return id_id_link_table.size(); }

        class link_generator {
                // lock the copy constructor
                link_generator(link_generator &){}
                // store all possible types of link
                static map<string,link_generator*> prototypes;
            protected:
                // allow derived class to use it
                link_generator() {}
                // after you create a new link type, please register the factory of this link type by this function
                void register_link_type(link_generator *h) { prototypes[h->type()] = h; }
                // you have to implement your own generate() to generate your link
                virtual link* generate(unsigned int _id1, unsigned int _id2) = 0;
            public:
                // you have to implement your own type() to return your link type
        	    virtual string type() = 0;
        	    // this function is used to generate any type of link derived
        	    static link * generate (string type, unsigned int _id1, unsigned int _id2) {
        	        if(id_id_link_table.find(pair<unsigned int,unsigned int>(_id1,_id2))!=id_id_link_table.end()){
        	            std::cerr << "duplicate link id" << std::endl; // link id is duplicated
        	            return nullptr;
        	        }
        	        else if ( BROCAST_ID == _id1 || BROCAST_ID == _id2 ) {
        	            cerr << "BROCAST_ID cannot be used" << endl;
        	            return nullptr;
        	        }
            		else if (prototypes.find(type) != prototypes.end()){ // if this type derived exists 
            		    link * created_link = prototypes[type]->generate(_id1,_id2);
            			return created_link; // generate it!!
            		}
            		std::cerr << "no such link type" << std::endl; // otherwise
            		return nullptr;
            	}
            	static void print () {
            	    cout << "registered link types: " << endl;
            	    for (map<string,link::link_generator*>::iterator it = prototypes.begin(); it != prototypes.end(); it ++)
            	        cout << it->second->type() << endl;
            	}
            	virtual ~link_generator(){};
        };
};
map<string,link::link_generator*> link::link_generator::prototypes;
map<pair<unsigned int,unsigned int>, link*> link::id_id_link_table;

void node::add_phy_neighbor (unsigned int _id, string link_type){
    if (id == _id) return; // if the two nodes are the same...
    if (id_node_table.find(_id)==id_node_table.end()) return; // if this node does not exist
    if (phy_neighbors.find(_id)!=phy_neighbors.end()) return; // if this neighbor has been added
    phy_neighbors[_id] = true;
    
    link::link_generator::generate(link_type,id,_id);
}
void node::del_phy_neighbor (unsigned int _id){
    phy_neighbors.erase(_id);
    
}


class simple_link: public link {
    protected:
        simple_link() {} // it should not be used outside the class
        simple_link(simple_link&) {} // it should not be used
        simple_link(unsigned int _id1, unsigned int _id2): link (_id1,_id2){} // this constructor cannot be directly called by users
    
    public:
        virtual ~simple_link() {}
        virtual double getLatency() { return ONE_HOP_DELAY; } // you can implement your own latency
        
        class simple_link_generator;
        friend class simple_link_generator;
        // simple_link is derived from link_generator to generate a link
        class simple_link_generator : public link_generator {
                static simple_link_generator sample;
                // this constructor is only for sample to register this link type
                simple_link_generator() { /*cout << "simple_link registered" << endl;*/ register_link_type(&sample); }
            protected:
                virtual link * generate(unsigned int _id1, unsigned int _id2) 
                { /*cout << "simple_link generated" << endl;*/ return new simple_link(_id1,_id2); }
            public:
                virtual string type() { return "simple_link"; }
                ~simple_link_generator(){}
        };
};

simple_link::simple_link_generator simple_link::simple_link_generator::sample;

class SDN_switch: public node {
        // map<unsigned int,bool> one_hop_neighbors; // you can use this variable to record the node's 1-hop neighbors 
        
        bool hi; // this is used for example; you can remove it when doing hw2

    protected:
        SDN_switch() {} // it should not be used
        SDN_switch(SDN_switch&) {} // it should not be used
        SDN_switch(unsigned int _id): node(_id), hi(false) {} // this constructor cannot be directly called by users
    
    public:
        ~SDN_switch(){}
        string type() { return "SDN_switch"; }
        
        // please define recv_handler function to deal with the incoming packet
        virtual void recv_handler (packet *p);
        
        // void add_one_hop_neighbor (unsigned int n_id) { one_hop_neighbors[n_id] = true; }
        // unsigned int get_one_hop_neighbor_num () { return one_hop_neighbors.size(); }
        
        class SDN_switch_generator;
        friend class SDN_switch_generator;
        // SDN_switch is derived from node_generator to generate a node
        class SDN_switch_generator : public node_generator{
                static SDN_switch_generator sample;
                // this constructor is only for sample to register this node type
                SDN_switch_generator() { /*cout << "SDN_switch registered" << endl;*/ register_node_type(&sample); }
            protected:
                virtual node * generate(unsigned int _id){ /*cout << "SDN_switch generated" << endl;*/ return new SDN_switch(_id); }
            public:
                virtual string type() { return "SDN_switch";}
                ~SDN_switch_generator(){}
        };
};
SDN_switch::SDN_switch_generator SDN_switch::SDN_switch_generator::sample;


// the data_packet_event function is used to add an initial event
void data_packet_event (unsigned int src, unsigned int dst, unsigned int t = 0, string msg = "default"){
    if ( node::id_to_node(src) == nullptr || (dst != BROCAST_ID && node::id_to_node(dst) == nullptr) ) {
        cerr << "src or dst is incorrect" << endl; return ;
        return;
    }
    SDN_data_packet *pkt = dynamic_cast<SDN_data_packet*> ( packet::packet_generator::generate("SDN_data_packet") );
    if (pkt == nullptr) { 
        cerr << "packet type is incorrect" << endl; return; 
    }
    SDN_data_header *hdr = dynamic_cast<SDN_data_header*> ( pkt->getHeader() );
    SDN_data_payload *pld = dynamic_cast<SDN_data_payload*> ( pkt->getPayload() );
    
    if (hdr == nullptr) {
        cerr << "header type is incorrect" << endl; return ;
    }
    if (pld == nullptr) {
        cerr << "payload type is incorrect" << endl; return ;
    }
    
    SDN_switch *dst_n = nullptr;
    
    if ( dst != BROCAST_ID) {
        dst_n = dynamic_cast<SDN_switch*> (node::id_to_node(dst));
        if ( dst_n == nullptr) {
            cerr << "node type is incorrect" << endl; return ;
        }
    }

    hdr->setSrcID(src);
    hdr->setDstID(dst);
    hdr->setPreID(src);
    hdr->setNexID(src);

    pld->setMsg(msg);
    
    recv_event::recv_data e_data;
    e_data.s_id = src;
    e_data.r_id = src;
    e_data._pkt = pkt;
    
    // for (unsigned int i = 0; i < 10; i ++){
    //     hdr->push_visited_node(i);
    // }
    // for (unsigned int i = 0; i < 5; i ++){
    //     hdr->mark_visited_node(i);
    // }
    
    recv_event *e = dynamic_cast<recv_event*> ( event::event_generator::generate("recv_event",t, (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

// the ctrl_packet_event function is used to add an initial event
void ctrl_packet_event (unsigned int dst, unsigned int t = 0, string msg = "default"){
    if ( dst == BROCAST_ID || node::id_to_node(dst) == nullptr ) {
        cerr << "dst is incorrect" << endl; return ;
        return;
    }
    SDN_ctrl_packet *pkt = dynamic_cast<SDN_ctrl_packet*> ( packet::packet_generator::generate("SDN_ctrl_packet") );
    if (pkt == nullptr) { 
        cerr << "packet type is incorrect" << endl; return; 
    }
    SDN_ctrl_header *hdr = dynamic_cast<SDN_ctrl_header*> ( pkt->getHeader() );
    SDN_ctrl_payload *pld = dynamic_cast<SDN_ctrl_payload*> ( pkt->getPayload() );
    
    if (hdr == nullptr) {
        cerr << "header type is incorrect" << endl; return ;
    }
    if (pld == nullptr) {
        cerr << "payload type is incorrect" << endl; return ;
    }
    
    SDN_switch *dst_n = nullptr;
    
    if ( dst != BROCAST_ID) {
        dst_n = dynamic_cast<SDN_switch*> (node::id_to_node(dst));
        if ( dst_n == nullptr) {
            cerr << "node type is incorrect" << endl; return ;
        }
    }

    unsigned int src = node::getNodeNum(); // we assume the controller exists and its ID is the largest ID
    hdr->setSrcID(src); 
    hdr->setDstID(dst);
    hdr->setPreID(src);
    hdr->setNexID(dst);

    pld->setMsg(msg);
    
    recv_event::recv_data e_data;
    e_data.s_id = src;
    e_data.r_id = dst;
    e_data._pkt = pkt;
    
    // for (unsigned int i = 0; i < 10; i ++){
    //     hdr->push_visited_node(i);
    // }
    // for (unsigned int i = 0; i < 5; i ++){
    //     hdr->mark_visited_node(i);
    // }
    
    recv_event *e = dynamic_cast<recv_event*> ( event::event_generator::generate("recv_event",t, (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

// send_handler function is used to transmit packet p based on the information in the header
// Note that the packet p will not be discard after send_handler ()

void node::send_handler(packet *p){
    packet *_p = packet::packet_generator::replicate(p);
    send_event::send_data e_data;
    e_data.s_id = _p->getHeader()->getPreID();
    e_data.r_id = _p->getHeader()->getNexID();
    e_data._pkt = _p;
    send_event *e = dynamic_cast<send_event*> (event::event_generator::generate("send_event",event::getCurTime(), (void *)&e_data) );
    if (e == nullptr) cerr << "event type is incorrect" << endl;
}

void node::send (packet *p){ // this function is called by event; not for the user
    if (p == nullptr) return;
    
    unsigned int _nexID = p->getHeader()->getNexID();
    for ( map<unsigned int,bool>::iterator it = phy_neighbors.begin(); it != phy_neighbors.end(); it ++) {
        unsigned int nb_id = it->first; // neighbor id
        
        if (nb_id != _nexID && BROCAST_ID != _nexID) continue; // this neighbor will not receive the packet
        
        unsigned int trigger_time = event::getCurTime() + link::id_id_to_link(id, nb_id)->getLatency() ; // we simply assume that the delay is fixed
        // cout << "node " << id << " send to node " <<  nb_id << endl;
        recv_event::recv_data e_data;
        e_data.s_id = id;
        e_data.r_id = nb_id;
        
        packet *p2 = packet::packet_generator::replicate(p);
        e_data._pkt = p2;
        
        recv_event *e = dynamic_cast<recv_event*> (event::event_generator::generate("recv_event", trigger_time, (void*) &e_data)); // send the packet to the neighbor
        if (e == nullptr) cerr << "event type is incorrect" << endl;
    }
    packet::discard(p);
}

// you have to write the code in recv_handler of SDN_switch
void SDN_switch::recv_handler (packet *p){
    // in this function, you are "not" allowed to use node::id_to_node(id) !!!!!!!!

    // this is a simple example
    // node 0 broadcasts its message to every node and every node relays the packet "only once"
    // the variable hi is used to examine whether the packet has been received before
    // you can remove the variable hi and create your own variables in class SDN_switch
    if (p == nullptr) return ;
    
    if (p->type() == "SDN_data_packet") { // the switch receives a packet from the other switch
        //cout << "node " << getNodeID() << " send the packet" << endl;
        SDN_data_packet * p2 = nullptr;
        p2 = dynamic_cast<SDN_data_packet*> (p);

        if(truth_table[getNodeID()].count(p2->getHeader()->getDstID())==0) return;
        
        //dst doesn't exist  ||  getNodeID == dstNode  ||  data_time < instime 
        if(getNodeID() == p2->getHeader()->getDstID() || truth_table[getNodeID()].empty()) return;

        auto itr = truth_table[getNodeID()].find(p2->getHeader()->getDstID());
        //doesn't connect
        if(itr->second ==-1) return;
        p2->getHeader()->setPreID (getNodeID());
        p2->getHeader()->setNexID (itr->second);
        p2->getHeader()->setDstID (p2->getHeader()->getDstID());
        send_handler (p2);
    }
    else if (p->type() == "SDN_ctrl_packet") { // the switch receives a packet from the controller
        SDN_ctrl_packet *p3 = nullptr;
        p3 = dynamic_cast<SDN_ctrl_packet*> (p);
        SDN_ctrl_payload *l3 = nullptr;
        l3 = dynamic_cast<SDN_ctrl_payload*> (p3->getPayload());
        
        string msg = l3->getMsg(); // get the msg

        //cut string
        stringstream ss(msg);
        string tmp;
        vector<string> vec;
        while(getline(ss, tmp, ' ')){
            vec.push_back(tmp);
        }

        //string_into_int
        vector<int> vec_int;
        for(int i=0;i<2;i++){
            int integer = stoi(vec[i]);
            vec_int.push_back(integer);
        }

        truth_table[getNodeID()][vec_int[0]] = vec_int[1]; 

        //cout << "\n" << getNodeID() << " received packet with msg context \"" << msg << "\""<< endl;
    }

    // you should implement the SDN's distributed algorithm in recv_handler
    // getNodeID() returns the id of the current node
    
    // The current node's neighbors are already stored in the following variable 
    // map<unsigned int,bool> node::phy_neighbors
    // however, this variable is private in the class node
    // You have to use node::getPhyNeighbors to get the variable
    // for example, if you want to print all the neighbors of this node
    // const map<unsigned int,bool> &nblist = getPhyNeighbors();
    // cout << "node " << getNodeID() << "'s nblist: ";
    // for (map<unsigned int,bool>::const_iterator it = nblist.begin(); it != nblist.end(); it ++) {
    //     cout << it->first << ", " ;
    // }
    // cout << endl;


    // you can use p->getHeader()->setSrcID() or getSrcID()
    //             p->getHeader()->setDstID() or getDstID()
    //             p->getHeader()->setPreID() or getPreID()
    //             p->getHeader()->setNexID() or getNexID() to change or read the packet header
    
    // In addition, you can get the packet, header, and payload with the correct type (GR)
    // in fact, this is downcasting
    // SDN_data_packet * pkt = dynamic_cast<SDN_data_packet*> (p);
    // SDN_data_header * hdr = dynamic_cast<SDN_data_header*> (p->getHeader());
    // SDN_data_payload * pld = dynamic_cast<SDN_data_payload*> (p->getPayload());
    
    // you can also change the SDN_data_payload setting
    // pld->setMsg(string): to set the message transmitted to the destination
    
    // Besides, you can use packet::packet_generator::generate() to generate a new packet; note that you should fill the header and payload in the packet
    // moreover, you can use "packet *p2 = packet::packet_generator::replicate(p)" to make a clone p2 of packet p
    // note that if the packet is generated or replicated manually, you must delete it by packet::discard() manually before recv_handler finishes
    
    // "IMPORTANT":
    // You have to "carefully" fill the correct information (e.g., srcID, dstID, ...) in the packet before you send it
    // Note that if you want to transmit a packet to only one next node (i.e., unicast), then you fill the ID of a specific node to "nexID" in the header
    // Otherwise, i.e., you want to broadcasts, then you fill "BROCAST_ID" to "nexID" in the header
    // after that, you can use send() to transmit the packet 
    // usage: send_handler (p);
    
    // note that packet p will be discarded (deleted) after recv_handler(); you don't need to manually delete it
}

bool cmp(test A, test B){
    if(A.input_time!=B.input_time){
        return A.input_time < B.input_time;
    }
    else if(A.input_SrcID!=B.input_SrcID){
        return A.input_SrcID < B.input_SrcID;
    }
    return A.input_DstID < B.input_DstID;
}

void dijkstra(int dst, dat link[100000], int old_new){
    map<int, int> pre, dis, flag;
    priority_queue< pair<int, int>, vector< pair<int,int> >, greater< pair<int,int> > > qu;
    for(unsigned int i = 0; i < node::getNodeNum(); i ++){
        dis[i] = 0x3f3f3f3f;
        pre[i]=-1;
        flag[i]=0;
    }

    dis[dst] = 0;
    pre[dst] = dst;
    qu.push(make_pair(0,dst)); //dis nodeID

    while(!qu.empty()){
        int key=qu.top().second, len = link[key].arr.size();
        qu.pop();
        if(flag[key]!=1){
            flag[key]=1;
            for(int i=0; i<len; i++){
                int neighbor = link[key].arr[i].second, dis_neighbor = link[key].arr[i].first;
                if(flag[neighbor]!=1){
                    if(dis[key]+dis_neighbor < dis[neighbor]){
                        dis[neighbor] = dis[key]+dis_neighbor;    
                        pre[neighbor]=key;
                    }
                    else if(dis[key]+dis_neighbor == dis[neighbor]){
                        pre[neighbor]=min(key,pre[neighbor]);
                    }
                    qu.push(make_pair(dis[neighbor],neighbor));
                }
            }
        }
    }
    if(old_new == 0){
        for(unsigned int i = 0; i < node::getNodeNum(); i ++){
            ans_old_table[i].insert( {dst, pre[i]} );
        }   
    }
    else if(old_new == 1){
        for(unsigned int i = 0; i < node::getNodeNum(); i ++){
            ans_new_table[i].insert( {dst, pre[i]} );
        }
    }    
}

int main()
{
    // header::header_generator::print(); // print all registered headers
    // payload::payload_generator::print(); // print all registered payloads
    // packet::packet_generator::print(); // print all registered packets
    // node::node_generator::print(); // print all registered nodes
    // event::event_generator::print(); // print all registered events
    // link::link_generator::print(); // print all registered links 
    
    // Instime mean the ctrl start  // Upate_lower mean how many packet is before update
    unsigned int node_num, dst_num, link_num, Instime, Updtime, SimDuration, dst_ID[5000], Update_lower = 0;

    cin >> node_num >> dst_num >> link_num >> Instime >> Updtime >> SimDuration;

    for(unsigned int i = 0; i < dst_num; i ++){
        cin >> dst_ID[i];
    }

    //ctrl --> node_ID --> msg(dst, next_ID) small-to-big
    sort(dst_ID, dst_ID + dst_num);

    // read the input and generate switch nodes
    for (unsigned int id = 0; id < node_num; id ++){
        node::node_generator::generate("SDN_switch",id);
    }

    // set switches' neighbors
    for(unsigned int i=0; i < link_num; i ++){
        
        int link_ID, Node1, Node2, oldW, newW;
        cin >> link_ID >> Node1 >> Node2 >> oldW >> newW;
        node::id_to_node(Node1)->add_phy_neighbor(Node2);
        node::id_to_node(Node2)->add_phy_neighbor(Node1);
        link_old[Node1].arr.push_back(make_pair(oldW, Node2));
        link_old[Node2].arr.push_back(make_pair(oldW, Node1));
        link_new[Node1].arr.push_back(make_pair(newW, Node2));
        link_new[Node2].arr.push_back(make_pair(newW, Node1));        
    }
    
    // find shortest path in old & new
    for(unsigned int i = 0; i < dst_num; i ++){
        dijkstra(dst_ID[i], link_old, 0);
    }

    for(unsigned int i = 0; i < dst_num; i ++){
        dijkstra(dst_ID[i], link_new, 1);
    }

    // send data_packet
    unsigned int t, SrcID, DstID, Input_num=0;

    while(cin >> t >> SrcID >> DstID){
        if(t < Updtime) Update_lower ++;
        input_information[Input_num].input_time = t;
        input_information[Input_num].input_SrcID = SrcID;
        input_information[Input_num].input_DstID = DstID;
        Input_num ++;
    }

    //data_packet --> time --> srcID --> dstID
    sort(input_information, input_information + Input_num, cmp);


    int start=0;

    while(input_information[start].input_time < Instime){
        data_packet_event(input_information[start].input_SrcID, input_information[start].input_DstID, input_information[start].input_time);
        start ++;
    }

    // ctrl_packet_event
    //Instime
    for(unsigned int i = 0; i < node::getNodeNum(); i ++){
        for(unsigned int j = 0; j < dst_num; j ++){
            if( dst_ID[j] != i ){
                auto itr = ans_old_table[i].find( dst_ID[j] );
                string msg = to_string( dst_ID[j] ) + " " + to_string(itr->second);
                ctrl_packet_event(i, Instime, msg);
            }     
        }
    }

    // data_packet_event
    for(unsigned int i = start; i < Update_lower; i ++){
        data_packet_event(input_information[i].input_SrcID, input_information[i].input_DstID, input_information[i].input_time);        
    }
        
    // ctrl_packet_event
    //Updatetime
    for(unsigned int i = 0; i < node::getNodeNum(); i ++){
        for(unsigned int j = 0; j < dst_num; j ++){
            if( dst_ID[j] != i){
                auto itr = ans_new_table[i].find( dst_ID[j] );
                auto itr2 = ans_old_table[i].find( dst_ID[j]);
                if(itr->second != itr2->second){
                    string msg = to_string( dst_ID[j] ) + " " + to_string(itr->second);
                    ctrl_packet_event(i, Updtime, msg);
                }                
            }     
        }
    }

    for(unsigned int i = Update_lower; i < Input_num; i ++){
        data_packet_event(input_information[i].input_SrcID, input_information[i].input_DstID, input_information[i].input_time);        
    }

    

/*
    ctrl:
        // msg is used to update an entry in the switch's routing table
        // note that msg should contain the two factors:
        // 1st factor: the destination id
        // 2nd factor: the next hop id toward the destination

    data:
        //data_packet_event(src, dst, t);
*/ 

    // start simulation!!
    event::start_simulate(SimDuration);
    // event::flush_events() ;
    // cout << packet::getLivePacketNum() << endl;
    return 0;
}

