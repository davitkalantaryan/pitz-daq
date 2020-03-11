
#include <eq_client.h>
#include <iostream>

//#define TEST_DOOCSADDR      "PITZ.UTIL/MEMORY/TEST_PROP_0/FL1"
#define TEST_DOOCSADDR      "PITZ.RF/SIS8300DMA/RF5_DMA.ADC0/CH00.ZMQ"

int main()
{
    EqData dataIn, dataOut;
    EqAdr eqAdr;
    EqCall eqCall;

    eqAdr.adr(TEST_DOOCSADDR);
    if(eqCall.get(&eqAdr,&dataIn,&dataOut)){
        ::std::cout << "Unable to get\n";
        return 1;
    }

    EqDataBlock* pDataBlock = dataOut.data_block();

    ::std::cout << pDataBlock->data_u.DataUnion_u.d_float << ::std::endl;

    return 0;
}
