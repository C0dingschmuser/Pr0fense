#ifndef PATH_H
#define PATH_H
#include <QString>

class Path
{
public:
    Path();
    std::vector <int> path;
    int getNext(int cpos);
    bool active = false;
};

#endif // PATH_H
