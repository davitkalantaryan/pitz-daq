
#include "eq_adr.h"
#include "eq_data.h"
#include "eq_client.h"
#include "eq_dmsg.h"
#include "eq_ext.h"

#ifdef DMSG

#include <zmq.h>
#include <cstring>
#include <vector>
#include <cstdlib>

#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <dlfcn.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <arpa/inet.h>


static void my_swap2(short *in, short *out, int wc) {
    int i, wd;
    short *li, *lo;
    int mask0 = 0x00ff;
    int mask1 = 0xff00;
	
    li = in;
    lo = out;
    for (i = 0; i < wc; i++) {
        wd = *li++;
        *lo++ = ((wd << 8) & mask1) |
		((wd >> 8) & mask0);
    }
}

static void my_swap3(unsigned char *in, unsigned char *out, int wc) {
    register unsigned char *s;
    register unsigned char *d;
    register unsigned char r, l;
	
    s = in;
    d = out;
	
    while (wc--) {
        r = s[2];
        l = s[0];
        d[0] = r;
        d[1] = s[1];
        d[2] = l;
        s += 3;
        d += 3;
    }
}



static void           *context = (void *) 0; // context

// for linking

static const char     *dmsg_fnm [] = {

       "zmq_ctx_new",
       "zmq_ctx_destroy",
       "zmq_socket",
       "zmq_getsockopt",
       "zmq_setsockopt",
       "zmq_close",
       "zmq_bind",
       "zmq_connect",
       "zmq_poll",
       "zmq_msg_send",
       "zmq_msg_recv",
       "zmq_msg_init",
       "zmq_msg_init_size",
       "zmq_msg_close",
       "zmq_msg_size",
       "zmq_msg_data"
};

static void                        *dmsg_hdl = nullptr;
static std::vector<void *>          dmsg_func;
static int                          dmsg_errf;

static int                          stmp_conf; 
static int                          stmp_activ; // stamp activities
static int                          stmp_div;   // timer update counter
static int                          stmp_dly;   // timer update delay

static int64_t                      mp_num;     // global system stamp (time, macropulse#)
static int32_t                      tm_sec;
static int32_t                      tm_usec; 
#define MY_MAXHOSTNAMELEN 256

static char           hostname [MY_MAXHOSTNAMELEN];
static u_long         hostaddr;


static u_int          ports [512]; // port table

static int            debug = 1;  // global debug
static int            cycles = 100; // number of get cycles 


static int dmsg_open (void) {
        int      i;
        int      sz;
        void     *fh;

        if (dmsg_hdl) return 0;
         
#ifdef __APPLE__

        static char libzmq [] = "libzmq.3.dylib";
        
#else
        static char libzmq [] = "libzmq.so.3";
        
#endif

        dmsg_hdl = dlopen (libzmq, RTLD_LAZY);
        if (!dmsg_hdl) {

            if (!dmsg_errf) {
                printf ("dmsg %s", dlerror ());
            }
            dmsg_errf = 1;
            return 1;
        }
        sz = sizeof (dmsg_fnm) / sizeof (dmsg_fnm [0]);
        for (i = 0; i < sz; i++) dmsg_func.push_back ((void *) 0);

        for (i = 0; i < sz; i++) {

             fh = dlsym (dmsg_hdl, dmsg_fnm [i]);
	     if (!fh) return 1;

             dmsg_func.at (i) = fh;
        }
        return 0;
}


void *
lib_zmq_ctx_new (void)
{
        void   *(*fp) (void);

        fp = (void *(*) (void)) dmsg_func [0];
        return fp ();
}


int
lib_zmq_ctx_destroy (void *cont)
{
        int   (*fp) (void *);
        
        fp = (int (*) (void *)) dmsg_func [1];        
        return fp (cont);
}


void *
lib_zmq_socket (void *cont, int type)
{
        void   *(*fp) (void *, int);
        
        fp = (void *(*) (void *, int)) dmsg_func [2];
        return fp (cont, type);
}


int
lib_zmq_getsockopt (void *sock, int oname, void *oval, size_t *olen)
{
        int   (*fp) (void *, int, void *, size_t *);
        
        fp = (int (*) (void *, int, void *, size_t *)) dmsg_func [3];
        return fp (sock, oname, oval, olen);
}


int
lib_zmq_setsockopt (void *sock, int oname, void *oval, size_t olen)
{
        int   (*fp) (void *, int, void *, size_t);
        
        fp = (int (*) (void *, int, void *, size_t)) dmsg_func [4];
        return fp (sock, oname, oval, olen);
}



int
lib_zmq_close (void *sock)
{
        int   (*fp) (void *);
        
        fp = (int (*) (void *)) dmsg_func [5];
        return fp (sock);
}


int
lib_zmq_bind (void *sock, const char *endpoint)
{
        int   (*fp) (void *, const char *);
        
        fp = (int (*) (void *, const char *)) dmsg_func [6];
        return fp (sock, endpoint);
}


int
lib_zmq_connect (void *sock, const char *ep)
{
        int   (*fp) (void *, const char *);
        
        fp = (int (*) (void *, const char *)) dmsg_func [7];
        return fp (sock, ep);
}


int
lib_zmq_poll (zmq_pollitem_t *items, int nitems, long tmo)
{
        int   (*fp) (zmq_pollitem_t *, int, long);
        
        fp = (int (*) (zmq_pollitem_t *, int, long)) dmsg_func [8];
        return fp (items, nitems, tmo);
}



int
lib_zmq_msg_send (zmq_msg_t *msg, void *sock, int flags)
{
        int   (*fp) (zmq_msg_t *, void *, int);

        fp = (int (*) (zmq_msg_t *, void *, int)) dmsg_func [9];
        return fp (msg, sock, flags);
}

int
lib_zmq_msg_recv (zmq_msg_t *msg, void *sock, int flags)
{
        int   (*fp) (zmq_msg_t *, void *, int);
        
        fp = (int (*) (zmq_msg_t *, void *, int)) dmsg_func [10];
        return fp (msg, sock, flags);
}

int
lib_zmq_msg_init (zmq_msg_t *msg)
{
        int   (*fp) (zmq_msg_t *);
        
        fp = (int (*) (zmq_msg_t *)) dmsg_func [11];
        return fp (msg);
}


int
lib_zmq_msg_init_size (zmq_msg_t *msg, size_t sz)
{
        int   (*fp) (zmq_msg_t *, size_t);
        
        fp = (int (*) (zmq_msg_t *, size_t)) dmsg_func [12];
        return fp (msg, sz);
}


int
lib_zmq_msg_close (zmq_msg_t *msg)
{
        int   (*fp) (zmq_msg_t *);
        
        fp = (int (*) (zmq_msg_t *)) dmsg_func [13];
        return fp (msg);
}


size_t
lib_zmq_msg_size (zmq_msg_t *msg)
{
        size_t   (*fp) (zmq_msg_t *);
        
        fp = (size_t (*) (zmq_msg_t *)) dmsg_func [14];
        return fp (msg);
}



void *
lib_zmq_msg_data (zmq_msg_t *msg)
{
        void   *(*fp) (zmq_msg_t *);
        
        fp = (void *(*) (zmq_msg_t *)) dmsg_func [15];
        return fp (msg);
}



static void* init (void) {
  struct hostent  *hep;
  
  dmsg_errf = 0;
  dmsg_hdl  = (void *) 0;
  context   = (void *) 0;
  
  memset (ports, 0, sizeof (ports));
  
  hostaddr  = 0;
  memset (hostname, 0, sizeof (hostname));
  
  gethostname (hostname, sizeof (hostname));
  
  hep = gethostbyname (hostname);
  if (hep) hostaddr = ntohl (((in_addr *) hep->h_addr_list [0])->s_addr);
  
  void      *p;
  
  
  p = context;
  if (p) {
    return p;
  }
  // load shared library

  if (dmsg_open ()) {

    return (void *) 0;
  }
  
  // create context
  
  p = lib_zmq_ctx_new ();
  if (p) context = p;
  return p;
}




void call_back(void*, EqData *data, dmsg_info_t *ts){
  char buf[2048];

  int length = data->length();

  memset((void*)buf, 0, sizeof(buf));
  int groups, grpsize;
  float grpinc;
  char *str; 
  int c_len;
  time_t tm;
  float st, inc;
  u_int s; 
  float *a;
  int f_len;

  struct timeval dtm;

  gettimeofday(&dtm, NULL);
  
  double tmdif =  ((double)dtm.tv_sec*1000000. + (double)dtm.tv_usec - (double)ts->sec*1000000. - (double)ts->usec)/1000.;
  
  switch(debug) {
  case 1:
  #ifdef MACOSX
    printf("[%5.2f ms]  data of type %s  length:%d %d:%d_%lld stat:%d\n", 
	   tmdif,  data->type_string(), length, 
	   ts->sec, ts->usec, ts->ident, ts->stat);
  #else 
    printf("[%5.2f ms]  data of type %s  length:%d %d:%d_%ld stat:%d\n", 
	   tmdif,  data->type_string(), length, 
	   ts->sec, ts->usec, ts->ident, ts->stat);
  #endif 
    break;
  case 2:
    data->get_string (buf, sizeof(buf));
    #ifdef MACOSX
      printf("%d:%d_%lld DATA: %s\n", ts->sec, ts->usec, ts->ident, buf);
    #else
      printf("%d:%d_%ld ", ts->sec, ts->usec, ts->ident);
      printf("   DATA %s\n", buf);
    #endif    
    break;
  case 3:
  case 4:



    if( data->get_string_arg(buf, sizeof(buf)) == NULL) {
      printf("NULL\n");
    }

    switch(data->type()) {
    case DATA_GSPECTRUM:
      #ifdef MACOSX
        printf("\n%d:%d_%lld %s %s\n", ts->sec, ts->usec, ts->ident, data->type_string(), buf);
      #else
        printf("\n%d:%d_%ld %s %s\n", ts->sec, ts->usec, ts->ident, data->type_string(), buf);
      #endif
      if(data->get_spec_groups (&groups, &grpsize, &grpinc)) {
	printf("group parameters: groups:%d group size:%d group incr.:%f\n", groups, grpsize, grpinc);
      } else {
	printf("Failed to get group parameters\n");
      }
      /* fall through */

    case DATA_SPECTRUM:
      if(data->type() != DATA_GSPECTRUM) {
      #ifdef MACOSX
	printf("\n%d:%d_%lld %s %s\n", ts->sec, ts->usec, ts->ident, data->type_string(), buf);
      #else
        printf("\n%d:%d_%ld ", ts->sec, ts->usec, ts->ident);
	printf(" %s\n", data->type_string());
      #endif
      }
      if(data->get_spectrum (&str, &c_len, &tm, &st, &inc, &s, &a, &f_len) ) {
	printf("Spectrum parameters: Comment:'%s'\n", str);
	printf("tm:%ld status:%d start:%f inc:%f data entries:%d\n", tm, s, st, inc, f_len);
      } else {
	printf("Failed to get spectrum parameters\n");
      }

      if(debug == 4) {
	int once = 1;
	for(int index=0; index<f_len; index++) {

	  if(!once && !((index)%10)) printf("\n");
	  printf("[%05d]:%05.2f\t", index, a[index]);

	  once = 0;
	}
	printf("\n");
      }

      break;

    case DATA_IMAGE:
      {
	time_t psec, pusec;
	int pstat;
	IMH valp;
	data->get_timestamp (&psec, &pusec, &pstat);
	#ifdef MACOSX
	  printf("\n%d:%d_%lld %s %s from image %ld:%ld_%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), buf, psec, pusec, pstat);
	#else 
	  printf("\n%d:%d_%ld %s %s from image %ld:%ld_%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), buf, psec, pusec, pstat);
	#endif
	data->get_image_header(&valp);
	printf("width:%d(xstart:%d, aoi_width:%d)\theight:%d(ystart:%d aoi_height:%d)\tbpp:%d(xbin:%d ybin:%d ebit:%d)\ts_format:%d\ti_format:%d\tframe:%d\tev#:%d\tlen:%d\n",
	       valp.width, valp.x_start, valp.aoi_width, valp.height, valp.y_start, valp.aoi_height,
	       valp.bpp, valp.hbin, valp.vbin, valp.ebitpp,
	       valp.source_format, valp.image_format, valp.frame, valp.event, valp.length);
	
	printf("scale_x:%f\tscale_y:%f\trotation:%f\tflags:0x%X\tfspares:(%f %f %f) \tispares:(%d %d %d)\n\n",
	       valp.scale_x, valp.scale_y, valp.image_rotation, valp.image_flags,
	       valp.fspare2, valp.fspare3, valp.fspare4,
	       valp.ispare2, valp.ispare3, valp.ispare4);
	if(debug == 4) {
	  int maxy = valp.aoi_height;
	  if(maxy <=0) maxy = valp.height;
	  int maxx = valp.aoi_width;
	  if(maxx <=0) maxx = valp.width;
	  if((maxy > 0) && (maxx > 0)) {
	    u_char*d_char;
	    int lenp;
	    int once = 1;
	    int mask = (1<<valp.ebitpp) - 1;
	    data->get_image (&d_char, &lenp);
	    if((lenp > 0) && (lenp == maxx*maxy*valp.bpp)) {
	      u_short * d_short = (u_short*)d_char;;

	      // first we have to check if we need swapping

	      if(((1 != ntohl(1)) && !(valp.image_flags & 1)) || ((1 == ntohl(1)) && (valp.image_flags & 1) )) {
		// We have to swap first
		switch(valp.bpp) {
		case 2:
		  my_swap2((short *)d_short, (short *)d_short, maxy*maxx);
		  break;
		case 3:
		  my_swap3(d_char, d_char, maxy*maxx);
		  break;
		default:
		  break;
		}
	      }
	      


	      for(int y=0; y<maxy; y++) {
		for(int x=0; x<maxx; x++) {
		  if(!once && !((y*maxx +x)%10)) printf("\n");
		  switch(valp.bpp) {
		  case 1:
		    printf("[%03dx%03d]:%d\t", y, x, d_char[y*maxx +x]&mask);
		    break;
		  case 2:
		    printf("[%03dx%03d]:%d\t", y, x, d_short[y*maxx +x]&mask);
		    break;
		  case 3:	 
		    printf("[%03dx%03d]:(%03d,%03d,%03d)\t", y, x, d_char[y*maxx*3 +x*3]&0xFF, d_char[y*maxx*3 +x*3 + 1]&0xFF, d_char[y*maxx*3 +x*3+2]&0xFF);
		    break;
		  }
		  once = 0;
		}
	      }
	    } else {
	      printf("Invalid image length %d != %d x %d x %d \n", lenp, maxx, maxy, valp.bpp);
	    }
	  } else {
	    printf("Invalid geometry %dx%d\n", maxx, maxy);
	  }

	}	
      }
      break;
    case DATA_A_SHORT:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {
	EqDataBlock * blk = data->data_block ();
	short *pshort = blk->data_u.DataUnion_u.d_short_array.d_short_array_val;
	for(int i=0; i<length; i++) {
	  //	  printf("[  %04d]:%d (0x%X) \t", i, data->get_int(i)&0xFFFF, data->get_int(i)&0xFFFF );
	  printf("[%04d]:%d (0x%X) \t", i, pshort[i]&0xFFFF, pshort[i]&0xFFFF );
	  if(i && !(i%10)) printf("\n");
	}
      }
      break;
    case DATA_A_INT:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {
	for(int i=0; i<length; i++) {
	  printf("[%04d]:%d\t", i, data->get_int(i));
	  if(i && !(i%10)) printf("\n");
	}
      }
      break;
       case DATA_A_FLOAT:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {
	for(int i=0; i<length; i++) {
	  printf("[%04d]:%5.2f\t", i, data->get_float(i));
	  if(i && !(i%10)) printf("\n");
	}
      }
      
      break;
            case DATA_A_LONG:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {
	for(int i=0; i<length; i++) {
	  printf("[%04d]:%lld\t", i, data->get_long(i));
	  if(i && !(i%10)) printf("\n");
	}
      }
      
      break;
    case DATA_DOUBLE:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {

	  printf("%5.2f\n", data->get_double());

      }
      
      break;
    case DATA_FLOAT:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {

	  printf("%5.2f\n", data->get_float());

      }
      
      break;
    case DATA_INT:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d ", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
      if(debug == 4) {

	  printf("%d\n", data->get_int());

      }
      
      break;
    case DATA_IFFF:
      #ifdef MACOSX
        printf("%d:%d_%lld %s I:%d F1:%f F2:%f F3:%f\n", ts->sec, ts->usec, ts->ident, data->type_string(), data->get_int(0), data->get_float(1), data->get_float(2), data->get_float(3));
      #else
        printf("%d:%d_%ld %s I:%d F1:%f F2:%f F3:%f\n", ts->sec, ts->usec, ts->ident, data->type_string(), data->get_int(0), data->get_float(1), data->get_float(2), data->get_float(3));
      #endif
      break;
    default:
      #ifdef MACOSX
        printf("%d:%d_%lld %s length:%d  not implemented\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #else
        printf("%d:%d_%ld %s length:%d  not implemented\n", ts->sec, ts->usec, ts->ident, data->type_string(), length);
      #endif
    }
    break;
  default:
    break;
  }
}



struct chan_cb {

       struct chan_cb  *next;

       void            *pp;
       void            (*cb) (void *, EqData *, dmsg_info_t *);
};


struct chan {

       char           name [ADDR_STRING_LENGTH];
       char           host [MY_MAXHOSTNAMELEN];
       int            loc;
       u_int          port;
       u_int          type;
       int            len;
       char           *dp;
       int            hwm;
       int            refcnt;
       struct chan_cb *root;
       int            cmd;
};


void dmsg_err (const char *msgp) {
  printf("dmsg: %s\n", msgp);
}



int get_bind_info (EqAdr *ea, EqData *ed, char *hostp, int len, int *portp, int *typep) {
        int          rc;
        float        f1;
        float        f2;
        char         *sp;
        time_t       tm;
        EqAdr        addr;
        EqData       dat;
        EqCall       eq;

        // get channel port number

        addr = ea;

        addr.set_property ("SPN");

        printf("Trying %s\n", addr.show_adr().c_str());

        dat.set (1, 0.0f, 0.0f, (time_t) 0, ea->property (), 0);

        rc = eq.set (&addr, &dat, ed);
        if (rc || ed->error ()) {
            const char  *emp = "can not get channel port";
            printf("ERROR: %s prop:%s\n", ed->get_string().c_str(), ea->property ());
	    

            dmsg_err (emp);

            ed->error (ERR_ILL_SERV, emp);
            return -1;
        }


        if (ed->type () == DATA_INT) {

            // old style of subscription

            int       i;
            int       len;
 
            *portp = ed->get_int ();
        
            // get channel type
        

            addr.set_property ("*");
 
            rc = eq.names (&addr, ed);
            if (rc) {
            
                const char  *emp = "can not get channel data type";

                dmsg_err (emp);

                ed->error (ERR_ILL_SERV, emp);
                return -1;
            }
        
            rc  = -1;
            len = strlen (ea->property ());
        
            for (i = 0; i < ed->length (); i++) {
                 USTR       *up;
                          
                 up = ed->get_ustr (i);
                                       
                 if (strncmp (ea->property (), up->str_data.str_data_val, len)) continue;
                 
                 *typep = up->i1_data;

                 rc = 0;
                 break;
            }
            if (rc < 0) {
                const char  *emp = "invalid channel name";

                dmsg_err (emp);

                ed->error (ERR_ILL_SERV, emp);
                return -1;
            }
 
        } else {

            ed->get_ustr (portp, &f1, &f2, &tm, &sp, 0);                
            *typep = (int) f1;
        }
                
        // get channel host name

        rc = eq.get_option (&addr, &dat, ed, EQ_HOSTNAME);
        if (rc) {
            const char  *emp = "can not get channel host";

            dmsg_err (emp);

            ed->error (ERR_ILL_SERV, emp);
            return -1;
        }

        ed->get_string (hostp, len);

//        eq.set_option (&addr, &dat, &dat, EQ_DESTROY); // disconnect
        
        // check whether the port number is valid

        if (!*portp) {
            const char  *emp = "invalid port number";

            dmsg_err (emp);

            ed->error (ERR_ILL_SERV, emp);
            return -1;
        }
        return 0;
}

int set_con_type (const char *name, char *machname, u_long machaddr, int *locp) {
        u_long          ipa;
        struct hostent  *hep;

        *locp = 1;

        if (!strcmp (name, machname)) return 0; // local

        #ifdef __APPLE__

        hep = gethostbyname (name);
        if (!hep) {
        
            *locp = 0;
            return -1; // error, set remote
        }

        #elif defined __GNUC__

        int             cc;
        int             err;
        struct hostent  host;
        char            buf [1024];
	
	
        cc = gethostbyname_r (name, &host, buf, sizeof (buf), &hep, &err);
        if (cc || !hep) {
        
            *locp = 0;
            return -1; // error, set remote
        }

        #else

        int             err;
        struct hostent  host;
        char            buf [1024];
	
        hep = gethostbyname_r (name, &host, buf, sizeof (buf), &err);
        if (!hep) {
        
            *locp = 0;
            return -1; // error, set remote
        }

        #endif
        
        ipa = ntohl (((in_addr *) hep->h_addr_list [0])->s_addr);
        
        if (ipa == machaddr) return 0; // local

        // remote
        
        *locp = 0;
        return 1;
}


int store_data(EqData* edp, dmsg_info_t* ip, char* dp, int sz)
{
    dmsg_hdr_t* hp;
    EqDataBlock* db;
    USTR* usp;
    int len;
    int type;

    hp = (dmsg_hdr_t*)dp;
    dp += hp->size;
    sz -= hp->size;

    // fill the message info block

    ip->ident = hp->ident;
    ip->sec = hp->sec;
    ip->usec = hp->usec;
    ip->stat = hp->stat;

    // fill EqData object

    db = edp->data_block();

    len = (int)hp->len;
    type = hp->type;
    
    db->tm = (time_t)hp->sec;
    db->error = (int)hp->stat;

    db->data_u.data_sel = type;
    switch (type) {

    case DATA_INT:
        memcpy(&db->data_u.DataUnion_u.d_int, dp, sizeof(db->data_u.DataUnion_u.d_int));
        break;

    case DATA_FLOAT:
        memcpy(&db->data_u.DataUnion_u.d_float, dp, sizeof(db->data_u.DataUnion_u.d_float));
        break;

    case DATA_DOUBLE:
        memcpy(&db->data_u.DataUnion_u.d_double, dp, sizeof(db->data_u.DataUnion_u.d_double));
        break;

    case DATA_IIII:
        memcpy(&db->data_u.DataUnion_u.d_iiii, dp, sizeof(db->data_u.DataUnion_u.d_iiii));
        break;

    case DATA_IFFF:
        memcpy(&db->data_u.DataUnion_u.d_ifff, dp, sizeof(db->data_u.DataUnion_u.d_ifff));
        break;

    case DATA_TTII:
        memcpy(&db->data_u.DataUnion_u.d_ttii, dp, sizeof(db->data_u.DataUnion_u.d_ttii));
        break;

    case DATA_XYZS:
        memcpy(&db->data_u.DataUnion_u.d_xyzs, dp, sizeof(db->data_u.DataUnion_u.d_xyzs));
        db->data_u.DataUnion_u.d_xyzs.loc.loc_val = dp + sizeof(XYZS);
        break;

    case DATA_SPECTRUM:
        memcpy(&db->data_u.DataUnion_u.d_spectrum, dp, sizeof(db->data_u.DataUnion_u.d_spectrum));
        db->data_u.DataUnion_u.d_spectrum.comment.comment_val = dp + sizeof(SPECTRUM);
        db->data_u.DataUnion_u.d_spectrum.d_spect_array.d_spect_array_val = (float*)(dp + sizeof(SPECTRUM) + STRING_LENGTH);
        break;
        
    case DATA_GSPECTRUM:
        memcpy(&db->data_u.DataUnion_u.d_gspectrum, dp, sizeof(db->data_u.DataUnion_u.d_gspectrum));
        db->data_u.DataUnion_u.d_gspectrum.comment.comment_val = dp + sizeof(GSPECTRUM);
        db->data_u.DataUnion_u.d_gspectrum.d_gspect_array.d_gspect_array_val = (float*)(dp + sizeof(GSPECTRUM) + STRING_LENGTH);
        break;

    case DATA_A_SHORT:
        db->data_u.DataUnion_u.d_short_array.d_short_array_val = (short*)dp;
        db->data_u.DataUnion_u.d_short_array.d_short_array_len = len;
        break;

    case DATA_A_INT:
        db->data_u.DataUnion_u.d_int_array.d_int_array_val = (int*)dp;
        db->data_u.DataUnion_u.d_int_array.d_int_array_len = len;
        break;

    case DATA_A_LONG:
        db->data_u.DataUnion_u.d_llong_array.d_llong_array_val = (long long*)dp;
        db->data_u.DataUnion_u.d_llong_array.d_llong_array_len = len;
        break;

    case DATA_A_FLOAT:
        db->data_u.DataUnion_u.d_float_array.d_float_array_val = (float*)dp;
        db->data_u.DataUnion_u.d_float_array.d_float_array_len = len;
        break;

    case DATA_A_DOUBLE:
        db->data_u.DataUnion_u.d_double_array.d_double_array_val = (double*)dp;
        db->data_u.DataUnion_u.d_double_array.d_double_array_len = len;
        break;

    case DATA_STRING:
    case DATA_TEXT:
        db->data_u.DataUnion_u.d_char.d_char_val = dp;
        db->data_u.DataUnion_u.d_char.d_char_len = strlen(dp);
        break;

    case DATA_A_USTR:
        db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_val = (USTR*)dp;
        db->data_u.DataUnion_u.d_ustr_array.d_ustr_array_len = len;
        usp = (USTR*)dp;
        usp->str_data.str_data_val = dp + sizeof(USTR);
        break;

    case DATA_IMAGE: {
        char* ptr = dp;


        memcpy(&db->data_u.DataUnion_u.d_image.hdr, dp, sizeof(db->data_u.DataUnion_u.d_image.hdr));

        dp += sizeof(db->data_u.DataUnion_u.d_image.hdr);
        db->data_u.DataUnion_u.d_image.sec = *(time_t*)dp;

        dp += sizeof(time_t);
        db->data_u.DataUnion_u.d_image.usec = *(time_t*)dp;

        dp += sizeof(time_t);
        db->data_u.DataUnion_u.d_image.status = *(int*)dp;

        dp += sizeof(int);
        db->data_u.DataUnion_u.d_image.comment.comment_val = dp;
        db->data_u.DataUnion_u.d_image.comment.comment_len = strlen(dp) + 1;

        dp += strlen(dp) + 1;
        db->data_u.DataUnion_u.d_image.val.val_val = (u_char*)dp;
        db->data_u.DataUnion_u.d_image.val.val_len = sz - (dp - ptr);
    } break;

    case DATA_A_BYTE: {
        int* hdp = (int*)dp;

        db->data_u.DataUnion_u.d_byte_struct.x_dim = hdp[0];
        db->data_u.DataUnion_u.d_byte_struct.y_dim = hdp[1];
        db->data_u.DataUnion_u.d_byte_struct.x_offset = hdp[2];
        db->data_u.DataUnion_u.d_byte_struct.y_offset = hdp[3];
        db->data_u.DataUnion_u.d_byte_struct.option = hdp[4];

        dp += sizeof(int) * 5;

        db->data_u.DataUnion_u.d_byte_struct.d_byte_array.d_byte_array_len = hdp[0];
        db->data_u.DataUnion_u.d_byte_struct.d_byte_array.d_byte_array_val = (u_char*)dp;
    } break;

    default:
        return 1;
    }
    return 0;
}


int store_msg (struct chan *chp, std::vector<zmq_msg_t *> *mq, EqData *edp, dmsg_info_t *ip) {
        int       i;
        int       len;
        int       tmp;
        char      *dp;
        char      *dtp;
        
        len = 0;
        
        // calculate length of data
        
        for (i = 0; (u_int) i < mq->size (); i++) {
             zmq_msg_t    *msgp;
             
             msgp = mq->at (i);
             len += lib_zmq_msg_size (msgp);
        }
        tmp = len / sizeof (int);
        if (len % sizeof (int)) tmp++;
        
        
        if (chp->len < len) {
            if (chp->dp) delete chp->dp;
                        
            chp->dp  = (char *) new int [tmp];
            chp->len = tmp;
        }
        dp  = chp->dp;
        dtp = dp;
        
        // store data

        for (i = 0; (u_int) i < mq->size (); i++) {

             zmq_msg_t    *msgp;
             int          sz;
             
             msgp = mq->at (i);
             
             sz = lib_zmq_msg_size (msgp);
             memcpy (dp, (char *) lib_zmq_msg_data (msgp), sz);
             dp += sz;
        }
        
        // update EqData

        store_data (edp, ip, dtp, len);
        return 0;
}


// source of update of the global system mask

#define  STAMP_TM                0 // timer
#define  STAMP_SG                1 // interruprt (signal)
#define  STAMP_CB                2 // message (zmq)

int set_global_stamp_src (int src) {

        if (src < STAMP_TM && src > STAMP_CB) return -1;

        // configure the global stamp update rules

        if (src == STAMP_CB) stmp_conf = 1 << STAMP_CB;
        else
        if (src == STAMP_SG) stmp_conf = 1 << STAMP_SG;
        else stmp_conf = 1 << STAMP_TM;
        return 0;
}

int set_global_stamp (int src, int32_t sec, int32_t usec, int64_t mpnum) {
        int     bit;

        if (src < STAMP_TM && src > STAMP_CB) return -1;
                
        
        bit = 1 << src;


        if (stmp_conf & bit) {

            tm_sec  = sec;
            tm_usec = usec;
            mp_num  = mpnum;
            
            stmp_activ |= bit;

            if (src != STAMP_TM) {

                bit = 1 << STAMP_TM;

                if (stmp_conf & bit) {

                    stmp_conf  ^= bit; // reset timer updates
                    stmp_activ ^= bit; // reset timer activities
                    stmp_div    = 0;   // reset timer events divider
                    
                    printf("Global timestamp using interrupt");
                }
            }


            return 0;
        }
        
        // only for server updates
        
        if (src == STAMP_TM) {
        
            if (stmp_activ) {

                stmp_activ = 0; // reset timer activities
                stmp_div   = 0; // reset timer events divider

            } else
            if (stmp_div++ > stmp_dly) {

                stmp_div = 0; // reset timer events divider

                // switch to update mode 

               stmp_conf |= bit;

               printf ("Global timestamp replaced by timer update");
            }
        }

        return 0;
}

int receive_msg (void *sockp, struct chan *chp) {
        u_int           i;
        int             rc;
        zmq_msg_t       *msgp;
        EqDataBlock     db;
        dmsg_info_t     infob;
        struct chan_cb  *ccp;
        
        EqData          ed (&db, 1);

        std::vector<zmq_msg_t *>  msgq;

        rc = 0;
        for ( ; ; ) {
        
             size_t     size;
             int        more;

             msgp = new zmq_msg_t; 

             lib_zmq_msg_init (msgp);

             rc = lib_zmq_msg_recv ( msgp, sockp, 0);
             if (rc < 0) {
                 dmsg_err ("can not receive");
                 break;
             }

             msgq.push_back (msgp);
              
             size = sizeof (more);
             rc = lib_zmq_getsockopt (sockp, ZMQ_RCVMORE, &more, &size);
             if (rc || !more) break;
        }
        if (!rc) {
            rc = store_msg (chp, &msgq, &ed, &infob);
            if (rc) dmsg_err ("can not store message");

        }

        for (i = 0; i < msgq.size (); i++) {
             msgp = msgq [i];
             
             lib_zmq_msg_close (msgp);
             delete msgp;
        }

        if (rc) return rc;
                
        // call all registered callbacks
        


        // set global stamp

        set_global_stamp (STAMP_CB, infob.sec, infob.usec, infob.ident);
        
	for (ccp = chp->root; ccp; ccp = ccp->next) {
             ccp->cb (ccp->pp, &ed, &infob);
        }
	
        return 0;
}




void * dmsg_rcv (struct chan *ip) {
        struct chan    *chp;
        void           *sock;
        int            rc;
        int            loc;
        int            port;

        int            attempts_port;
        int            attempts_sock;
        int            attempts_poll;
        char           name [ADDR_STRING_LENGTH];
        char           opts [ADDR_STRING_LENGTH];
        char           host [MY_MAXHOSTNAMELEN];
        char           addr [512];
        zmq_pollitem_t items;
        int            hwm;
        char           message       [MSG_SIZE];
        
        memset (message, 0, sizeof (message));
         
        chp = (struct chan *) ip;
        
        memset (opts, 0, sizeof (opts));


        memcpy (name, chp->name, sizeof (name));
	memset ((void*)host, 0, sizeof (host));

	memcpy (host, chp->host, sizeof (host));
	port = chp->port;
	loc  = chp->loc;
	hwm  = chp->hwm;
        
        rc = chp->cmd;
        chp->cmd = 0;
        
        sock = (void *) 0;

        attempts_port = 1;
        attempts_sock = 0;
        attempts_poll = 0;

        // input stream processing

	port = -1;

        while (cycles-- >0) {


             if (port == -1) {
                 int        type;
                 EqAdr      ea;
                 EqData     ed;
 
                 attempts_sock = 0;  // reset attemps counter
                 attempts_poll = 0;

                 type = 0;

                 if (--attempts_port) rc = -1;
                 else {
                     attempts_port = 10;

                     // get a new configuration from server every 3 sec

                     ea.adr (name);

                     rc = get_bind_info (&ea, &ed, host, sizeof (host), &port, &type);

            		     printf("Connecting to %s:%d  data type:%d\n", host, port, type);

                     if (rc) {
                         char      abuf [256];
                         char      ebuf [128];


                         ea.show_adr (abuf, sizeof (abuf));
                         printf ("%s - %s", abuf, ed.get_string (ebuf, sizeof (ebuf)));
                     }
                 } 
                         
                 
                 if (rc < 0) {

                     // wait and try again

                     poll (0, 0, 300); // wait 300 msec
                     continue;
                 }

                 set_con_type (host, hostname, hostaddr, &loc);
                                                           
                 chp->port = port;
                 chp->type = type;
                 chp->loc  = loc;

                 memcpy (chp->host, host, sizeof (host));
             }

             if (!sock) {

                 // open connection      

                 sock = lib_zmq_socket (context, ZMQ_SUB);
                 if (!sock) {
                     dmsg_err ("can not create");
                     
                     attempts_sock++; // increment attempts counter
                     
                     // wait and try again

                     poll (0, 0, 300);
                     continue;
                 }
         
                 memset (opts, 0, sizeof (opts));

                 // subscribe

                 rc = lib_zmq_setsockopt (sock, ZMQ_SUBSCRIBE, opts, strlen (opts));
                 if (rc < 0) {
                     dmsg_err ("can not configure socket");

                     lib_zmq_close (sock);
                     sock = (void *) 0;

                     attempts_sock++; // increment attempts counter
                     
                     // wait and try again

                     poll (0, 0, 300);
                     continue;
                 }

                 // set the high water mark (fifo for received messages)
		 if (hwm < 0) hwm = 200; 
		 printf("hwm: %d, sock:%d\n", hwm, *((int*)sock));

                 rc = lib_zmq_setsockopt (sock, ZMQ_RCVHWM, &hwm, sizeof (hwm));
                 if (rc < 0){
		   perror("lib_zmq_setsockopt");
		   dmsg_err ("can not set hwm");
		 }
                 if (loc) snprintf (addr, sizeof (addr), "ipc:///tmp/%d", port);
                 else     snprintf (addr, sizeof (addr), "tcp://%s:%d", host, port);                

                 // connect to the server

		 printf("Trying to connect %s\n", addr);

                 rc = lib_zmq_connect (sock, addr);

		 printf("Connected %s  rc:%d\n", addr, rc);

                 if (rc < 0) {
                     dmsg_err ("can not connect");
                     
                     memset (opts, 0, sizeof (opts));

                     // unsubscribe

                     lib_zmq_setsockopt (sock, ZMQ_UNSUBSCRIBE, opts, strlen (opts));

                     // close socket

                     lib_zmq_close (sock);
                     sock = (void *) 0;

                     attempts_sock++; // increment attempts counter
                     
                     // wait and try again

                     poll (0, 0, 300);
                     continue;
                 }
                 attempts_poll = 0;

                 items.socket = sock;
                 items.fd     = 0;
                 items.events = ZMQ_POLLIN;
             }
             // wait for data

             items.revents = 0;
             rc = lib_zmq_poll (&items, 1, 250); // 250 msec
             if (rc <= 0) {

                 if (++attempts_poll < 100) continue; // try to poll
                 
                 // after 1 min no response - reestablish a connection
                 
                 memset (opts, 0, sizeof (opts));

                 // unsubscribe

                 lib_zmq_setsockopt (sock, ZMQ_UNSUBSCRIBE, opts, strlen (opts));

                 // close socket

                 lib_zmq_close (sock);
                 sock = (void *) 0;
                 
                 attempts_sock++;
                 continue;
             }
             
             // receive data ....
             
             receive_msg (sock, chp);

             attempts_poll = 0;
             attempts_sock = 0;
        }
        
        // close connection      

        if (sock) lib_zmq_close (sock);
        return ip;
}
#endif

int main(int argc, char ** argv) {

#ifdef DMSG

  struct chan chp;
  struct chan_cb cb;
  const char * chan = NULL;


  for(int i=1; i<argc; i++) {
    if(!strcmp(argv[i], "-c")) {
      if((i+1) < argc) {
	chan = argv[++i];
      } else break;
    } else if(!strcmp(argv[i], "-n")) {
      if((i+1) < argc) {
	cycles = atoi(argv[++i]);
      } else break;
    } else if(!strcmp(argv[i], "-d")) {
      if((i+1) < argc) {
	debug = atoi(argv[++i]);
      } else break;
    }
  }


  if(!chan) {
    printf("Usage: %s -c DOOCS_address [-n cycles][-d debug_level]\n", argv[0]);
    printf("\n");
    printf("DOOCS_addres   property DOOCS addres to subscribe\n");
    printf("cycles         number of get cycles (default:%d)\n", cycles);
    printf("debug_level    debug_level (default:%d)  as:\n", debug);
    printf("          0 - print nothing (silent mode)\n");
    printf("          1 - print data type, timing/event/status info\n");
    printf("          2 - print data for simple data types and info for arrays/images\n");
    printf("          3 - print parameters for all data types\n");
    printf("          4 - print data for all data types\n");
    return -1;
  }

  memset((void*)&chp, 0, sizeof(chp));
  memset((void*)&cb, 0, sizeof(cb));

  chp.root = &cb;
  cb.cb = call_back;
 

  snprintf(chp.name, sizeof(chp.name), "%s", chan); 
  if(init ()) {
    dmsg_rcv (&chp);
    lib_zmq_ctx_destroy (context); 
  } else {
    printf("init () failed\n");
  }

#else
  printf("It's not suppoterd for this computer platform, SORRY\n");
#endif
}
