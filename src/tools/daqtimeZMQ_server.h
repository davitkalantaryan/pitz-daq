#ifndef daqtimeZMQ_server_h
#define daqtimeZMQ_server_h 



/*class  ZMQclass
{

public:

        zmq::context_t *context;
	zmq::socket_t  *subscriber;

	ZMQclass();

	void  initZMQ(void);
};*/
#include <zmq.h>

#ifdef WIN32
#include <WINDOWS.H>
#include <ERRNO.H>
typedef HANDLE TYPE_MUT;
#else
#include <pthread.h>
typedef pthread_mutex_t TYPE_MUT;
#endif


class ZMQsuscrb
{
    class MutexDv;
    friend void AtExitZMQ(void);
public:
    ZMQsuscrb();

    ~ZMQsuscrb();

    int     Connect(const char* Publisher);

    void    Close();

    inline int recv (void *buf_, size_t len_, int flags_ = 0)
    {
        return zmq_recv (m_pSocketZMQ, buf_, len_, flags_);
        /*if (nbytes >= 0)
            return (size_t) nbytes;
        if (zmq_errno () == EAGAIN)
            return 0;
        throw error_t ();*/
    }

private:
    // Static members
    static void*    m_pContextZMQ;
    static MutexDv  m_MutexContext;

    void*           m_pSocketZMQ;

    //int             m_


    /////////////////////////////////////
    class MutexDv
    {
    public:
        MutexDv();
        ~MutexDv();
        int  Lock();
        int  UnLock();

    private:
        TYPE_MUT m_MutexLock;
    };
};

#endif


