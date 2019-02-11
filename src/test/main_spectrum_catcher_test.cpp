
// main_spectrum_catcher_test.cpp
// 2017 Jan 22

#include "mclistener.hpp"
#include "udpmcastdaq_commonheader.h"
#include <stdio.h>
#include "util/pitz_daq_util_spectrumcatcher.hpp"
#ifdef _WIN32
#include <conio.h>
#else
#define _getch(...)
#endif

#define HOST_TO_GET	"picus8"
//#define HOST_TO_GET	"vmepitz08"
//#define HOST_TO_GET	"vmepitz21"


#if 0


int main()
{
	pitz::daq::util::SpectrumCatcher aCatcher;

	MClistener::Init();

	aCatcher.Initilize(1024);


	aCatcher.AddData("name1", HOST_TO_GET, 0, 0);


	MClistener::Cleanup();


	return 0;
}


#else

int main()
{
	static const int scnSize = sizeof(DATA_struct) / 4;
	u_long* pnSwapData;
	MClistener aListener;
	int i,nReturn = -1, nReceived, nIteration;
	bool bMCastLibInited = false;
	DATA_struct aBuffer;

	pnSwapData = (u_long*)&aBuffer;

	if (!MClistener::Init()) { goto returnPoint; }
	bMCastLibInited = true;

	if(aListener.ConnectToTheMGroup(HOST_TO_GET)){
		fprintf(stderr, "Unable to connect to the host " HOST_TO_GET "\n");
		goto returnPoint;
	}

#if 1
	if (aListener.SetSocketTimeout(5000)) {
		fprintf(stderr, "Unable to set socket timeout!\n");
		goto returnPoint;
	}
#endif

	for (nIteration = 0; nIteration < 100;++nIteration) {
		nReceived = aListener.recvC(&aBuffer, sizeof(DATA_struct));
		if (nReceived != sizeof(DATA_struct)) { nReceived = -errnoNew; }
		printf("iter: %d, received: %d ", nIteration, nReceived);
		if(nReceived==sizeof(DATA_struct)){

			if(aBuffer.endian!=1){
				for (i = 0; i < scnSize; ++i) { pnSwapData[i]=_byteswap_ulong(pnSwapData[i]); }
			}
#if 0
			struct DATA_struct
			{
				int              endian;
				int              branch_num;
				int              seconds;
				int              gen_event;
				int              samples;
				float            f[2048];
			};
#endif
			printf("{branch_num=%d,seconds=%d,gen_event=%d,samples=%d,f[0]=%f}",
				aBuffer.branch_num, aBuffer.seconds, aBuffer.gen_event, aBuffer.samples, aBuffer.f[0]);
		} // if(nReceived==sizeof(DATA_struct)){

		printf("\n");
	}

	nReturn = 0;


returnPoint:
	if(bMCastLibInited){ aListener .CloseSock();}
	if(bMCastLibInited){MClistener::Cleanup();}

	printf("Press any key to exit!"); fflush(stdout);
	_getch();
	printf("\n");

	return nReturn;
}


#endif
