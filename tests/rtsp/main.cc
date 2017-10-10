#include "test_client.h"
#include <swift/thread/engine_runner.hpp>
#include <stdio.h>
int main()
{
    swift::engine eng;
    swift::engine_runner runner(eng);
    runner.start();

    MyRtspClient *client = new MyRtspClient(eng);
    client->start();

    getchar();
    client->stop();

    return 0;
}
