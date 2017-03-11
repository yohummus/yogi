#include "ptree.hpp"

#include <boost/property_tree/json_parser.hpp>
namespace pt = boost::property_tree;


namespace yogi {
namespace internal {
namespace {

template <typename Fn>
void traverse_property_tree(const boost::property_tree::ptree& tree,
    const boost::property_tree::ptree::path_type& parentPath,
    const boost::property_tree::ptree::path_type& path, Fn fn)
{
    bool iterateChildren = fn(tree, parentPath, path);
    if (!iterateChildren) {
        return;
    }

    for (auto& child : tree) {
        auto childPath = path / pt::ptree::path_type(child.first);
        traverse_property_tree(child.second, path, childPath, fn);
    }
}

} // anonymous namespace

void merge_property_trees(boost::property_tree::ptree* dest, const boost::property_tree::ptree& src)
{
    traverse_property_tree(src, "", "", [&](const pt::ptree& child, const pt::ptree::path_type& parentPath, const pt::ptree::path_type& childPath) {
        if (child.data() == "null") {
            auto parentPath = childPath.dump();
            parentPath.erase(parentPath.find_last_of('.')); // TODO

            auto childName = childPath.dump();
            childName = childName.substr(childName.find_last_of('.') + 1);

            dest->get_child(parentPath).erase(childName);
        }
        else {
            bool isRoot  = childPath.empty();
            bool isArray = !!child.count("");

            if (isRoot || !isArray) {
                dest->put(childPath, child.data());
            }
            else {
                auto childName = childPath.dump();
                auto pos = childName.find_last_of('.');
                if (pos != std::string::npos) {
                    childName = childName.substr(pos + 1);
                }
                dest->get_child(parentPath).add_child(childName, child);
                return false;
            }
        }

        return true;
    });
}

boost::property_tree::ptree parse_json_files(const std::vector<std::string>& fileNames)
{
    pt::ptree tree;
    for (auto& fileName : fileNames) {
        pt::ptree partialTree;
        pt::read_json(fileName, partialTree);
        merge_property_trees(&tree, partialTree);
    }

    return tree;
}

} // namespace internal
} // namespace yogi
