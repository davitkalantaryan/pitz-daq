/*
 *	File      : main_polux_client.cpp
 *
 *	Created on: 22 Mar, 2017
 *	Author    : Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *
 */
#include "pitz_daq_daqinterface_application.hpp"
#include "pitz_daq_daqinterface_mainwindow.hpp"
#include <stdio.h>

int main(int argc, char* argv[])
{
    freopen( "/dev/null", "w", stderr);

    pitz::daq::daqinterface::Application app(argc,argv);
    //QCoreApplication::instance();

    qRegisterMetaType<pitz::daq::daqinterface::STaskDoneStruct>( "pitz::daq::daqinterface::STaskDoneStruct" );
    qRegisterMetaType<pitz::daq::daqinterface::SCollemtorItem*>( "pitz::daq::daqinterface::SCollemtorItem*" );
    qRegisterMetaType<pitz::daq::daqinterface::collectorChangeCode>( "pitz::daq::daqinterface::collectorChangeCode" );

    pitz::daq::daqinterface::MainWindow aMainWnd;
    aMainWnd.show();

    app.exec();

    return 0;
}
