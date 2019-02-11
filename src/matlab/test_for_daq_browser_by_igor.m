%
% File created by Igor.
% the aim of this file is to test mex file for getting data from
% PITZ DAQ
%

%===============================new_time_input=====================
current_date1 = clock;

prompt = {'Year' 'Month' 'Day' 'Hour' 'Min' 'Sec' 'Interval [sec]'};
dlg_title = 'Input time';
num_lines = 1;
def = {num2str(current_date1(1)),num2str(6),num2str(10), num2str(12) , '00', '00' ,'5' };
answer = inputdlg(prompt,dlg_title,num_lines,def);

DV1 = [str2double(answer{1}) str2double(answer{2}),...
    str2double(answer{3}) str2double(answer{4}) str2double(answer{5}),...
    str2double(answer{6})];

DateNum1 = datenum(DV1);

DateNum2 = DateNum1 + str2double(answer{7})/24/3600;

DV2 = round(datevec(DateNum2));

time_1 = [num2str(DV1(1)) '-' num2str(DV1(2),'%.2d') '-' num2str(DV1(3),'%.2d') '_' ...
    num2str(DV1(4),'%.2d') ':' num2str(DV1(5),'%.2d') ':' num2str(DV1(6),'%.2d')]
time_2 = [num2str(DV2(1)) '-' num2str(DV2(2),'%.2d') '-' num2str(DV2(3),'%.2d') '_' ...
    num2str(DV2(4),'%.2d') ':' num2str(DV2(5),'%.2d') ':' num2str(DV2(6),'%.2d')]

%===============================new_time_input=====================

time1i32 = createepoch(time_1)
time2i32 = createepoch(time_2)

disp('Please wait...')

%return;

branches{1}='RF2Cpl10MWFW';
branches{2}='RF2Cpl10MWRE';
%

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

data =daq_browser_4_0_2('--get-data-for-time-interval',branches,time1i32,time2i32);
