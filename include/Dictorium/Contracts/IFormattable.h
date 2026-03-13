#ifndef IFORMATTABLE_H
#define IFORMATTABLE_H

#include <string>
#include <sstream>

namespace dtr {

class IFormattable {
public:
    virtual ~IFormattable() = default;
    virtual void WriteToStream(std::ostream& os) const = 0;

    virtual std::string ToString() {
        std::string s;
        std::ostringstream stringStream(s);

        WriteToStream(stringStream);
        return s;
    }
};

inline std::ostream& operator<<(std::ostream& os, const IFormattable& obj) {
    obj.WriteToStream(os);
    return os;
}

}

#endif //IFORMATTABLE_H
