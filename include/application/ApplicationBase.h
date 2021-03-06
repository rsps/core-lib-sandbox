/*!
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * \copyright   Copyright 2022 RSP Systems A/S. All rights reserved.
 * \license     Mozilla Public License 2.0
 * \author      Steffen Brummer
 */

#ifndef INCLUDE_APPLICATION_APPLICATIONBASE_H_
#define INCLUDE_APPLICATION_APPLICATIONBASE_H_

#include <utils/CoreException.h>
#include <logging/Logger.h>
#include <application/CommandLine.h>

namespace rsp::application {

/**
 * \class ApplicationBase
 * \brief Easy to use base class for applications using this framework.
 */
class ApplicationBase
{
public:
    const int cResultSuccess = 0;
    const int cResultUnhandledError = -1;

    /**
     * Constructor that optionally takes command line arguments.
     *
     * \throws ESingletonViolation if more than one application instance is tried created.
     *
     * \param argc Number of arguments
     * \param argv Pointer to arguments
     */
    ApplicationBase(int argc = 0, const char **argv = nullptr);
    virtual ~ApplicationBase();
    /**
     * Inhibit copy and move of the application object.
     */
    ApplicationBase(const ApplicationBase &other) = delete;
    ApplicationBase(ApplicationBase &&other) = delete;
    ApplicationBase& operator=(const ApplicationBase &other) = delete;
    ApplicationBase& operator=(ApplicationBase &&other) = delete;

    /**
     * Get the default logger instance
     *
     * \return Reference to Logger instance.
     */
    logging::Logger& GetLog() { return mLogger; }

    /**
     * Get the command line object.
     *
     * \return Reference to the CommandLine object.
     */
    CommandLine& GetCommandLine() { return mCmd; }

    /**
     * Static getter for the ApplicationBase instance.
     *
     * \return Reference to the ApplicationBase object
     */
    static ApplicationBase& Get() { return Get<ApplicationBase>(); }

    /**
     * Static getter with cast to any overridden application object.
     *
     * \throws ENoInstance if no application have been created.
     *
     * \tparam T Type to cast the application reference into.
     * \return Reference to T type application object.
     */
    template<class T>
    static T& Get() {
        if (!mpInstance) {
            THROW_WITH_BACKTRACE(rsp::utils::ENoInstance);
        }
        return *static_cast<T*>(mpInstance);
    }

    /**
     * Call this to execute the main application loop.
     * It will return when the application is terminated.
     *
     * \return Integer result code to exit from the application.
     */
    int Run();

    /**
     * Terminate the application.
     *
     * \param aResult Integer result value to return from the application.
     */
    void Terminate(int aResult) {
        if (!mTerminated) {
            mApplicationResult = aResult;
            mTerminated = true;
        }
    }

protected:
    int mApplicationResult = 0;
    bool mTerminated = false;
    logging::Logger mLogger;
    CommandLine mCmd;

    /**
     * Virtual helpers, override these to add functionality during the run loop.
     */
    virtual void beforeExecute();
    virtual void execute() {};
    virtual void afterExecute() {};

    /**
     * Virtual helpers. Override to change default behaviors.
     */
    virtual void handleOptions();
    virtual void showHelp();
    virtual void showVersion();

private:
    /**
     * Private instance pointer for the singleton pattern.
     * Private to avoid it from being tampered with.
     */
    static ApplicationBase* mpInstance;
};

} /* namespace rsp::application */

#endif /* INCLUDE_APPLICATION_APPLICATIONBASE_H_ */
