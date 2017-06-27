#ifndef HELPERS_OSTREAM_HH
#define HELPERS_OSTREAM_HH

#include <QString>

#include <ostream>


std::ostream& operator<< (std::ostream& os, const QString& str);

#endif // HELPERS_OSTREAM_HH
