/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       pitz_daq_root_for_matlab_false.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   To be done \n
 *
 */

#ifndef PITZ_DAQ_ROOT_FOR_MATLAB_FALSE_HPP
#define PITZ_DAQ_ROOT_FOR_MATLAB_FALSE_HPP

#include<stdint.h>
#include <matrix.h>

extern "C"  mxArray*  MatlabGetMultipleBranchesForTimeInterval2(int a_argc, char* a_argv[]);


#endif
