#include "ostream.hh"


std::ostream& operator<< (std::ostream& os, const QString& str)
{
    return os << str.toStdString();
}
