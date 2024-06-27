#include "XMLWriter.h"

#include <iostream>

using namespace std::literals;

XMLWriter::XMLWriter()
{
}

XMLWriter::XMLWriter(bool initialise)
{
    if (initialise) initialiseDocument();
}

void XMLWriter::initialiseDocument()
{
    m_currentIndent = 0;
    m_xmlString = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
}

void XMLWriter::estimateSize(size_t size)
{
    m_xmlString.reserve(size);
}

void XMLWriter::initiateTag(const std::string &tag, const std::map<std::string, std::string> &attributes)
{
    for (int i = 0; i < m_currentIndent; i++) { m_xmlString.push_back(' '); }
    m_xmlString.append("<"s + tag + " "s);
    for (auto &&iter : attributes)
    {
        m_xmlString.append(iter.first + "=\""s + iter.second + "\" "s);
    }
    m_xmlString.pop_back();
    m_xmlString.append(">\n"s);
    m_currentIndent += 2;
}

void XMLWriter::terminateTag(const std::string &tag)
{
    m_currentIndent -= 2;
    if (m_currentIndent < 0)
    {
        std::cerr << "XMLWriter::XMLTerminateTag error: indent < 0 tag=\"" << "\"\n";
        m_currentIndent = 0;
    }
    else
    {
        for (int i = 0; i < m_currentIndent; i++) { m_xmlString.push_back(' '); }
    }
    m_xmlString.append("</"s + tag + ">\n"s);
}

void XMLWriter::tagAndContent(const std::string &tag, const std::string &content)
{
    for (int i = 0; i < m_currentIndent; i++) { m_xmlString.push_back(' '); }
    if (content.size() > 0)
    {
        m_xmlString.append("<"s + tag + ">"s);
        m_xmlString.append(content);
        m_xmlString.append("</"s + tag + ">\n"s);
    }
    else
    {
        m_xmlString.append("<"s + tag + "/>\n"s);
    }
}

void XMLWriter::tagAndAttributes(const std::string &tag, const std::map<std::string, std::string> &attributes)
{
    for (int i = 0; i < m_currentIndent; i++) { m_xmlString.push_back(' '); }
    m_xmlString.append("<"s + tag + " "s);
    for (auto &&iter : attributes)
    {
        m_xmlString.append(iter.first + "=\""s + iter.second + "\" "s);
    }
    m_xmlString.pop_back();
    m_xmlString.append("/>\n"s);
}

void XMLWriter::appendText(const std::string &text)
{
    m_xmlString.append(text);
}

int XMLWriter::currentIndent() const
{
    return m_currentIndent;
}

void XMLWriter::setCurrentIndent(int newCurrentIndent)
{
    m_currentIndent = newCurrentIndent;
}

const std::string &XMLWriter::xmlString() const
{
    return m_xmlString;
}

void XMLWriter::setXmlString(const std::string &newXmlString)
{
    m_xmlString = newXmlString;
}
