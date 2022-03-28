/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2022 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */


#include <cstdlib>
#include <algorithm>
#include <memory>
#include <logging/Logger.h>
#include <utils/StrUtils.h>
#include <utils/json/Json.h>
#include <utils/json/JsonString.h>

namespace rsp::utils::json {

Json::Json(const Json &arOther)
    : mpValue{}
{
    if (arOther.mpValue) {
        mpValue = arOther.mpValue->clone();
    }
}

Json::Json(Json &&arOther)
    : mpValue{arOther.mpValue}
{
    arOther.mpValue = nullptr;
    arOther.Clear();
}

Json::~Json()
{
    Clear();
}

Json& Json::Clear()
{
    if (mpValue) {
        delete mpValue;
        mpValue = nullptr;
    }
    return *this;
}


Json& Json::operator=(const Json &arOther)
{
    Clear();
    if (arOther.mpValue) {
        mpValue = arOther.mpValue->clone();
    }
    return *this;
}

Json& Json::operator=(Json &&arOther)
{
    Clear();
    mpValue = arOther.mpValue;
    arOther.mpValue = nullptr;
    arOther.Clear();
    return *this;
}

JsonObject& Json::MakeObject()
{
    if (mpValue) {
        THROW_WITH_BACKTRACE(EInstanceExists);
    }

    mpValue = new JsonObject();
    return *static_cast<JsonObject*>(mpValue);
}

JsonArray& Json::MakeArray()
{
    if (mpValue) {
        THROW_WITH_BACKTRACE(EInstanceExists);
    }

    mpValue = new JsonArray();
    return *static_cast<JsonArray*>(mpValue);
}

void Json::Decode(const std::string &arJson)
{
    JsonString js(arJson);
    mpValue = js.GetValue();
}

std::string Json::Encode(bool aPrettyPrint) const
{
    if (!mpValue) {
        THROW_WITH_BACKTRACE(ENoInstanceExists);
    }

    return mpValue->Encode(aPrettyPrint);
}

JsonValue& Json::Get() const
{
    if (!mpValue) {
        THROW_WITH_BACKTRACE(ENoInstanceExists);
    }

    return *mpValue;
}

JsonValue& Json::operator *()
{
    if (!mpValue) {
        THROW_WITH_BACKTRACE(ENoInstanceExists);
    }

    return *mpValue;
}

JsonValue* Json::operator ->()
{
    if (!mpValue) {
        THROW_WITH_BACKTRACE(ENoInstanceExists);
    }

    return mpValue;
}

} /* namespace rsp::utils::json */

