//
// Created by Thomas on 2/3/2026.
//

#ifndef ERRORCONTEXT_H
#define ERRORCONTEXT_H

#include <format>
#include <string>

#include "Core/CoreTypes.h"

namespace BrainFreeze {

class ErrorContext {
public:

    ErrorContext() = delete;
    ErrorContext(const std::string& fileName, const std::string& filePath, int32 lineNumber, int32 columnNumber)
    : FileName(fileName), FilePath(filePath), LineNumber(lineNumber), ColumnNumber(columnNumber){}
    friend std::ostream& operator<<(std::ostream& os, const ErrorContext& obj);

protected:
private:
    std::string FileName;
    std::string FilePath;
    int32 LineNumber = INDEX_NONE;
    int32 ColumnNumber = INDEX_NONE;
};

inline std::ostream& operator<<(std::ostream& os, const ErrorContext& obj) {
    return os << std::format("File {}: Line {} : Char {}", obj.FileName, obj.LineNumber, obj.ColumnNumber);
}

}
#endif //ERRORCONTEXT_H
