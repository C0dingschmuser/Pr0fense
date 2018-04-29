#include "path.h"

Path::Path()
{

}

int Path::getNext(int cpos)
{
    int next = -1;
    for(uint i=0;i<path.size();i++) {
        if(cpos==path[i]) {
            next = i+1;
        }
    }
    return next;
}
