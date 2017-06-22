#ifndef HELPERS_OSTREAM_HPP
#define HELPERS_OSTREAM_HPP

#include <QString>

#include <ostream>


std::ostream& operator<< (std::ostream& os, const QString& str);

#endif // HELPERS_OSTREAM_HPP
