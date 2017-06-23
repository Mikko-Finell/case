#include <cpptest.hpp>
#include "../log.hpp"
#include "../timer.hpp"

namespace t_log {

bool basic() {
    CASE::Log log{"test1.data"};
    log.out("this is a test");
    return false;
}

bool stream() {
    CASE::Log log{"test2.data"};
    log << "this is another test" << '\n';
    return false;
}

bool streams() {
    CASE::Log log{"test3.data"};
    log << "test one\n" << "test two\n" << "test three\n";
    return false;
}

void run() {
    CASE::Log log{"test.dat"};
    CASE::Timer timer;
    for (auto i = 0; i < 1000 * 1000; i++)
        log.out(i * i);
    std::cout << timer.dt() << std::endl;

    /*
    cpptest::Module test{"log"};
    test.fn("basic", basic);
    test.fn("stream", stream);
    test.fn("several streams", streams);
    */
}

}
