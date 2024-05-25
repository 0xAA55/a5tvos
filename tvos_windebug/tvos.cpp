
#include <graphics.hpp>
#include "dibwin.hpp"

#include <iostream>

using namespace TVOS;
using namespace DIBWin;

class MyTestApp :
    public Window,
    public Graphics
{
public:
    MyTestApp();
    MyTestApp(int Width, int Height);
};

int main()
{
    
}
