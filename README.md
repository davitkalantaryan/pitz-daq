# pitz-daq

![PITZ logo](docs/images/pitz_logo.png)


DAQ is abbreviation for Data AcQuisition
. PITZ has acquisition system in order to collect different data for offline analysis
, historizing or for preparing later on reports, etc.

<br />
PITZ DAQ consists from group of applications and utilites (MATLAB mex files, libraries, etc)
. Each application or utilite orginize one part of the collection procedure, or used for finally getting and displaying data.

<br />
This is from Igor's thesis <br />
It interferes with the PITZ Data AcQuisition system (DAQ) and provides a userfriendly
interface with synchronized spectra information about IL detectors and RF
directional couplers, located in the gun section, for a requested time period. The time
period is usually selected as the period when an IL event occurred, but any other time
range, in which an operator is interested, can be defined. LILI provides plots with the
combined spectra in such a way that is easy to read and to compare their behavior.
The LILI interface is able to show each event (which comes every 0.1 sec during the
normal gun operation) as well as a combination of several events. The combined events
option helps to see a sequence of pulses (presented by different colors) and in case of
an IL event, to understand its origin if there were a few IL detectors triggered.
  <br />

<br />

## versions

### Root file versions  
#### version 0  
![version0](docs/images/root_file_version0.png)  
  
#### version 1  
![version1](docs/images/root_file_version1.png)  
  
### Colector config version  
currently version 1  

## Most used parts of the system are following
### 1. [Data collector servers](docs/subsystems/collectors)  
### 2. Application to control the collectors  
### 3. Data getter and handler applications and tools  

Some additional information can be obtained from the files from 'doc' directory   

## Usefull links  
[API](https://davitkalantaryan.github.io/pitz-daq/docs/doxy/html/index.html)   
[web](https://davitkalantaryan.github.io/pitz-daq/index.html)   <br />  
[github](https://github.com/davitkalantaryan/pitz-daq)		<br />  
[DESY bitbucket](https://github.com/davitkalantaryan/pitz-daq)	<br />  
[todo list](https://docs.google.com/document/d/1iTEdPX8mgdXXk3oF4MCKzOL8G_-i_1ULmwTxfbOzHnk/edit?usp=sharing)  
[binaries](https://desycloud.desy.de/index.php/s/zrrx5ePfa4WPExx?path=%2F)  
[root formats](https://root.cern.ch/doc/master/classTTree.html#a7fdd71ed8f39c76c9a32d6b936a9737a)
[variablarray size](https://root.cern.ch/root/html/tutorials/tree/tree3.C.html)

## Assumptions  
 1.  Additional data size is not changed  

