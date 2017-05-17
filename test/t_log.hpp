#include <cpptest.hpp>
#include "../log.hpp"

namespace t_log {

bool basic() {
    CASE::Log log{"test1.data"};
    log.out("this is a test");
    return false;
}

bool stream() {
    CASE::Log log{"test2.data"};
    log << "this is another test" << CASE::endl;
    return false;
}

bool streams() {
    CASE::Log log{"test3.data"};
    log << "test one\n" << "test two\n" << "test three\n";
    return false;
}

void run() {
    cpptest::Module test{"log"};
    test.fn("basic", basic);
    test.fn("stream", stream);
    test.fn("several streams", streams);
}

}
