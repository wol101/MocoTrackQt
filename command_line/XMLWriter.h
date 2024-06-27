#ifndef XMLWRITER_H
#define XMLWRITER_H

#include <string>
#include <map>

class XMLWriter
{
public:
    XMLWriter();
    XMLWriter(bool initialise);

    void initialiseDocument();
    void estimateSize(size_t size);

    void initiateTag(const std::string &tag, const std::map<std::string, std::string> &attributes = std::map<std::string, std::string>());
    void terminateTag(const std::string &tag);
    void tagAndContent(const std::string &tag, const std::string &content = std::string());
    void tagAndAttributes(const std::string &tag, const std::map<std::string, std::string> &attributes);
    void appendText(const std::string &text);

    int currentIndent() const;
    void setCurrentIndent(int newCurrentIndent);

    const std::string &xmlString() const;
    void setXmlString(const std::string &newXmlString);

private:
    int m_currentIndent = 0;
    std::string m_xmlString;
};

#endif // XMLWRITER_H
