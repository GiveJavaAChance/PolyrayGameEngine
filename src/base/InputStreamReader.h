#ifndef INPUTSTREAMREADER_H_INCLUDED
#define INPUTSTREAMREADER_H_INCLUDED

#pragma once
#include "InputStream.h"

class InputStreamReader {
public:

    InputStreamReader(InputStream& in) : input(in) {}

private:
    InputStream& input;
};

#endif
