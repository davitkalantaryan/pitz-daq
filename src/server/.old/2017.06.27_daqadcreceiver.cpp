#ifndef DAQADCRECEIVER_HPP
#define DAQADCRECEIVER_HPP

#include "pitz_daq_collectorbase.hpp"

#define    FIX_UNRESOLVED
#define     CODEDAQADCRECEIVER  300	// eq_fct_type number for the .conf file
//#define MAX_BUFFER_NO	16

namespace pitz{ namespace daq{

struct str2_19
{
    int         m_nIteration;
    int         m_nIndex;
    int         m_nChanNum;
    Int_t       seconds;
    Int_t       gen_event;
    Float_t     array_value[2048];
};


class DaqAdcReceiver;

class D_host2 : public D_string
{
public:
#if 1
    std::string __xpg_basename() const{return std::string("Hallo");}
#endif
        DaqAdcReceiver* eq_fct_;
        D_host2(const char* pn, DaqAdcReceiver* eq_fct);
#ifdef OPEN_ALL_TEXT
        //void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
#endif  // #ifdef OPEN_ALL_TEXT
protected:
};


class D_myString : public D_string
{
public:
    D_myString(const char* pn, EqFct* loc);
};



class D_ThreadStatus2 : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{return std::string("Hallo");}
#endif
        DaqAdcReceiver* eq_fct_;
        D_ThreadStatus2(const char* pn, DaqAdcReceiver* eq_fct);
        void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
};


class D_Command2 : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{return std::string("Hallo");}
#endif
        DaqAdcReceiver* eq_fct_;
        D_Command2(const char* pn, DaqAdcReceiver* eq_fct);
        void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
};

class D_Alarm2 : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{return std::string("Hallo");}
#endif
        DaqAdcReceiver* eq_fct_;
        D_Alarm2(const char* pn, DaqAdcReceiver* eq_fct);
        void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
        bool clean;
protected:
};

class D_WriteFile2 : public D_int
{
public:

        DaqAdcReceiver* eq_fct_;
        D_WriteFile2(const char* pn, DaqAdcReceiver* eq_fct);
        void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
        #ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{return std::string("Hallo");}
#endif
};

class DaqAdcReceiver : public CollectorBase
{
private:
protected:
        D_name		alias_;
public:
        DaqAdcReceiver( );

        ~DaqAdcReceiver();

        D_Command2	Command_;
        D_ThreadStatus2 ThreadStatus_;
        D_ThreadStatus2 Conds_;
        D_ThreadStatus2 root_length;
        D_ThreadStatus2 gen_event_;
        D_Alarm2        Alarm_;
        D_WriteFile2    WriteFile_;
        D_ThreadStatus2 datastream_;
        D_ThreadStatus2 serverstatus_;

        D_ThreadStatus2 bad_time_;

        D_host2         host_name;
        MClistener*    listener;

        int  write_file;

        void	update();
        void	init();	// started after creation of all Eq's

        int	fct_code()	{ return CODEDAQADCRECEIVER; }

        static int	conf_done;

};

} // namespace daq
} // namespace pitz

#endif // DAQADCRECEIVER_HPP
