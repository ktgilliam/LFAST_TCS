#include "../00_Utils/karbon_can_bus.h"
#include "../00_Utils/bashWrapper.h"
#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include <sstream>
#include <thread>

// To execute tests:
// cd build && ctest --output-on-failure .
///
/// Support functions ///
///
void removeNewlines(std::string &s)
{
    int pos;
    if ((pos = s.find('\n')) != std::string::npos)
        s.erase(pos);
}

///
/// Karbon CAN Bus unit tests ///
///

TEST(karbon_can_bus_tests, kcb_cnstrctr1)
{
    int testVal = 4;
    KarbonCANBus kcb(testVal);
    // EXPECT_EQ(kcb.getTestVar(), testVal);
    SUCCEED();
}

///
/// Bash wrapper unit tests ///
///

// TEST(bash_wrapper_tests, testExecBashCommand)
// {
//     std::stringstream ss;
//     const std::string bashbw = "echo ";
//     const std::string dblQuote = "\"";
//     std::string message = "This is a test.";

//     ss << bashbw << dblQuote << message << dblQuote; // << std::endl;
//     std::string bashCommandStr = ss.str();
//     ss.str(""); // clear the stringstream

//     ss << message << std::endl;
//     std::string expectedResponseStr = ss.str();
//     ss.str(""); // clear the stringstream

//     std::string receivedResponseStr;
//     int bashExitCode;
//     BashWrapper bw;
//     receivedResponseStr = bw.execBashCommand(bashCommandStr, bashExitCode);

//     EXPECT_STREQ(expectedResponseStr.c_str(), receivedResponseStr.c_str());
// }


TEST(bash_wrapper_tests, testBashCommandWithPipes)
{
    BashWrapper bw;
    bw.Command = "grep wacky - ";
    bw.StdIn = R"(This is a wacky test.)";;
    bw.execBashCommandWithPipes();

    std::cout << "RECEIVED REPLY: " << bw.StdOut << std::endl;
    std::stringstream ss;
    ss << bw.StdOut;
    std::string outString = ss.str();
    removeNewlines(outString);
    ss.str(""); // clear the stringstream
    EXPECT_STREQ(outString.c_str(), bw.StdIn.c_str());
}

TEST(bash_wrapper_tests, testBashWrapperSingleLineRead)
{
    BashWrapper bw;
    bw.Command = "grep wacky - ";
    bw.StdIn = R"(This is a wacky test.)";;
    bw.execBashCommandWithPipes();

    std::cout << "RECEIVED REPLY: " << bw.StdOut << std::endl;
    std::stringstream ss;
    ss << bw.StdOut;
    std::string outString = ss.str();
    removeNewlines(outString);
    ss.str(""); // clear the stringstream
    EXPECT_STREQ(outString.c_str(), bw.StdIn.c_str());
}

// TEST(bash_wrapper_tests, testExecMultiLineStdOut)
// {


//     std::string bwStr = "seq 0 255";

//     // seq 0 255 | while read n; do printf "%04X;" $n; done

//     // bw.Command = "for ii in {1..5}; do echo $ii && sleep 0.1; done";
//     // bw.StdIn = "seq 0 5";
//     // bw.Command = R"(while read -e n; do printf "%04X" $n; done)";
//     // bw.execBashCommandWithPipes();

//     // std::cout << "REPLY LENGTH: " << bw.StdOut.length() <<std::endl;
//     // std::cout << "RECEIVED REPLY: " << bw.StdOut << std::endl;


//     // std::string receivedResponseStr;
//     // int bashExitCode;
//     // BashWrapper bw;
//     // receivedResponseStr = bw.execBashCommand(bwStr, bashExitCode);
//     // std::cout << receivedResponseStr << "################" << std::endl;

//     BashWrapper::CommandWithPipes bw;

//     FAIL();
// }



