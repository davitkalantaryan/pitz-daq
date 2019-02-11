
function varargout = test_for_daq_browser( varargin )
%
%
% file:         test_for_daq_browser.m
% created on:   2018 Nov 25
% created by:   D. Kalantaryan (davit.kalantaryan@desy.de)
%
% The aim of this file is to demonstrate the usage of the mex file 
% 'daq_browser_x_y_z'. The x, y and z are the numbers corresponding 
% to DAQ development release numbers
%
% If you want to write quick test that really uses last (actual) version
% of MEX file, then you can use following MEX file 'actual_daq_browser.mexa64'
%
% Any MATLAB application based on the MEX file, that should be stable 
% (for example is used by shift crew), should used versioned version
% of MEX file, because actual version can be changed in itterface or
% the changes can introduce bug andstable application can be affected
%

if nargin>0
    optionR = varargin{1};
else
    optionR = 'time';
end

actual_daq_browser('--set-log-level',2);

switch optionR
    case 'info'       
        %data =actual_daq_browser('--get-data-for-time-interval',branches,time1i32,time2i32,'--debug-app');
        %data =actual_daq_browser('--get-file-entries','/acs/pitz/daq/2018/06/pitznoadc0/PITZ_DATA.pitznoadc0.2018-06-10-0540.root');
        %data =actual_daq_browser('--get-file-entries','/acs/pitz/daq/2018/06/pitznoadc0/PITZ_DATA.pitznoadc0.2018-06-10-0540.root');
        %data =actual_daq_browser('--get-file-entries','/acs/pitz/daq/2018/06/pitznoadc0/PITZ_DATA.pitznoadc0.2018-06-10-0540.root');
        data =actual_daq_browser('--get-file-entries','/acs/pitz/daq/2018/11/pitznoadc1/PITZ_DATA.pitznoadc1.2018-10-28-1032.root');
    case 'entries'
        branches{1}='RF2Cpl10MWFW';
        branches{2}='RF2Cpl10MWRE';
        data =actual_daq_browser('--get-data-for-entries','/acs/pitz/daq/2018/06/pitzrf2.adc/PITZ_DATA.pitzrf2.adc.2018-06-10-1255.root',branches);
    otherwise 
        time_1 = '2018-06-10_12:00:00';
        time_2 = '2018-06-10_12:00:05';
        
        time1i32 = createepoch(time_1);
        time2i32 = createepoch(time_2);
        
        branches{1}='RF2Cpl10MWFW';
        branches{2}='RF2Cpl10MWRE';
        branches{3}='GUN__COUPLER__PMT_20140905';
        branches{4}='GUN__COUPLER__E_DET_20140905';
        branches{5}='GUN__WG1__THALES_PMT_VAC_20140905';
        branches{6}='GUN__WG2__THALES_PMT_VAC_20140905';
        branches{7}='GUN__WG1__THALES_E_DET_VAC_20140905';
        branches{8}='GUN__WG2__THALES_E_DET_VAC_20140905';
        branches{9}='RF2WG1CavityFW';
        branches{10}='RF2WG2CavityFW';
        branches{11}='RF2WG1CavityRE';
        branches{12}='RF2WG2CavityRE';
        branches{13}='GUN__WG1__THALES_PMT_AIR_20140905';
        branches{14}='GUN__WG2__THALES_PMT_AIR_20140905';
        branches{15}='GUN__WG1__RF_WINDOW_PMT_AIR_20140905';
        branches{16}='GUN__WG2__RF_WINDOW_PMT_AIR_20140905';
        branches{17}='X2TIMER_RF2_MACRO_PULSE_NUMBER_20170512';
        branches{18}='INTERLOCKV4_RF2_G5__CNTMAX__BIT18_20170803';
        
        %data =actual_daq_browser('--get-data-for-time-interval',branches,time1i32,time2i32,'--debug-app');
        data =actual_daq_browser('--get-data-for-time-interval',branches,time1i32,time2i32);
end

if nargout>0
    varargout{1}=data;
else
    assignin('base', 'data', data)
end
