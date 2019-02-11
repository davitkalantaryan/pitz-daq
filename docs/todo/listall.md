#To do list

 1. for DAQ collector servers, if config file will be accidentially modified
    and the name of informative property changed, then should be implemented
    API to set it back to the default
 2. Implement some delay during server startup (for being able to control the server from watchdog)
 3. there should be global int property for all daq collectors to 
    et and to unset this variable
 4. For client applications also, this variable should be resolved and proper debugging
    based on the value of this implemented
 5. start button coloring for DAQinterface should be changed
 6. During DAQ collector startup AFS cleanup should be implemented, because sometime
    DAQ collector server can die unexpectively without cleaning up AFS and
    without writing necessary data to the DCache
 7. To investigate and fix following problems
    a) Can't open destination file : "General problem: No route to host"
    b) System error: Input/output error
    Most probably these both errors come from one file access operation (and probably network file
    AFS or DCache)
 8. There is a problem to collect data in the AFS, because of permission
    [pitzdaq5] /doocs/data/DAQdata/daqL/2017/02/pitzdiag.adc % touch test.txt
    touch: cannot touch `test.txt': Permission denied
    [pitzdaq5] /doocs/data/DAQdata/daqL/2017/02/pitzdiag.adc %

    
#Already done list

 1. All servers, step by step should be migrated to the new DOOCS facility
    The name of this facility is PITZ.DAQ. In this facility only DAQ collector
    servers should exists. This will make possible for handler client side applications,
    to find all collector servers
 2. Setting itself in the watchdog_server to online, should be removed
 3. Added new button for setting in the watchdog online or ofline
