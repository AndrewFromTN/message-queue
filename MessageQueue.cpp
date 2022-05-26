#include <iostream>
#include "zmq.hpp"

int main (int argc, char** argv)
{
    zmq::context_t context(1);
    zmq::socket_t sub(context, ZMQ_XSUB);
    sub.bind("tcp://*:5556");

    zmq::socket_t pub(context, ZMQ_XPUB);
    pub.bind("tcp://*:5557");

    zmq_proxy((void*)sub, (void*)pub, nullptr);

    return 0;
}