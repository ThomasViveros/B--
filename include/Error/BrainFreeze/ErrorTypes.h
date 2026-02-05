//
// Created by Thomas on 2/3/2026.
//

#ifndef ERRORTYPES_H
#define ERRORTYPES_H

#include "ErrorContext.h"

namespace BrainFreeze {

    class Error {
    public:
        Error() = delete;
        Error(const std::string& Reason, const ErrorContext& Context)
            : Reason(Reason), Context(Context) {}

        virtual ~Error() = default;
        friend std::ostream& operator<<(std::ostream& os, const Error& obj);
    private:
        std::string Reason;
        ErrorContext Context;
    };

    inline std::ostream& operator<<(std::ostream& os, const Error& obj) {
        return os << std::format("SyntaxError: {}: {}", obj.Context, obj.Reason);
    }

    class SyntaxError : public Error {

    };

    class TypeError : public Error {

    };

    class LinkError : public Error {

    };

}


#endif //ERRORTYPES_H
