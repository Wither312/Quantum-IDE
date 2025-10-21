#define NOMINMAX //TODO ADD IN CMAKELITS
#define CATCH_CONFIG_MAIN  // Only in one CPP file, usually your main test file
#include "Core.hpp" 
#include <catch2/catch_all.hpp>


TEST_CASE("FileSystem creates directories correctly", "[FileSystem]") {
    core::FileSystem fs;
    REQUIRE(fs.createDirectory("TestDir") == true);
    REQUIRE(fs.exists("TestDir") == true);
    // Cleanup if needed
}

TEST_CASE("FileSystem reads and writes files", "[FileSystem]") {
    core::FileSystem fs;
    std::string testFile = "TestDir/test.txt";
    std::string content = "Hello Catch2!";

    REQUIRE(fs.writeFile(testFile, content) == true);
    auto readContent = fs.readFile(testFile);
    REQUIRE(readContent.has_value());
    REQUIRE(readContent.value() == content);
}
