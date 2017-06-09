#ifndef TEMPLATE_STRING_HH
#define TEMPLATE_STRING_HH

#include <string>
#include <vector>
#include <ostream>
#include <memory>


class TemplateString
{
public:
    TemplateString();
    TemplateString(std::string value);
    TemplateString(std::string name, std::string value);

    std::string name() const;
    std::string value() const;

    void resolve(const TemplateString& other);
    void resolve(const std::vector<TemplateString>& others);

private:
    std::string m_name;
    std::string m_value;
};


typedef std::vector<TemplateString> template_string_vector;
typedef std::shared_ptr<template_string_vector> template_string_vector_ptr;

std::ostream& operator<< (std::ostream& os, const TemplateString& ts);

#endif // TEMPLATE_STRING_HH
