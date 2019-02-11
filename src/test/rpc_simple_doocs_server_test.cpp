
#include "eq_fct.h"
#include <stdio.h>
#include <unistd.h>

const char*	object_name = "test_doocs_server";
void interrupt_usr1_prolog(int)  {}
void	eq_init_epilog() 	{}
void eq_cancel(void){}
void post_init_prolog(void){}
void	eq_init_prolog() 	{}


extern int ring_buffer;

class EqFctTestServer : public EqFct
{
public:
        EqFctTestServer() :
                EqFct("NAME = location")
        {
            printf("!!!!! %s\n",__FUNCTION__);
        }

        int fct_code(){ return 306; }

        void    update(void)
        {
#if 1
                static int snSeverity = 0;
                static int snValue = 0;
                //err_.set_value(13,NULL,snValue++,0);
                //err_.set_value(snValue,NULL);
                set_error(snValue,"blah");
                //usleep(100000);
                //
                printf("!!!!!!!!!!!!!! val=%d, err=%d, _status=%x\n", snValue,get_error(),g_sts_.status_);
                if(snValue==0){snValue=13;}
#endif
        }

        void init()
        {
        }

};



EqFct* eq_create(int eq_code, void*)
{
    EqFct* pRet = NULL;
    const char* cpcNameString;

        switch (eq_code)
        {
        case 306:
            printf("!!!!!!!!!!!\n");
        pRet = new EqFctTestServer;
        cpcNameString = pRet->name_str();
        printf("!!!!!!!!!!!name_str = \"%s\"\n",cpcNameString);
        break;
        default: break;
        }
    return pRet;
}

void refresh_prolog(){}
void refresh_epilog()	{}	// called after "update"
void post_init_epilog(void)	 {}
void interrupt_usr1_epilog(int)  {}
