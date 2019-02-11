/*
 * File:   main.cc
 * Author: bagrat
 *
 * Created on 20. Juni 2014, 14:08
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <common/zmqsubscriber.hpp>
#include <stdint.h>
#include <stdio.h>

//using namespace std;
extern std::string currentTime();

const int16_t GROUPSIZE = 24;
const int16_t GROUP_COUNT = 6;

    typedef struct {
        uint32_t statusBits;
        float value[GROUPSIZE];
        uint32_t dummy[400];
    } SignalGroup;

    typedef struct {
        uint32_t increment;
        uint32_t groupSize;
        uint32_t groupNum;
        SignalGroup group[GROUP_COUNT];

        void print() {
            std::cout << "id = " << increment << ", groupsize = " << groupSize << ", groupnumber = " << groupNum << '\n';
            for (int i(0); i < GROUP_COUNT; ++i) {
                std::cout << "group " << i << ": status = " << group[i].statusBits << "\n\t";
                for (int j(0); j < GROUPSIZE; ++j) {
                    std::cout << "val[" << j << "]=" << std::setprecision(3) << group[i].value[j] << ' ';
                    if ((j + 1) % 6 == 0) std::cout << "\n\t";
                }
                std::cout << std::endl;
            }
        }

        void setTestValues(int counter) { // for debug
            for (int i(0); i < GROUP_COUNT; ++i) {
                group[i].statusBits = counter + i;
                for (int j(0); j < GROUPSIZE; ++j) {
                    group[i].value[j] = 0.1 * j;
                }
            }
        }

        void init() { // for debug
            for (int i(0); i < GROUP_COUNT; ++i) {
                group[i].statusBits = 0;
                for (int j(0); j < GROUPSIZE; ++j) {
                    group[i].value[j] = 0.0;
                }
            }
        }
    } IntlkData;

int main(int argc, char** ) {

    if (argc == 1) {
        printf("server: \n");
        std::cout<<"start as server"<<std::endl;
        IntlkData data;
        printf("size of data = %d\n", (int)sizeof (IntlkData));

        try {
            DataSender ds("epgm://238.2.30.61:7764");
            nanoWait jam;
            int counter = 0;
            while (1) {
                data.setTestValues(++counter);
                ds.sendData(&data, sizeof(IntlkData));
                jam.WAIT(0.1);
            }
        } catch (char const *msg) {
            printf("%s\n", msg);
        }
        return 0;
    } else {
        std::cout<<"start as client"<<std::endl;
        IntlkData data;
        uint32_t errors = 0;
        uint32_t events = 0;
        try {
            DataReceiver ds("epgm://238.2.30.61:7764");
            uint32_t oldEvent=0;
            while (1) {
                ds.receiveData(&data, sizeof(IntlkData));
                ++events;
                //data.print();
                if(oldEvent != data.increment - 1) {
                    std::cerr<<currentTime()<<" old event = "<<oldEvent<<", new event = "<< data.increment<<std::endl;
                    ++errors;
                }
                std::cout << "received " << sizeof (IntlkData) << " with ID " << data.increment << ": happend " << errors << " by receiving " << events << " datagrams\r" << std::flush;
                oldEvent = data.increment;
                data.init();
            }
        } catch (char const *msg) {
            std::cerr<<msg<<std::endl;
            ++errors;
        }
        return 0;
    }
}

