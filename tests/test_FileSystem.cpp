#define NOMINMAX
#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "Core.hpp" // Assuming FileSystem is declared here
#include <fstream>

namespace fs = std::filesystem;

TEST_CASE("FileSystem creates and removes directories", "[FileSystem]") {
    core::FileSystem fs;
    std::string dir = "TestDir";

    SECTION("Create directory") {
        REQUIRE(core::FileSystem::createDirectory(dir));
        REQUIRE(core::FileSystem::exists(dir));
    }

    SECTION("Remove directory") {
        core::FileSystem::createDirectory(dir);
        REQUIRE(core::FileSystem::remove(dir));
        REQUIRE_FALSE(core::FileSystem::exists(dir));
    }
}

TEST_CASE("FileSystem reads and writes files", "[FileSystem]") {
    std::string dir = "TestDir";
    std::string file = dir + "/test.txt";
    std::string content = "Hello Catch2!";

    core::FileSystem::createDirectory(dir);

    SECTION("Write file") {
        REQUIRE(core::FileSystem::writeFile(file, content));
        REQUIRE(core::FileSystem::exists(file));
    }

    SECTION("Read file") {
        core::FileSystem::writeFile(file, content);
        auto result = core::FileSystem::readFile(file);
        REQUIRE(result.has_value());
        REQUIRE(result.value() == content);
    }

    // Cleanup
    core::FileSystem::remove(dir);
}

TEST_CASE("FileSystem listFiles works", "[FileSystem]") {
    const std::string dir = "ListTestDir";
    const std::string file1 = dir + "/a.txt";
    const std::string file2 = dir + "/b.txt";

    // Ensure test directory exists
    REQUIRE(core::FileSystem::createDirectory(dir));

    // Create two files with different content
    REQUIRE(core::FileSystem::writeFile(file1, "File A"));
    REQUIRE(core::FileSystem::writeFile(file2, "File B"));

    // List files in directory (non-recursive)
    auto files = core::FileSystem::listFiles(dir);
    REQUIRE(files.size() == 2);

    // Extract just the filenames
    std::vector<std::string> found;
    found.reserve(files.size());
    std::transform(files.begin(), files.end(), std::back_inserter(found),
                   [](const std::filesystem::path& p) { return p.filename().string(); });

    // Check if both expected filenames are found, order doesn't matter
   // REQUIRE_THAT(found, Catch::Matchers::UnorderedEquals({"a.txt", "b.txt"})); TODO Fix missmatched arguments?

    // Cleanup test directory and contents
    REQUIRE(core::FileSystem::remove(dir));
}

TEST_CASE("FileSystem exists handles files and folders", "[FileSystem]") {
    std::string dir = "ExistTestDir";
    std::string file = dir + "/file.txt";

    REQUIRE_FALSE(core::FileSystem::exists(dir));
    REQUIRE(core::FileSystem::createDirectory(dir));
    REQUIRE(core::FileSystem::exists(dir));

    REQUIRE(core::FileSystem::writeFile(file, "data"));
    REQUIRE(core::FileSystem::exists(file));

    // Cleanup
    core::FileSystem::remove(dir);
}

TEST_CASE("FileSystem handles invalid read/write safely", "[FileSystem]") {
    std::string invalidFile = "/root/forbidden.txt"; // assuming this will fail on most systems

    auto result = core::FileSystem::readFile(invalidFile);
    REQUIRE_FALSE(result.has_value());

    bool writeResult = core::FileSystem::writeFile(invalidFile, "Should fail");
    REQUIRE_FALSE(writeResult);
}
