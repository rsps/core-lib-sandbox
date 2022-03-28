/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2022 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */

#include "doctest.h"
#include <iostream>
#include <utils/StrUtils.h>
#include <utils/InRange.h>
#include <utils/json/Json.h>

using namespace rsp::utils;
using namespace rsp::utils::json;

TEST_CASE("Json") {

    JsonString json_object{ R"(
{
    "NullValue": null,
    "BooleanValue": true,
    "StringValue": "Hello\tWorld",
    "IntValue": 42,
    "FloatValue": 1.234567,
    "ArrayValue": [
        32,
        "string",
        true,
        null
    ],
    "ObjectMember": {
        "Boolean": false,
        "Empty": null
    }
}    
)" };

    SUBCASE("Create Float") {
        JsonValue v1(1.4f);
        CHECK(v1.GetJsonType() == JsonTypes::Number);
        CHECK(IsEqual(1.4f, static_cast<float>(v1), 0.000001f));
    }

    SUBCASE("Create String") {
        JsonValue v2("My World");
        CHECK(v2.GetType() == JsonValue::Types::String);
        CHECK("My World" == v2.AsString());
        CHECK(StrUtils::Contains(v2.Encode(), "\"My World\""));
        CHECK("\"My World\"" == v2.Encode());
    }

    SUBCASE("Create Unicode String") {
        JsonValue* v3 = JsonString("\"My \\u0057orld\"").GetValue();
        CHECK(v3->GetType() == JsonValue::Types::String);
        CHECK("My World" == v3->AsString());
        delete v3;

        v3 = JsonString("\"Euro sign: \\u20AC\"").GetValue();
//            CHECK("Euro sign: €" == "Euro sign: \\u20AC");
        CHECK(v3->GetType() == JsonValue::Types::String);
        CHECK("Euro sign: €" == v3->AsString());
    }

    SUBCASE("Ignore Whitespace") {
        std::string ws = R"({"whitespace":  

	
null }
)";
        auto p = JsonString(ws).GetValue();
        JsonValue& v4 = *p;

        CHECK(v4.GetJsonType() == JsonTypes::Object);
        CHECK(v4.AsObject().GetCount() == 1);
        CHECK(v4.AsObject()["whitespace"].IsNull());
        MESSAGE(v4.Encode());
        delete p;
    }

    SUBCASE("Decode Object") {
        JsonValue& v = *JsonString(json_object).GetValue();

        CHECK(v.GetJsonType() == JsonTypes::Object);
        CHECK(v.AsObject().GetCount() == 7);

        JsonObject& o = v.AsObject();

        CHECK(o.MemberExists("NullValue"));
        CHECK(o["NullValue"].IsNull());

        CHECK(o.MemberExists("BooleanValue"));
        CHECK(o["BooleanValue"].IsNull() == false);
        CHECK(static_cast<bool>(o["BooleanValue"]) == true);

        CHECK(o.MemberExists("StringValue"));
        CHECK(o["StringValue"].IsNull() == false);

        CHECK(o.MemberExists("IntValue"));
        CHECK(o["IntValue"].IsNull() == false);
        CHECK(o["IntValue"].GetType() == Variant::Types::Int64);
        CHECK(static_cast<int>(o["IntValue"]) == 42);

        CHECK(o.MemberExists("FloatValue"));
        CHECK(o["FloatValue"].IsNull() == false);

        CHECK(o.MemberExists("ArrayValue"));
        JsonArray& a = o["ArrayValue"].AsArray();
        CHECK(a.GetCount() == 4);
        CHECK(a.IsNull() == false);
        CHECK(static_cast<int>(a[0u]) == 32);
        CHECK(a[1u].AsString() == "string");
        CHECK(static_cast<bool>(a[2]));
        CHECK(a[3u].IsNull());

        CHECK(o.MemberExists("ObjectMember"));
        CHECK(o["ObjectMember"].IsNull() == false);
        CHECK(o["ObjectMember"].AsObject().GetCount() == 2);
        CHECK_FALSE(static_cast<bool>(o["ObjectMember"].AsObject()["Boolean"]));
        CHECK(o["ObjectMember"].AsObject()["Empty"].IsNull());

        CHECK_THROWS_AS(o["ObjectMember"].AsObject()["empty"].IsNull(), const EJsonException &);

        o["ObjectMember"].AsObject()["Empty"] = 12.34;
        CHECK_FALSE(o["ObjectMember"].AsObject()["Empty"].IsNull());
        CHECK(IsEqual(static_cast<double>(o["ObjectMember"].AsObject()["Empty"]), 12.34, 0.00000000001));

        delete &v;
    }

    SUBCASE("Encode Object") {
        std::string orig = json_object;
        JsonValue& v = *JsonString(json_object).GetValue();

        CHECK(v.GetJsonType() == JsonTypes::Object);
        CHECK(v.AsObject().GetCount() == 7);

        std::string result = v.Encode(true);

        StrUtils::Trim(result);
        StrUtils::Trim(orig);
        MESSAGE(result.length());
        MESSAGE(orig.length());
        CHECK(result == orig);

        delete &v;

        // Validate UCS2 code-points in output:
        orig = "\"Euro sign: \\u20ac\"";
        JsonValue *v1 = JsonString(orig).GetValue();
        CHECK(v1->GetType() == JsonValue::Types::String);
        CHECK("Euro sign: €" == v1->AsString());
        result = v1->Encode(true, true);
        CHECK(result == orig);

        delete v1;
    }

    SUBCASE("Validate") {
        CHECK_THROWS_AS(JsonString(R"(1.23456.7)").GetValue(), const EJsonNumberError &); // Bad number
        CHECK_THROWS_AS(JsonString(R"(BadString)").GetValue(), const EJsonParseError &);
        CHECK_THROWS_AS(JsonString(R"("Bad Character \k")").GetValue(), const EJsonFormatError &);
        CHECK_THROWS_AS(JsonString(R"(TRUE)").GetValue(), const EJsonParseError &);
        CHECK_NOTHROW(JsonString(R"([   ])").GetValue());
        CHECK_NOTHROW(JsonString(R"([ null ])").GetValue());
        CHECK_THROWS_AS(JsonString(R"([ , ])").GetValue(), const EJsonParseError &);
        CHECK_THROWS_AS(JsonString(R"([ "BadArray", "Excessive Delimiter",])").GetValue(), const EJsonParseError &);
        CHECK_NOTHROW(JsonString(R"({   })").GetValue());
        CHECK_NOTHROW(JsonString(R"({ "empty":null })").GetValue());
        CHECK_THROWS_AS(JsonString(R"({ null })").GetValue(), const EJsonParseError &);
        CHECK_THROWS_AS(JsonString(R"({ , })").GetValue(), const EJsonParseError &);
        CHECK_THROWS_AS(JsonString(R"({ "BadObject": "Excessive Delimiter",})").GetValue(), const EJsonParseError &);
    }

    SUBCASE("Copy") {
        std::string orig = json_object;
        StrUtils::Trim(orig);
        JsonValue* p = JsonString(json_object).GetValue();

        JsonObject dst;
        dst = p->AsObject();

        dst["IntValue"] = 43;

        CHECK(p->IsArray() == dst.IsArray());
        CHECK(p->IsObject() == dst.IsObject());
        CHECK(dst.IsObject() == true);
        CHECK(p->AsObject().GetCount() == dst.GetCount());
        CHECK(p->GetType() == dst.GetType());
        CHECK(&p->AsObject() != &dst);
        CHECK(p->GetJsonTypeAsString() == dst.GetJsonTypeAsString());
        CHECK(p->AsObject()["IntValue"].AsInt() != dst["IntValue"].AsInt());

        JsonObject dst2(p->AsObject());

        dst["IntValue"] = 42;
        CHECK(p->Encode(true) == dst.Encode(true));
        CHECK(p->Encode(true) == dst2.Encode(true));
        CHECK(orig == dst2.Encode(true));
    }

    SUBCASE("Move") {
        std::string orig = json_object;
        StrUtils::Trim(orig);
        JsonValue* p = JsonString(json_object).GetValue();
        JsonObject dst = std::move(p->AsObject());

        CHECK(orig == dst.Encode(true));
        CHECK(p->IsNull());

        JsonObject dst2(std::move(dst));
        CHECK(orig == dst2.Encode(true));
        CHECK(dst.IsNull());
    }

    SUBCASE("Interface") {
        Json js;
        js.Decode(json_object);

        auto &o = js->AsObject();

        CHECK(o.MemberExists("NullValue"));
        CHECK(o["NullValue"].IsNull());

        CHECK(o.MemberExists("BooleanValue"));
        CHECK(o["BooleanValue"].IsNull() == false);
        CHECK(static_cast<bool>(o["BooleanValue"]) == true);

        CHECK(o.MemberExists("StringValue"));
        CHECK(o["StringValue"].IsNull() == false);

        CHECK(o.MemberExists("IntValue"));
        CHECK(o["IntValue"].IsNull() == false);
        CHECK(o["IntValue"].GetType() == Variant::Types::Int64);
        CHECK(static_cast<int>(o["IntValue"]) == 42);

        Json js1(js);
        CHECK(js.Encode() == js1.Encode());
        js1->AsObject()["IntValue"] = 43;

        CHECK(js1->AsObject()["IntValue"].AsInt() == 43);
        CHECK(js->AsObject()["IntValue"].AsInt() == 42);

        Json js2(std::move(js));
        js2->AsObject()["IntValue"] = 43;

        CHECK(js2.Encode() == js1.Encode());
        CHECK(js.Empty());
    }
}

