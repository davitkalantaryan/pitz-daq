
Following is the tasaks, that should be done!

1. Facility name for all DAQ collector servers will be following: PITZ.DAQ (instead of old DAQL)
2. There should be one location with the same name as a device name
2. For daqinterface GUI, there should not be necessity for 
	"//afs/ifh.de/group/pitz/doocs/data/DAQdata/bin/DAQinterface/config.h" config file.
	All information can be found using DOOCS interface
3. For daqinterface GUI all DOOCS actions (and networking based actions at all) should be 
	removed from GUI thread.
4. Syncronizing checking procedure (fl: pitz_daq_collectorinfo.cpp, ln:~35, ln:~55) (CreateNew, DeleteInstance,~CollectorInfo)
