//
// Created by Thomas on 2/3/2026.
//

#ifndef ERRORTYPES_H
#define ERRORTYPES_H

#include <iostream>
#include <string>
#include <ostream>
#include "ErrorContext.h"


namespace BrainFreeze {

    class Error {
    public:
        Error() = delete;
        Error(const std::string& Reason, const ErrorContext& Context)
            : Reason(Reason), Context(Context) {}

        virtual ~Error() = default;

    protected:
        std::string ErrorTypeStringRepresentation = "Error";
    private:
        std::string Reason;
        ErrorContext Context;
        friend std::ostream& operator<<(std::ostream& os, const Error& obj);
    };

    inline std::ostream& operator<<(std::ostream& os, const Error& obj) {
        return os << obj.ErrorTypeStringRepresentation << ": " << obj.Context << ": " << obj.Reason;
    }

    class RuntimeError : public Error {
    public:
        RuntimeError() = delete;
        RuntimeError(const std::string& Reason, const ErrorContext& Context) : Error(Reason, Context) {
            ErrorTypeStringRepresentation = "RuntimeError";
        }
    };

    class SyntaxError : public Error {
    public:
        SyntaxError() = delete;
        SyntaxError(const std::string& Reason, const ErrorContext& Context) : Error(Reason, Context) {
            ErrorTypeStringRepresentation = "SyntaxError";
        }
    };

    class TypeError : public Error {

    };

    class LinkError : public Error {

    };

}


#endif //ERRORTYPES_H
