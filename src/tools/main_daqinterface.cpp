#include <QApplication>
#include <QMainWindow>

#include <stdio.h>
#ifdef WIN32
#include <conio.h>
#else
#define _getch(...) getchar()
#endif

#include "daqinterface.hpp"

int g_nDoDebug = 0;

int main(int argc, char *argv[])
{
    for( int i(1); i < argc; ++i )
    {
        // Debug is on or off
        if( strcmp(argv[i],"--do-debug") == 0 || strcmp(argv[i],"-d") == 0 || strcmp(argv[i],"-dba") == 0)
        {
            g_nDoDebug = 1;
            if(i<argc-1)
            {
                g_nDoDebug = atoi(argv[++i]);
            }
        }

    }

    freopen( "/dev/null", "w", stderr);

#if 0
    printf("Press any key to continue!");
    fflush(stdout);
    _getch();
#endif

#ifdef WIN32
    g_nTrickiCondWaitForLibraryCleanup = 1;
    g_nPthreadJoinIgnoreForCleanup = 0;
#endif

    QApplication app(argc, argv);
    app.setFont(QFont("Sans Serif", 10, QFont::Normal));
    MainWindow mainWin;
    mainWin.setFixedWidth(1210);
    //mainWin.setFixedWidth(1190);
    mainWin.setMaximumHeight(720);
    mainWin.show();
    return app.exec();
}
