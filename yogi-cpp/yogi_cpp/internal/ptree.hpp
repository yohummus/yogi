#ifndef YOGICPP_INTERNAL_PTREE_HPP
#define YOGICPP_INTERNAL_PTREE_HPP

#include <boost/property_tree/ptree.hpp>

#include <vector>
#include <string>


namespace yogi {
namespace internal {

void merge_property_trees(boost::property_tree::ptree* dest, const boost::property_tree::ptree& src);
boost::property_tree::ptree parse_json_files(const std::vector<std::string>& filenames);

} // namespace internal
} // namespace yogi

#endif // YOGICPP_INTERNAL_PTREE_HPP
