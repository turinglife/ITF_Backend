//
// ITF Task Daemon, $Date$, $Revision$
// Copyright (C) 2015-2018 MMLab, EE, The Chinese University of HongKong
//

#ifndef ITF_TD_H
#define ITF_TD_H


#include "common.hpp"


class CTD {


public:
    void LoadRemoteTaskConfig();
    void LoadFileTaskConfig();


private:
    typedef struct {
        unsigned int size;
        unsigned int last_write_index;
        unsigned int last_read_index;
    } BufferHead;

    int shm_id;
    BufferHead *pbufferhead_;



};









#endif // ITF_TD_H
