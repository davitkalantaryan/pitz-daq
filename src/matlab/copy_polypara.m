function [ ] = copy_polypara( varargin )
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

numberOfChannelsPerAdc=8;

baseAddr='PITZ.RF/FASTADC/RF2.';
lastAdc=7;
lastChannel=7;
fileName='scopedaq.conf';
firstAdc=0; % this is always the case
firstChannel=0; % this is always the case

if nargin>0
    baseAddr=varargin{1};
end

if nargin>1
    lastAdc=varargin{2};
end

if nargin>2
    lastChannel=varargin{3};
end

if nargin>3
    fileName=varargin{4};
end

%if nargin>4
%    firstAdc=varargin{4};
%end

p1=firstAdc*numberOfChannelsPerAdc+firstChannel;
p2=lastAdc*numberOfChannelsPerAdc+lastChannel;

fileScopeConf=fopen(fileName,'w');
bException=0;

try
    
for p=p1:p2;
    adcNum=floor(p/numberOfChannelsPerAdc);
    channelNum=p-(numberOfChannelsPerAdc*adcNum);
    dcsAddr=sprintf('%sADC%d/CH%.2d.POLYPARA',baseAddr,adcNum,channelNum);
    %fprintf(1,'p=%d, adcNum=%d,channelNum=%d\n',p,adcNum,channelNum);
    polyPara=ttfr(dcsAddr);
    nIndex=p-p1;
    if(nIndex<10)
        fprintf(fileScopeConf,' ');
    end
    fprintf(fileScopeConf,'%d %s\t%d\t%e\t%e\t%e\t1\n',...
        nIndex,dcsAddr,...
        polyPara(1),polyPara(2),polyPara(3),polyPara(4));
    
end

catch excpt
    bException=1;
end

fclose(fileScopeConf);

if(bException)
    rethrow (excpt);
end


end

