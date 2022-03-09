#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
#ifdef PROJECT_SOURCE_DIR
    cout << PROJECT_SOURCE_DIR << endl;
#endif

    cout << "Hello World" << endl;
    return 0;    
}