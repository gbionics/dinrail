/*
 * SPDX-FileCopyrightText: 2006-2021 Istituto Italiano di Tecnologia (IIT)
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "Module.h"

#include <dinrail/RTMemoryGuard.h>

#include <yarp/robotinterface/Action.h>
#include <yarp/robotinterface/Device.h>
#include <yarp/robotinterface/Param.h>
#include <yarp/robotinterface/Robot.h>
#include <yarp/robotinterface/XMLReader.h>

#include <yarp/conf/system.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/ResourceFinder.h>

#if __has_include( \
    <execinfo.h>) && !defined(__APPLE__) && !defined(__arm__) && !defined(__aarch64__) && !defined(__PPC__)
#include <csignal>
#include <cstring>
#include <execinfo.h>
#endif

#ifndef DINRAIL_RUNNER_DEFAULT_USE_RT_SAFE_MEMORY_SETTINGS
#define DINRAIL_RUNNER_DEFAULT_USE_RT_SAFE_MEMORY_SETTINGS false
#endif

namespace
{
YARP_LOG_COMPONENT(DINRAIL_RUNNER_LOG, "dinrail.runner")
}

class dinrail::runner::Module::Private
{
public:
    Private(Module* parent);
    ~Private();

#if __has_include( \
    <execinfo.h>) && !defined(__APPLE__) && !defined(__arm__) && !defined(__aarch64__) && !defined(__PPC__)
    static struct sigaction old_action;
    static void sigsegv_handler(int nSignum, siginfo_t* si, void* vcontext);
#endif

    Module* const parent;
    yarp::robotinterface::Robot robot;
    dinrail::RTMemoryGuard rtMemoryGuard;
    int interruptReceived;
    bool closed;
    bool closeOk;
    bool autocloseAfterStart;
};

#if __has_include( \
    <execinfo.h>) && !defined(__APPLE__) && !defined(__arm__) && !defined(__aarch64__) && !defined(__PPC__)
struct sigaction dinrail::runner::Module::Private::old_action;
#endif

// At initialization 0 bytes stack prefault in rtMemoryGuard, and default 128 MiB heap reserve, do
// not activate now. However user is requested to call prefaultCurrentThreadStack(size_t bytes) in
// the begining of RT thread before loop starts. This will avoid first-use stack page faults so
// jitter in first few milliseconds is reduced. User must ensure bytes stays comfortably under the
// thread's actual stack size.
dinrail::runner::Module::Private::Private(Module* parent)
    : parent(parent)
    , rtMemoryGuard(0, 128 * 1024 * 1024, false)
    , interruptReceived(0)
    , closed(false)
    , closeOk(true)
{
}

dinrail::runner::Module::Private::~Private() = default;

#if __has_include( \
    <execinfo.h>) && !defined(__APPLE__) && !defined(__arm__) && !defined(__aarch64__) && !defined(__PPC__)
void dinrail::runner::Module::Private::sigsegv_handler(int nSignum, siginfo_t* si, void* vcontext)
{
    auto context = reinterpret_cast<ucontext_t*>(vcontext);
#if defined(__x86_64__)
    context->uc_mcontext.gregs[REG_RIP]++;
#else
    context->uc_mcontext.gregs[REG_EIP]++;
#endif

    const size_t max_depth = 100;
    size_t stack_depth;
    void* stack_addrs[max_depth];
    char** stack_strings;
    stack_depth = backtrace(stack_addrs, max_depth);
    stack_strings = backtrace_symbols(stack_addrs, stack_depth);

    yError("dinrail-runner intercepted a segmentation fault caused by a faulty plugin:");
    yError("%s\n", stack_strings[2]);
    yarp_print_trace(stderr, __FILE__, __LINE__);

    // Free memory allocated by backtrace_symbols()
    free(stack_strings);

    // Restore original action
    sigaction(SIGSEGV, &old_action, nullptr);
}
#endif

dinrail::runner::Module::Module()
    : mPriv(new Private(this))
{
#if __has_include( \
    <execinfo.h>) && !defined(__APPLE__) && !defined(__arm__) && !defined(__aarch64__) && !defined(__PPC__)
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    memset(&Private::old_action, 0, sizeof(struct sigaction));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = Private::sigsegv_handler;
    sigaction(SIGSEGV, &action, &Private::old_action);
#endif
}

dinrail::runner::Module::~Module()
{
    delete mPriv;
}

bool dinrail::runner::Module::configure(yarp::os::ResourceFinder& rf)
{
    bool useRtSafeMemorySettings = DINRAIL_RUNNER_DEFAULT_USE_RT_SAFE_MEMORY_SETTINGS;
    if (rf.check("use_rt_safe_memory_settings"))
    {
        const yarp::os::Value optionValue = rf.find("use_rt_safe_memory_settings");
        useRtSafeMemorySettings = optionValue.isNull() ? true : optionValue.asBool();
    }

    if (useRtSafeMemorySettings)
    {
        if (!mPriv->rtMemoryGuard.activate())
        {
            const std::optional<std::string>& lastError = mPriv->rtMemoryGuard.lastError();
            if (lastError.has_value())
            {
                yError() << "RT-safe memory settings requested but not fully applied:"
                         << lastError.value();
            } else
            {
                yError() << "RT-safe memory settings requested but not fully applied.";
            }
            return false;
        }
    }

    if (!rf.check("config"))
    {
        yFatal() << "Missing \"config\" argument";
    }

    const std::string& filename = rf.findFile("config");
    yTrace() << "Reading robot config file" << filename;

    bool verbosity = rf.check("verbose");
    bool deprecated = rf.check("allow-deprecated-dtd");
    bool dryrun = rf.check("dryrun");
    mPriv->autocloseAfterStart = rf.check("autocloseAfterStart");

    yarp::robotinterface::XMLReader reader;
    reader.setVerbose(verbosity);
    reader.setEnableDeprecated(deprecated);

    // Prepare configuration for sub-devices
    yarp::os::Property config;
    config.fromString(rf.toString());
    // The --config option is consumed by dinrail-runner, and never
    // forwarded to the devices)
    config.unput("config");

    yarp::robotinterface::XMLReaderResult result = reader.getRobotFromFile(filename, config);

    if (!result.parsingIsSuccessful)
    {
        yFatal() << "Config file " << filename << " not parsed correctly.";
    }

    mPriv->robot = std::move(result.robot);
    // yDebug() << mPriv->robot;

    // User can use YARP_PORT_PREFIX environment variable to override
    // the default name, so we don't care of handling the --name
    // argument
    setName(mPriv->robot.portprefix().c_str());

    mPriv->robot.setVerbose(verbosity);
    mPriv->robot.setAllowDeprecatedDevices(rf.check("allow-deprecated-devices"));
    mPriv->robot.setDryRun(dryrun);

    std::string portprefix = mPriv->robot.portprefix();
    if (portprefix[0] != '/')
    {
        yWarning() << "****************************************************************************"
                      "*********\n"
                      "* dinrail-runner 'portprefix' parameter does not follow convention,  *\n"
                      "* it MUST start with a leading '/' since it is used as the full prefix port "
                      "name    *\n"
                      "*     name:    full port prefix name with leading '/',  e.g.  /robotName    "
                      "  *\n"
                      "* A temporary automatic fix will be done for you, but please fix your "
                      "config file   *\n"
                      "****************************************************************************"
                      "*********";
        portprefix = "/" + portprefix;
    }

    // Enter startup phase
    if (!mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseStartup)
        || !mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseRun))
    {
        yError() << "Error in" << ActionPhaseToString(mPriv->robot.currentPhase())
                 << "phase... see previous messages for more info";
        // stopModule() calls interruptModule() internally.
        // This ensure that interrupt1 phase actions (i.e. detach) are
        // performed before destroying the devices when we call close();
        stopModule();
        // According to robotology/yarp#482, close() is not called by
        // runModule(rf), and the user is supposed to leave everything
        // clean.
        close();
        return false;
    }
    return true;
}

double dinrail::runner::Module::getPeriod()
{
    // This return the period (in seconds) with which the updateModule method is called by the main
    // thread, as nothing is done there, 60 seconds (1 minute) is ok see
    // https://www.yarp.it/v3.12/classyarp_1_1os_1_1RFModule.html#ace2fdadde1a2690f274079fabd6420d2
    return 60;
}

bool dinrail::runner::Module::updateModule()
{
    yCDebug(DINRAIL_RUNNER_LOG) << "dinrail-runner running happily";
    if (mPriv->autocloseAfterStart
        && mPriv->robot.currentPhase() == yarp::robotinterface::ActionPhaseRun)
    {
        yCInfo(DINRAIL_RUNNER_LOG) << "`autocloseAfterStart` option selected. The executable is "
                                      "exiting";
        return false;
    }

    return true;
}

bool dinrail::runner::Module::interruptModule()
{
    mPriv->interruptReceived++;

    yCWarning(DINRAIL_RUNNER_LOG) << "Interrupt #" << mPriv->interruptReceived << "# received.";

    mPriv->robot.interrupt();

    // In the first interrupt, after sending the interrupt() command
    // to the robot we exit the callback. In the close() method then
    // we proceed with the interupt1 phase, where we wait for all the
    // threads already started are joined, and finally we start the
    // interrupt1 actions.
    // In the second and third interrupts, we enter the interrupt2
    // phase in the callback. This means that in Robot, we cannot
    // wait for the other threads using join().
    switch (mPriv->interruptReceived)
    {
    case 1:
        break;
    case 2:
        if (!mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseInterrupt2))
        {
            yCError(DINRAIL_RUNNER_LOG)
                << "Error in" << ActionPhaseToString(yarp::robotinterface::ActionPhaseInterrupt2)
                << "phase... see previous messages for more info";
            return false;
        }
        break;
    case 3:
        if (!mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseInterrupt3))
        {
            yCError(DINRAIL_RUNNER_LOG)
                << "Error in" << ActionPhaseToString(yarp::robotinterface::ActionPhaseInterrupt3)
                << "phase... see previous messages for more info";
            return false;
        }
        break;
    default:
        return false;
    }

    return true;
}

bool dinrail::runner::Module::close()
{
    if (mPriv->closed)
    {
        return mPriv->closeOk;
    }
    mPriv->closed = true;

    // If called from the first interrupt, enter the corresponding
    // interrupt phase.
    switch (mPriv->interruptReceived)
    {
    case 1:
        if (!mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseInterrupt1))
        {
            yCError(DINRAIL_RUNNER_LOG)
                << "Error in" << ActionPhaseToString(yarp::robotinterface::ActionPhaseInterrupt1)
                << "phase... see previous messages for more info";
            mPriv->closeOk = false;
        }
        break;
    case 2:
    case 3:
        break;
    default:
        mPriv->closeOk = false;
    }

    // Finally call the shutdown phase.
    if (!mPriv->robot.enterPhase(yarp::robotinterface::ActionPhaseShutdown))
    {
        yCError(DINRAIL_RUNNER_LOG)
            << "Error in" << ActionPhaseToString(yarp::robotinterface::ActionPhaseShutdown)
            << "phase... see previous messages for more info";
        mPriv->closeOk = false;
    }

    return mPriv->closeOk;
}
