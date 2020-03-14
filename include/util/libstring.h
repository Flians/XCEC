#ifndef _LIBSTRING_H_
#define _LIBSTRING_H_

#include "libhead.h"

class libstring
{
private:
    /* data */
public:
    libstring(/* args */);
    ~libstring();

    string trim(const string& str);
    void split(const string& str, vector<string>& ret_, string sep);
    string replace(const string& str, const string& src, const string& dest);
};

#endif
