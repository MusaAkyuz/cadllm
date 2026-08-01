#include <iostream>

#include "version.h"

int main()
{
    std::cout << "cadllm " << appVersion() << std::endl;
    return 0;
}
