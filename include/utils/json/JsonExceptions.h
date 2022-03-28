/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2022 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */
#ifndef INCLUDE_UTILS_JSON_JSONEXCEPTIONS_H_
#define INCLUDE_UTILS_JSON_JSONEXCEPTIONS_H_

#include <utils/CoreException.h>

namespace rsp::utils::json {

/**
 * \class EJsonException
 * \brief Base class for all exceptions thrown by the json module.
 */
class EJsonException : public rsp::utils::CoreException {
public:
    explicit EJsonException(const std::string &aMsg) : rsp::utils::CoreException(aMsg) {}
};

class EJsonParseError : public EJsonException {
public:
    explicit EJsonParseError(const std::string &aMsg) : EJsonException("Json Parse Error: " + aMsg) {}
};

class EJsonFormatError : public EJsonException {
public:
    explicit EJsonFormatError(const std::string &aMsg) : EJsonException("Json Format Error: " + aMsg) {}
};

class EJsonNumberError : public EJsonException {
public:
    explicit EJsonNumberError(const std::string &aMsg) : EJsonException("Json Number Error: " + aMsg) {}
};

class EJsonTypeError : public EJsonException {
public:
    explicit EJsonTypeError(const std::string &aMsg) : EJsonException("Json Type Error: " + aMsg) {}
};

class EInstanceExists: public EJsonException {
public:
    explicit EInstanceExists() : EJsonException("The Json content have already been created.") {}
};

class ENoInstanceExists: public EJsonException {
public:
    explicit ENoInstanceExists() : EJsonException("The Json content have not been created.") {}
};


} /* namespace rsp::utils::json */

#endif /* INCLUDE_UTILS_JSON_JSONEXCEPTIONS_H_ */
