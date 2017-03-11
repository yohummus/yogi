#include "glob.hpp"
#include "../configuration.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include <vector>
#include <algorithm>
#include <regex>


namespace yogi {
namespace internal {
namespace {

std::vector<std::string> split_pattern_by_directory_and_simplify(const std::string& pattern)
{
    std::vector<std::string> parts;
    boost::split(parts, pattern, boost::is_any_of("/\\"));

    std::vector<std::string> simplifiedParts;
    for (auto& part : parts) {
        if (part == ".") {
            continue;
        }

        bool previousPartWasDir = !simplifiedParts.empty() && (simplifiedParts.back() != "..");
        if ((part == "..") && previousPartWasDir) {
            simplifiedParts.pop_back();
        }
        else {
            simplifiedParts.push_back(part);
        }
    }

    return simplifiedParts;
}

std::regex glob_expression_to_regex(std::string pattern)
{
    boost::replace_all(pattern, "\\", "\\\\");
    boost::replace_all(pattern, "^", "\\^");
    boost::replace_all(pattern, ".", "\\.");
    boost::replace_all(pattern, "$", "\\$");
    boost::replace_all(pattern, "|", "\\|");
    boost::replace_all(pattern, "(", "\\(");
    boost::replace_all(pattern, ")", "\\)");
    boost::replace_all(pattern, "[", "\\[");
    boost::replace_all(pattern, "]", "\\]");
    boost::replace_all(pattern, "*", "\\*");
    boost::replace_all(pattern, "+", "\\+");
    boost::replace_all(pattern, "?", "\\?");
    boost::replace_all(pattern, "/", "\\/");
    boost::replace_all(pattern, "\\?", ".");
    boost::replace_all(pattern, "\\*", ".*");

    return std::regex(pattern);
}

void recursively_find_files(std::vector<std::string>* filenames, const fs::path& path, std::vector<std::string> patternParts)
{
    std::regex regex(glob_expression_to_regex(patternParts.front()));

    for (fs::directory_iterator it(path); it != fs::directory_iterator(); ++it) {
        auto filename = it->path().filename();
        if (std::regex_match(filename.string(), regex)) {
            if (patternParts.size() == 1 && fs::is_regular_file(it->path())) {
                filenames->push_back(it->path().string());
            }
            else if (fs::is_directory(it->path()) && patternParts.size() > 1) {
                auto childPath         = path / filename;
                auto childPatternParts = std::vector<std::string>(patternParts.begin() + 1, patternParts.end());
                recursively_find_files(filenames, childPath, childPatternParts);
            }
        }
    }
}

std::vector<std::string> glob_single_pattern(const std::string& pattern)
{
    std::vector<std::string> filenames;

    auto patternParts = split_pattern_by_directory_and_simplify(pattern);
    fs::path startPath;

    // we got an absolute path
    if (patternParts.front().empty()) {
        startPath = "/";
        patternParts.erase(patternParts.begin());
    }
    // we got a drive letter (Windows) e.g. C:
    else if (patternParts.front().size() == 2 && patternParts.front()[1] == ':') {
        startPath = patternParts.front() + "\\";
        patternParts.erase(patternParts.begin());
    }
    // .. or a file/directory
    else {
        while (patternParts.front() == "..") {
            startPath /= "..";
            patternParts.erase(patternParts.begin());
        }

        if (startPath.empty()) {
            startPath = ".";
        }
    }

    // take a shortcut by going to the deepest directory directly if possible
    while (!patternParts.empty() && fs::is_directory(startPath / patternParts.front())) {
        startPath /= patternParts.front();
        patternParts.erase(patternParts.begin());
    }

    recursively_find_files(&filenames, startPath, patternParts);

    return filenames;
}

void merge_vectors(std::vector<std::string>* filenames, const std::vector<std::string>& newFilenames)
{
    for (auto& filename : newFilenames) {
        auto it = std::find(filenames->begin(), filenames->end(), filename);
        if (it != filenames->end()) {
            // move filename to end of vector
            std::rotate(it, it + 1, filenames->end());
        }
        else {
            filenames->push_back(filename);
        }
    }
}

} // anonymous namespace

std::vector<std::string> glob(const std::vector<std::string>& patterns)
{
    std::vector<std::string> filenames;

    for (auto& pattern : patterns) {
        auto newFilenames = glob_single_pattern(pattern);
        if (newFilenames.empty()) {
            throw BadConfigurationFilePattern(pattern);
        }

        merge_vectors(&filenames, newFilenames);
    }

    return filenames;
}

} // namespace internal
} // namespace yogi
