
// udpmcastdaq_commonheader.h
// 2017 Oct 23

#ifndef udpmcastdaq_commonheader_h
#define udpmcastdaq_commonheader_h


#if 0
#define     MAX_CH_NUM          64
struct  conf_struct2
{
        char   conf_daq[40];
        char 	 doocs_url[80];
        int 	 from;
        int 	 size;
        int 	 step;
        //int    type;
        //int     m_nChanNum;
};
#endif

// data transported from scheduler to subscriber
struct DATA_struct
{
        int              endian;
		int              branch_num_in_rcv_and_next_max_buffer_size_on_root;
        int              seconds;
        int              gen_event;
        int              samples;
        float            f[2048];
};

//#define DATA_OFFSET_ON_DATA_struct 20


#endif // #ifndef udpmcastdaq_commonheader_h
