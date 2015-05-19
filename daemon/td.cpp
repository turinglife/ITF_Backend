//
// ITF Task Daemon, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//


#include "task.hpp"
#include "buffer.hpp"



int main(int argc, char *argv[]) {
    
    unsigned int task_id = 0;
    
    // Check the input parameters
    
    
    // Init logger
    
    
    // Search database and check whether table exists according to task id.
    
    
    // Initialize a task object acccording to information retrieved from database. 
    CTask task;
    task.LoadTask(task_id);
    
    // Capture frames
    task.Capture();
    
    
    
    
    

    return 0;
}
