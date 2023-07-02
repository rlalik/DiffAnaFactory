#include <midas.hpp>

#include <TH1.h>

int main()
{
    const midas::context ctx("test", midas::dimension::DIM1);
    ctx.print();
}
