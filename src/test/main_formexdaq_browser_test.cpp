//
// file:        mex_simple_root_reader.cpp
//

#include "tool/formexdaq_browser_dynuser.h"

//#define DEBUG_APP(...)  do{mexPrintf(__VA_ARGS__);printf(__VA_ARGS__);}while(0)
#define ROOT_FILE_NAME_DEF "/acs/pitz/daq/2018/06/pitzdiag.adc/PITZ_DATA.pitzdiag.adc.2018-06-08-1526.root"
#define DAQ_ENTRY_NAME      "BOO_E-DET_WG1"


int main()
{
    int nReturn = -1;

    std::vector<std::vector<structCommon> > outVector;
    std::vector<std::string> daqNames;
    daqNames.resize(16);

    daqNames[0]="RF2Cpl10MWFW";
    daqNames[1]= "RF2Cpl10MWRE";
    daqNames[2]="GUN__COUPLER__PMT_20140905";
    daqNames[3]="GUN__COUPLER__E_DET_20140905";
    daqNames[4]="GUN__WG1__THALES_PMT_VAC_20140905";
    daqNames[5]="GUN__WG2__THALES_PMT_VAC_20140905";
    daqNames[6]="GUN__WG1__THALES_E_DET_VAC_20140905";
    daqNames[7]="GUN__WG2__THALES_E_DET_VAC_20140905";
    daqNames[8]="RF2WG1CavityFW";
    daqNames[9]="RF2WG2CavityFW";
    daqNames[10]="RF2WG1CavityRE";
    daqNames[11]="RF2WG2CavityRE";
    daqNames[12]="GUN__WG1__THALES_PMT_AIR_20140905";
    daqNames[13]="GUN__WG2__THALES_PMT_AIR_20140905";
    daqNames[14]="GUN__WG1__RF_WINDOW_PMT_AIR_20140905";
    daqNames[15]="GUN__WG2__RF_WINDOW_PMT_AIR_20140905";

    MultyReadEntriesFromIndexToVectorDl(&outVector,daqNames, 1530572400,1530572403 );



    nReturn = 0;
    return nReturn;

}
