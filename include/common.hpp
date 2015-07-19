//
// ITF Common, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_COMMON_H
#define ITF_COMMON_H


#include <iostream>
#include <string>
#include <thread>
#include <map>
#include <vector>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

#include "SQLiteCpp.h"


#define IN        // Input parameters
#define OUT       // Output parameters


// Instantiate a class with float and cv::Mat specifications.
#define INSTANTIATE_MYCLASS(classname) \
  template class classname<float>

  
  
  
  
  
//template class classname<double>






#endif
