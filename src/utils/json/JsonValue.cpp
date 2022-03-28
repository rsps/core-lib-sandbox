/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2022 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */

#include <iomanip>
#include <utils/json/JsonValue.h>
#include <utils/json/JsonArray.h>
#include <utils/json/JsonObject.h>
#include <logging/Logger.h>
#include <utils/StrUtils.h>
#include <utils/json/JsonExceptions.h>

namespace rsp::utils::json {

//#define JLOG(a) DLOG(a)
#define JLOG(a)


JsonValue::JsonValue(const JsonValue& arOther)
    : Variant(static_cast<const Variant&>(arOther))
{
    JLOG("JsonValue copy constructor");
}

JsonValue::JsonValue(const JsonValue&& arOther)
    : Variant(static_cast<const Variant&>(arOther))
{
    JLOG("JsonValue move constructor");
}

JsonValue::~JsonValue()
{
}

JsonValue& JsonValue::operator=(const JsonValue& arOther)
{
    JLOG("JsonValue copy assignment");
    if (&arOther != this) {
        Variant::operator=(static_cast<const Variant&>(arOther));
    }
    return *this;
}

JsonValue* JsonValue::clone() const
{
    JLOG("JsonValue::clone");
    auto result = new JsonValue();
    *result = *this;
    return result;
}

JsonValue& JsonValue::operator=(const JsonValue&& arOther)
{
    JLOG("JsonValue move assignment");
    if (&arOther != this) {
        Variant::operator=(static_cast<const Variant&&>(arOther));
    }
    return *this;
}


std::string JsonValue::Encode(bool aPrettyPrint, bool aForceToUCS2)
{
    std::stringstream result;

    if (aPrettyPrint) {
        PrintFormat pf(4, "\n", " ");
        toStringStream(result, pf, 0, aForceToUCS2);
    }
    else {
        PrintFormat pf;
        toStringStream(result, pf, 0, aForceToUCS2);
    }

    return result.str();
}


JsonArray& JsonValue::AsArray() const
{
    if (mType == Types::Pointer && mPointer == static_cast<uintptr_t>(JsonTypes::Array)) {
        return *dynamic_cast<JsonArray*>(const_cast<JsonValue*>(this));
    }
    else {
        THROW_WITH_BACKTRACE1(EJsonTypeError, "JsonValue of type " + GetJsonTypeAsString() + " cannot be converted to Array");
    }
}

JsonObject& JsonValue::AsObject() const
{
    if (mType == Types::Pointer && mPointer == static_cast<uintptr_t>(JsonTypes::Object)) {
        return *dynamic_cast<JsonObject*>(const_cast<JsonValue*>(this));
    }
    else {
        THROW_WITH_BACKTRACE1(EJsonTypeError, "JsonValue of type " + GetJsonTypeAsString() + " cannot be converted to Object");
    }
}

void JsonValue::toStringStream(std::stringstream &arResult, PrintFormat &arPf, unsigned int aLevel, bool aForceToUCS2)
{
    std::string in(static_cast<std::string::size_type>(arPf.indent) * aLevel, ' ');

    if (mType == Types::String) {
        std::string s = AsString();
        std::size_t i = 0;
        while (i < s.length()) {
            auto c = s[i];
            switch (c) {
                case '\"':
                    s.replace(i, 1, "\\\"");
                    break;
                case '\\':
                    s.replace(i, 1, "\\\\");
                    break;
                case '\b':
                    s.replace(i, 1, "\\b");
                    break;
                case '\f':
                    s.replace(i, 1, "\\f");
                    break;
                case '\n':
                    s.replace(i, 1, "\\n");
                    break;
                case '\r':
                    s.replace(i, 1, "\\r");
                    break;
                case '\t':
                    s.replace(i, 1, "\\t");
                    break;
                default:
                    if (aForceToUCS2 && static_cast<uint8_t>(c) > 127) {
                        int v = 0;
                        char buf[12];
                        switch (static_cast<uint8_t>(c) & 0xE0) {
                            case 0xE0:
                                v =   ((static_cast<int>(s[i]) & 0x0F) << 12)
                                    + ((static_cast<int>(s[i+1]) & 0x3F) << 6)
                                    + (static_cast<int>(s[i+2]) & 0x3F);
                                sprintf(buf, "\\u%04x", v);
                                s.replace(i, 3, buf);
                                i += 4;
                                break;

                            case 0xC0:
                                v =   ((static_cast<int>(s[i]) & 0x1F) << 6)
                                    + (static_cast<int>(s[i+1]) & 0x3F);
                                sprintf(buf, "\\u%04x", v);
                                s.replace(i, 2, buf);
                                i += 4;
                                break;

                            default:
                                THROW_WITH_BACKTRACE1(EJsonParseError, "JsonValue of type string has illegal character: " + c);
                                break;
                        }
                    }
                    break;
            }
            i++;
        }
        arResult << "\"" << s << "\"";
//        DLOG("toStringSteam: " << s);
//        for (auto c : s) {
//            DLOG("toStringSteam: " << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(c) << std::dec << " " << c);
//        }
    }
    else {
        arResult << AsString();
    }
}

std::string JsonValue::GetJsonTypeAsString() const
{
    switch (GetJsonType()) {
        case JsonTypes::Array: return "Array";
        case JsonTypes::Bool: return "Bool";
        default:
        case JsonTypes::Null: return "Null";
        case JsonTypes::Number: return "Number";
        case JsonTypes::Object: return "Object";
        case JsonTypes::String: return "String";
    }
}

JsonTypes JsonValue::GetJsonType() const
{
    switch (mType) {
        default:
        case Types::Null:
            return JsonTypes::Null;
        case Types::Bool:
            return JsonTypes::Bool;
        case Types::Int:
        case Types::Int64:
        case Types::Uint32:
        case Types::Float:
        case Types::Double:
            return JsonTypes::Number;
        case Types::Pointer:
            if (mPointer == static_cast<uintptr_t>(JsonTypes::Object)) {
                return JsonTypes::Object;
            }
            if (mPointer == static_cast<uintptr_t>(JsonTypes::Array)) {
                return JsonTypes::Array;
            }
            THROW_WITH_BACKTRACE1(EJsonTypeError, "Variant of type " + typeToText() + " cannot be converted to JsonTypes Object or Array");
            break;
    }
}

} // namespace rsp::utils::json

