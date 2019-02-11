#!/bin/bash
#
### BEGIN INIT INFO
# Author:		Davit Kalantaryan
# Script-name:		prepare.sh
# Required-Stop:   $network $remote_fs $syslog
# Default-Start:   2 3 4 5
# Default-Stop:    1
# Short-Description: Start watchdog_server 
### END INIT INFO

REPO_PATH=/doocs/develop/kalantar/programs/cpp/works/pitz/sys
SL5_PATH=$REPO_PATH/Boron/bin
SL6_PATH=$REPO_PATH/Santiago/bin
SL7_PATH=$REPO_PATH/Nitrogen/bin
SERVER_PATH=/export/doocs/server

# UDP multicast base
scp $SL5_PATH/udpmcastdaq pitzdaq1:$SERVER_PATH/pitzlsradc_server/pitzlsradc_server
#scp $SL5_PATH/udpmcastdaq pitzdaq2:$SERVER_PATH/pitzrf1adc_server/pitzrf1adc_server
scp $SL7_PATH/udpmcastdaq pitzdaq7:$SERVER_PATH/pitzrf1adc_server/pitzrf1adc_server  # ?
scp $SL5_PATH/udpmcastdaq pitzdaq3:$SERVER_PATH/pitzbpmadc_server/pitzbpmadc_server
scp $SL5_PATH/udpmcastdaq pitzdaq4:$SERVER_PATH/pitzrf2adc_server/pitzrf2adc_server
scp $SL6_PATH/udpmcastdaq pitzdaq5:$SERVER_PATH/pitzdiagadc_server/pitzdiagadc_server
scp $SL6_PATH/udpmcastdaq pitzdaq6:$SERVER_PATH/pitzwsadc_server/pitzwsadc_server   # ??

# Request Reply based
#scp $SL5_PATH/requestreplydaq pitzdaq2:$SERVER_PATH/pitznoadc2_server/pitznoadc2_server
scp $SL7_PATH/requestreplydaq pitzdaq7:$SERVER_PATH/pitznoadc2_server/pitznoadc2_server  # ?
scp $SL5_PATH/requestreplydaq pitzdaq3:$SERVER_PATH/pitzadc10hz_server/pitzadc10hz_server
scp $SL5_PATH/requestreplydaq pitzdaq3:$SERVER_PATH/pitznoadc0_server/pitznoadc0_server
scp $SL5_PATH/requestreplydaq pitzdaq3:$SERVER_PATH/pitznoadc1_server/pitznoadc1_server
scp $SL5_PATH/requestreplydaq pitzdaq3:$SERVER_PATH/pitzrf1mtca_server/pitzrf1mtca_server
scp $SL5_PATH/requestreplydaq pitzdaq3:$SERVER_PATH/pitzrf2mtca_server/pitzrf2mtca_server
scp $SL5_PATH/requestreplydaq pitzdaq4:$SERVER_PATH/pitznoadc5hz_server/pitznoadc5hz_server
scp $SL5_PATH/requestreplydaq pitzdaq4:$SERVER_PATH/pitzrapdaq_server/pitzrapdaq_server

