// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/**
 * Server/client environment: argument handling, config file parsing,
 * thread wrappers
 */
#ifndef BITCOIN_UTIL_H
#define BITCOIN_UTIL_H

#if defined(HAVE_CONFIG_H)
#include "config/islamic_digital_coin-config.h"
#endif

#include "fs.h"
#include "logging.h"
#include "compat.h"
#include "sync.h"
#include "tinyformat.h"
#include "utiltime.h"
#include "util/threadnames.h"

#include <atomic>
#include <exception>
#include <map>
#include <stdint.h>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/signals2/signal.hpp>
#include <boost/thread/condition_variable.hpp> // for boost::thread_interrupted

/** Signals for translation. */
class CTranslationInterface
{
public:
    /** Translate a message to the native language of the user. */
    boost::signals2::signal<std::string (const char* psz)> Translate;
};

extern const char * const ISLAMIC_DIGITAL_COIN_CONF_FILENAME;
extern const char * const ISLAMIC_DIGITAL_COIN_PID_FILENAME;
extern const char * const ISLAMIC_DIGITAL_COIN_MASTERNODE_CONF_FILENAME;
extern const char * const DEFAULT_DEBUGLOGFILE;

//ISLAMIC_DIGITAL_COIN only features

extern std::atomic<bool> fMasterNode;
extern bool fLiteMode;
extern std::string strMasterNodeAddr;
extern std::string strBudgetMode;

extern CTranslationInterface translationInterface;

/**
 * Translation function: Call Translate signal on UI interface, which returns a Optional result.
 * If no translation slot is registered, nothing is returned, and simply return the input.
 */
inline std::string _(const char* psz)
{
    // todo: this boost::optional is needed for now. Will get removed moving forward
    boost::optional<std::string> rv = translationInterface.Translate(psz);
    return rv ? (*rv) : psz;
}


void SetupEnvironment();
bool SetupNetworking();

template<typename... Args>
bool error(const char* fmt, const Args&... args)
{
    LogPrintf("ERROR: %s\n", tfm::format(fmt, args...));
    return false;
}

double double_safe_addition(double fValue, double fIncrement);
double double_safe_multiplication(double fValue, double fmultiplicator);
void PrintExceptionContinue(const std::exception* pex, const char* pszThread);
void FileCommit(FILE* fileout);
bool TruncateFile(FILE* file, unsigned int length);
int RaiseFileDescriptorLimit(int nMinFD);
void AllocateFileRange(FILE* file, unsigned int offset, unsigned int length);
bool RenameOver(fs::path src, fs::path dest);
bool TryCreateDirectory(const fs::path& p);
fs::path GetDefaultDataDir();
const fs::path &GetDataDir(bool fNetSpecific = true);
// Sapling network dir
const fs::path &ZC_GetParamsDir();
// Init sapling library
void initZKSNARKS();
void ClearDatadirCache();
fs::path GetConfigFile();
fs::path GetMasternodeConfigFile();
#ifndef WIN32
fs::path GetPidFile();
void CreatePidFile(const fs::path& path, pid_t pid);
#endif
#ifdef WIN32
fs::path GetSpecialFolderPath(int nFolder, bool fCreate = true);
#endif
fs::path GetTempPath();

void runCommand(std::string strCommand);

inline bool IsSwitchChar(char c)
{
#ifdef WIN32
    return c == '-' || c == '/';
#else
    return c == '-';
#endif
}

class ArgsManager
{
protected:
    mutable RecursiveMutex cs_args;
    std::map<std::string, std::string> mapArgs;
    std::map<std::string, std::vector<std::string>> mapMultiArgs;
    std::unordered_set<std::string> m_negated_args;

public:
    void ParseParameters(int argc, const char* const argv[]);
    void ReadConfigFile();

    /**
     * Return a vector of strings of the given argument
     *
     * @param strArg Argument to get (e.g. "-foo")
     * @return command-line arguments
     */
    std::vector<std::string> GetArgs(const std::string& strArg) const;

    /**
    * Return true if the given argument has been manually set
    *
    * @param strArg Argument to get (e.g. "-foo")
    * @return true if the argument has been set
    */
    bool IsArgSet(const std::string& strArg) const;

    /**
     * Return true if the argument was originally passed as a negated option,
     * i.e. -nofoo.
     *
     * @param strArg Argument to get (e.g. "-foo")
     * @return true if the argument was passed negated
     */
    bool IsArgNegated(const std::string& strArg) const;

    /**
    * Return string argument or default value
    *
    * @param strArg Argument to get (e.g. "-foo")
    * @param default (e.g. "1")
    * @return command-line argument or default value
    */
    std::string GetArg(const std::string& strArg, const std::string& strDefault) const;

    /**
    * Return integer argument or default value
    *
    * @param strArg Argument to get (e.g. "-foo")
    * @param default (e.g. 1)
    * @return command-line argument (0 if invalid number) or default value
    */
    int64_t GetArg(const std::string& strArg, int64_t nDefault) const;

    /**
    * Return boolean argument or default value
    *
    * @param strArg Argument to get (e.g. "-foo")
    * @param default (true or false)
    * @return command-line argument or default value
    */
    bool GetBoolArg(const std::string& strArg, bool fDefault) const;

    /**
    * Set an argument if it doesn't already have a value
    *
    * @param strArg Argument to set (e.g. "-foo")
    * @param strValue Value (e.g. "1")
    * @return true if argument gets set, false if it already had a value
    */
    bool SoftSetArg(const std::string& strArg, const std::string& strValue);

    /**
    * Set a boolean argument if it doesn't already have a value
    *
    * @param strArg Argument to set (e.g. "-foo")
    * @param fValue Value (e.g. false)
    * @return true if argument gets set, false if it already had a value
    */
    bool SoftSetBoolArg(const std::string& strArg, bool fValue);

    // Forces a arg setting, used only in testing
    void ForceSetArg(const std::string& strArg, const std::string& strValue);

private:

    // Munge -nofoo into -foo=0 and track the value as negated.
    void InterpretNegatedOption(std::string &key, std::string &val);
};

extern ArgsManager gArgs;

/**
 * Format a string to be used as group of options in help messages
 *
 * @param message Group name (e.g. "RPC server options:")
 * @return the formatted string
 */
std::string HelpMessageGroup(const std::string& message);

/**
 * Format a string to be used as option description in help messages
 *
 * @param option Option message (e.g. "-rpcuser=<user>")
 * @param message Option description (e.g. "Username for JSON-RPC connections")
 * @return the formatted string
 */
std::string HelpMessageOpt(const std::string& option, const std::string& message);

/**
 * Return the number of cores available on the current system.
 * @note This does count virtual cores, such as those provided by HyperThreading.
 */
int GetNumCores();

void SetThreadPriority(int nPriority);

/**
 * .. and a wrapper that just calls func once
 */
template <typename Callable>
void TraceThread(const char* name, Callable func)
{
    std::string s = strprintf("islamic_digital_coin-%s", name);
    util::ThreadRename(s.c_str());
    try {
        LogPrintf("%s thread start\n", name);
        func();
        LogPrintf("%s thread exit\n", name);
    } catch (boost::thread_interrupted) {
        LogPrintf("%s thread interrupt\n", name);
        throw;
    } catch (std::exception& e) {
        PrintExceptionContinue(&e, name);
        throw;
    } catch (...) {
        PrintExceptionContinue(NULL, name);
        throw;
    }
}

fs::path AbsPathForConfigVal(const fs::path& path, bool net_specific = true);

/**
 * On platforms that support it, tell the kernel the calling thread is
 * CPU-intensive and non-interactive. See SCHED_BATCH in sched(7) for details.
 *
 * @return The return value of sched_setschedule(), or 1 on systems without
 * sched_setchedule().
 */
int ScheduleBatchPriority(void);

#endif // BITCOIN_UTIL_H
