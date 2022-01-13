/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2021 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */

#include <iostream>
#include <sstream>
#include <map>
#include <logging/Logger.h>

namespace rsp::logging {


LoggerInterface::Handle_t LoggerInterface::AddLogWriter(std::shared_ptr<LogWriterInterface> aWriter)
{
    mWriters.push_back(aWriter);
    return reinterpret_cast<Handle_t>(aWriter.get());
}

void LoggerInterface::RemoveLogWriter(Handle_t aHandle)
{
    if (aHandle == 0) {
        return;
    }

    auto it = std::find_if(mWriters.begin(), mWriters.end(), [&](std::shared_ptr<LogWriterInterface> const& aWriter) {
        return aHandle == reinterpret_cast<Handle_t>(aWriter.get());
    });
    if (it != mWriters.end()) {
        mWriters.erase(it);
    }
}

bool LoggerInterface::HasWriters() const
{
    return !mWriters.empty();
}

void LoggerInterface::write(const LogStreamInterface *apStream, const std::string &arMsg)
{
    LogLevel current_level = apStream->GetLevel();
    std::lock_guard<std::mutex> lock(mMutex);

    for (auto w : mWriters) {
        w->Write(arMsg, current_level);
    }
}


LogStreamInterface::LogStreamInterface(LoggerInterface *apOwner, LogLevel aLevel)
    : mpOwner(apOwner),
      mLevel(aLevel)
{
}

void LogStreamInterface::ownerWrite(const std::string &arMsg)
{
    mpOwner->write(this, arMsg);
}


LogStream::LogStream(LoggerInterface *apOwner, LogLevel aLevel)
    : LogStreamInterface(apOwner, aLevel)
{
}

LogStream::LogStream(LogStream &&aFrom)
    : LogStreamInterface(aFrom.mpOwner, aFrom.mLevel)
{
    mBuffer = std::move(aFrom.mBuffer);
}

LogStream::~LogStream()
{
    Flush();
}

void LogStream::Flush()
{
    if (mBuffer.rdbuf()->in_avail() > 0) {
        ownerWrite(mBuffer.str());
        mBuffer.str("");
    }
}


OutStreamBuf::OutStreamBuf(LoggerInterface *apOwner, LogLevel aLevel)
    : std::streambuf(),
      LogStreamInterface(apOwner, aLevel)
{
}

int OutStreamBuf::overflow(int c)
{
    if (c != EOF) {
        mBuffer += static_cast<char>(c);
    }
    else {
        mBuffer += '#';
        sync();
    }

    return c;
}

int OutStreamBuf::sync()
{
    if (mMutex.try_lock()) {
//        std::cerr << "OutStreamBuf mutex was not locked!!! " << std::this_thread::get_id() << std::endl;
    }

    int l = mBuffer.length();
    if (l > 0) {
//        rsp::Backtrace::Print();
//        std::cout << "Message: (" << l << ") " << mBuffer << std::endl;
        ownerWrite(mBuffer);
        mBuffer = ""; //.erase();
    }
    mMutex.unlock();
//    std::cout << "Unlocked by " << std::this_thread::get_id() << std::endl;
    return 0;
}

std::ostream& operator <<(std::ostream &os, LogLevel aLevel)
{
    OutStreamBuf *stream = static_cast<OutStreamBuf *>(os.rdbuf());

    stream->Lock();
//    std::cout << "Locked by " << std::this_thread::get_id() << std::endl;
    stream->SetLevel(aLevel);

    return os;
}


Logger::Logger(bool aCaptureClog)
    : mpClogBackup(nullptr)
{
    if (aCaptureClog) {
        mpClogBackup = std::clog.rdbuf(new OutStreamBuf(this, cDefautLogLevel));
    }
}

Logger::~Logger()
{
    if (mpClogBackup) {
        std::clog.rdbuf(mpClogBackup); // Restore backup before delete, to avoid segfault.
    }
}

LogStream Logger::Emergency()
{
    return LogStream(this, LogLevel::Emergency);
}

LogStream Logger::Alert()
{
    return LogStream(this, LogLevel::Alert);
}

LogStream Logger::Critical()
{
    return LogStream(this, LogLevel::Critical);
}

LogStream Logger::Error()
{
    return LogStream(this, LogLevel::Error);
}

LogStream Logger::Warning()
{
    return LogStream(this, LogLevel::Warning);
}

LogStream Logger::Notice()
{
    return LogStream(this, LogLevel::Notice);
}

LogStream Logger::Info()
{
    return LogStream(this, LogLevel::Info);
}

LogStream Logger::Debug()
{
    return LogStream(this, LogLevel::Debug);
}

} /* namespace rsp */

