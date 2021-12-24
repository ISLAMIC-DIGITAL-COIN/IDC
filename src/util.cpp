// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Copyright (c) 2021 The ISLAMIC DIGITAL COIN developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include "config/islamic_digital_coin-config.h"
#endif

#include "util.h"

#include "allocators.h"
#include "chainparamsbase.h"
#include "random.h"
#include "utilstrencodings.h"
#include "utiltime.h"

#include <librustzcash.h>

#include <stdarg.h>
#include <thread>

#ifndef WIN32
// for posix_fallocate
#ifdef __linux__

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#define _POSIX_C_SOURCE 200112L

#endif // __linux__

#include <algorithm>
#include <fcntl.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/stat.h>

#else

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#pragma warning(disable : 4804)
#pragma warning(disable : 4805)
#pragma warning(disable : 4717)
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501

#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0501

#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <codecvt>

#include <io.h> /* for _commit */
#include <shlobj.h>
#endif

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#ifdef MAC_OSX
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <boost/program_options/detail/config_file.hpp>
#include <boost/program_options/parsers.hpp>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>

const char * const ISLAMIC_DIGITAL_COIN_CONF_FILENAME = "islamic_digital_coin.conf";
const char * const ISLAMIC_DIGITAL_COIN_PID_FILENAME = "islamic_digital_coin.pid";
const char * const ISLAMIC_DIGITAL_COIN_MASTERNODE_CONF_FILENAME = "masternode.conf";


// ISLAMIC_DIGITAL_COIN only features
// Masternode
std::atomic<bool> fMasterNode{false};
std::string strMasterNodeAddr = "";
bool fLiteMode = false;
// budget finalization
std::string strBudgetMode = "";

ArgsManager gArgs;

bool fDaemon = false;
CTranslationInterface translationInterface;

/** Init OpenSSL library multithreading support */
static RecursiveMutex** ppmutexOpenSSL;
void locking_callback(int mode, int i, const char* file, int line) NO_THREAD_SAFETY_ANALYSIS
{
    if (mode & CRYPTO_LOCK) {
        ENTER_CRITICAL_SECTION(*ppmutexOpenSSL[i]);
    } else {
        LEAVE_CRITICAL_SECTION(*ppmutexOpenSSL[i]);
    }
}

// Init
class CInit
{
public:
    CInit()
    {
        // Init OpenSSL library multithreading support
        ppmutexOpenSSL = (RecursiveMutex**)OPENSSL_malloc(CRYPTO_num_locks() * sizeof(RecursiveMutex*));
        for (int i = 0; i < CRYPTO_num_locks(); i++)
            ppmutexOpenSSL[i] = new RecursiveMutex();
        CRYPTO_set_locking_callback(locking_callback);

        // OpenSSL can optionally load a config file which lists optional loadable modules and engines.
        // We don't use them so we don't require the config. However some of our libs may call functions
        // which attempt to load the config file, possibly resulting in an exit() or crash if it is missing
        // or corrupt. Explicitly tell OpenSSL not to try to load the file. The result for our libs will be
        // that the config appears to have been loaded and there are no modules/engines available.
        OPENSSL_no_config();

#ifdef WIN32
        // Seed OpenSSL PRNG with current contents of the screen
        RAND_screen();
#endif

        // Seed OpenSSL PRNG with performance counter
        RandAddSeed();
    }
    ~CInit()
    {
        // Securely erase the memory used by the PRNG
        RAND_cleanup();
        // Shutdown OpenSSL library multithreading support
        CRYPTO_set_locking_callback(NULL);
        for (int i = 0; i < CRYPTO_num_locks(); i++)
            delete ppmutexOpenSSL[i];
        OPENSSL_free(ppmutexOpenSSL);
    }
} instance_of_cinit;


/**
 * Interpret a string argument as a boolean.
 *
 * The definition of atoi() requires that non-numeric string values like "foo",
 * return 0. This means that if a user unintentionally supplies a non-integer
 * argument here, the return value is always false. This means that -foo=false
 * does what the user probably expects, but -foo=true is well defined but does
 * not do what they probably expected.
 *
 * The return value of atoi() is undefined when given input not representable as
 * an int. On most systems this means string value between "-2147483648" and
 * "2147483647" are well defined (this method will return true). Setting
 * -txindex=2147483648 on most systems, however, is probably undefined.
 *
 * For a more extensive discussion of this topic (and a wide range of opinions
 * on the Right Way to change this code), see upstream PR12713.
 */
static bool InterpretBool(const std::string& strValue)
{
    if (strValue.empty())
        return true;
    return (atoi(strValue) != 0);
}

/**
 * Interpret -nofoo as if the user supplied -foo=0.
 *
 * This method also tracks when the -no form was supplied, and treats "-foo" as
 * a negated option when this happens. This can be later checked using the
 * IsArgNegated() method. One use case for this is to have a way to disable
 * options that are not normally boolean (e.g. using -nodebuglogfile to request
 * that debug log output is not sent to any file at all).
 */
void ArgsManager::InterpretNegatedOption(std::string& key, std::string& val)
{
    if (key.substr(0, 3) == "-no") {
        bool bool_val = InterpretBool(val);
        if (!bool_val ) {
            // Double negatives like -nofoo=0 are supported (but discouraged)
            LogPrintf("Warning: parsed potentially confusing double-negative %s=%s\n", key, val);
        }
        key.erase(1, 2);
        m_negated_args.insert(key);
        val = bool_val ? "0" : "1";
    } else {
        // In an invocation like "bitcoind -nofoo -foo" we want to unmark -foo
        // as negated when we see the second option.
        m_negated_args.erase(key);
    }
}

void ArgsManager::ParseParameters(int argc, const char* const argv[])
{
    LOCK(cs_args);
    mapArgs.clear();
    mapMultiArgs.clear();
    m_negated_args.clear();

    for (int i = 1; i < argc; i++) {
        std::string key(argv[i]);
        std::string val;
        size_t is_index = key.find('=');
        if (is_index != std::string::npos) {
            val = key.substr(is_index + 1);
            key.erase(is_index);
        }
#ifdef WIN32
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if (key[0] == '/')
            key[0] = '-';
#endif

        if (key[0] != '-')
            break;

        // Transform --foo to -foo
        if (key.length() > 1 && key[1] == '-')
            key.erase(0, 1);

        // Transform -nofoo to -foo=0
        InterpretNegatedOption(key, val);

        mapArgs[key] = val;
        mapMultiArgs[key].push_back(val);
    }
}

std::vector<std::string> ArgsManager::GetArgs(const std::string& strArg) const
{
    LOCK(cs_args);
    auto it = mapMultiArgs.find(strArg);
    if (it != mapMultiArgs.end()) return it->second;
    return {};
}

bool ArgsManager::IsArgSet(const std::string& strArg) const
{
    LOCK(cs_args);
    return mapArgs.count(strArg);
}

bool ArgsManager::IsArgNegated(const std::string& strArg) const
{
    LOCK(cs_args);
    return m_negated_args.find(strArg) != m_negated_args.end();
}

std::string ArgsManager::GetArg(const std::string& strArg, const std::string& strDefault) const
{
    LOCK(cs_args);
    auto it = mapArgs.find(strArg);
    if (it != mapArgs.end()) return it->second;
    return strDefault;
}

int64_t ArgsManager::GetArg(const std::string& strArg, int64_t nDefault) const
{
    LOCK(cs_args);
    auto it = mapArgs.find(strArg);
    if (it != mapArgs.end()) return atoi64(it->second);
    return nDefault;
}

bool ArgsManager::GetBoolArg(const std::string& strArg, bool fDefault) const
{
    LOCK(cs_args);
    auto it = mapArgs.find(strArg);
    if (it != mapArgs.end()) return InterpretBool(it->second);
    return fDefault;
}

bool ArgsManager::SoftSetArg(const std::string& strArg, const std::string& strValue)
{
    LOCK(cs_args);
    if (IsArgSet(strArg)) return false;;
    ForceSetArg(strArg, strValue);
    return true;
}

bool ArgsManager::SoftSetBoolArg(const std::string& strArg, bool fValue)
{
    if (fValue)
        return SoftSetArg(strArg, std::string("1"));
    else
        return SoftSetArg(strArg, std::string("0"));
}

void ArgsManager::ForceSetArg(const std::string& strArg, const std::string& strValue)
{
    LOCK(cs_args);
    mapArgs[strArg] = strValue;
    mapMultiArgs[strArg] = {strValue};
}

static const int screenWidth = 79;
static const int optIndent = 2;
static const int msgIndent = 7;

std::string HelpMessageGroup(const std::string &message) {
    return std::string(message) + std::string("\n\n");
}

std::string HelpMessageOpt(const std::string &option, const std::string &message) {
    return std::string(optIndent,' ') + std::string(option) +
           std::string("\n") + std::string(msgIndent,' ') +
           FormatParagraph(message, screenWidth - msgIndent, msgIndent) +
           std::string("\n\n");
}

static std::string FormatException(const std::exception* pex, const char* pszThread)
{
#ifdef WIN32
    char pszModule[MAX_PATH] = "";
    GetModuleFileNameA(NULL, pszModule, sizeof(pszModule));
#else
    const char* pszModule = "islamic_digital_coin";
#endif
    if (pex)
        return strprintf(
            "EXCEPTION: %s       \n%s       \n%s in %s       \n", typeid(*pex).name(), pex->what(), pszModule, pszThread);
    else
        return strprintf(
            "UNKNOWN EXCEPTION       \n%s in %s       \n", pszModule, pszThread);
}

void PrintExceptionContinue(const std::exception* pex, const char* pszThread)
{
    std::string message = FormatException(pex, pszThread);
    LogPrintf("\n\n************************\n%s\n", message);
    fprintf(stderr, "\n\n************************\n%s\n", message.c_str());
}

fs::path GetDefaultDataDir()
{
// Windows < Vista: C:\Documents and Settings\Username\Application Data\ISLAMIC_DIGITAL_COIN
// Windows >= Vista: C:\Users\Username\AppData\Roaming\ISLAMIC_DIGITAL_COIN
// Mac: ~/Library/Application Support/ISLAMIC_DIGITAL_COIN
// Unix: ~/.islamic_digital_coin
#ifdef WIN32
    // Windows
    return GetSpecialFolderPath(CSIDL_APPDATA) / "ISLAMIC_DIGITAL_COIN";
#else
    fs::path pathRet;
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
        pathRet = fs::path("/");
    else
        pathRet = fs::path(pszHome);
#ifdef MAC_OSX
    // Mac
    pathRet /= "Library/Application Support";
    TryCreateDirectory(pathRet);
    return pathRet / "ISLAMIC_DIGITAL_COIN";
#else
    // Unix
    return pathRet / ".islamic_digital_coin";
#endif
#endif
}

static fs::path pathCached;
static fs::path pathCachedNetSpecific;
static fs::path zc_paramsPathCached;
static RecursiveMutex csPathCached;

static fs::path ZC_GetBaseParamsDir()
{
    // Copied from GetDefaultDataDir and adapter for zcash params.
    // Windows < Vista: C:\Documents and Settings\Username\Application Data\ISLAMIC_DIGITAL_COINParams
    // Windows >= Vista: C:\Users\Username\AppData\Roaming\ISLAMIC_DIGITAL_COINParams
    // Mac: ~/Library/Application Support/ISLAMIC_DIGITAL_COINParams
    // Unix: ~/.islamic_digital_coin-params
#ifdef WIN32
    // Windows
    return GetSpecialFolderPath(CSIDL_APPDATA) / "ISLAMIC_DIGITAL_COINParams";
#else
    fs::path pathRet;
    char* pszHome = getenv("HOME");
    if (pszHome == NULL || strlen(pszHome) == 0)
        pathRet = fs::path("/");
    else
        pathRet = fs::path(pszHome);
#ifdef MAC_OSX
    // Mac
    pathRet /= "Library/Application Support";
    TryCreateDirectory(pathRet);
    return pathRet / "ISLAMIC_DIGITAL_COINParams";
#else
    // Unix
    return pathRet / ".islamic_digital_coin-params";
#endif
#endif
}

const fs::path &ZC_GetParamsDir()
{
    LOCK(csPathCached); // Reuse the same lock as upstream.

    fs::path &path = zc_paramsPathCached;

    // This can be called during exceptions by LogPrintf(), so we cache the
    // value so we don't have to do memory allocations after that.
    if (!path.empty())
        return path;

#ifdef USE_CUSTOM_PARAMS
    path = fs::system_complete(PARAMS_DIR);
#else
    if (gArgs.IsArgSet("-paramsdir")) {
        path = fs::system_complete(gArgs.GetArg("-paramsdir", ""));
        if (!fs::is_directory(path)) {
            path = "";
            return path;
        }
    } else {
        path = ZC_GetBaseParamsDir();
    }
#endif

    return path;
}

void initZKSNARKS()
{
    const fs::path& path = ZC_GetParamsDir();
    fs::path sapling_spend = path / "sapling-spend.params";
    fs::path sapling_output = path / "sapling-output.params";

    bool fParamsFound = false;
    if (fs::exists(sapling_spend) && fs::exists(sapling_output)) {
        fParamsFound = true;
    } else {
#ifdef MAC_OSX
        // macOS fallback path for params located within the app bundle
        // This is a somewhat convoluted series of CoreFoundation calls
        // that will result in the full path to the app bundle's "Resources"
        // directory, which will contain the sapling params.
        LogPrintf("Attempting to find params in app bundle...\n");
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef bundleURL = CFBundleCopyBundleURL(mainBundle);

        CFStringRef strBundlePath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
        const char* pathBundle = CFStringGetCStringPtr(strBundlePath, CFStringGetSystemEncoding());

        fs::path bundle_path = fs::path(pathBundle);
        LogPrintf("App bundle Resources path: %s\n", bundle_path);
        sapling_spend = bundle_path / "Contents/Resources/sapling-spend.params";
        sapling_output = bundle_path / "Contents/Resources/sapling-output.params";

        // Release the CF objects
        CFRelease(strBundlePath);
        CFRelease(bundleURL);
        CFRelease(mainBundle);
#else
        // Linux fallback path for debuild/ppa based installs
        sapling_spend = "/usr/share/islamic_digital_coin/sapling-spend.params";
        sapling_output = "/usr/share/islamic_digital_coin/sapling-output.params";
        if (fs::exists(sapling_spend) && fs::exists(sapling_output)) {
            fParamsFound = true;
        } else {
            // Linux fallback for local installs
            sapling_spend = "/usr/local/share/islamic_digital_coin/sapling-spend.params";
            sapling_output = "/usr/local/share/islamic_digital_coin/sapling-output.params";
        }
#endif
        if (fs::exists(sapling_spend) && fs::exists(sapling_output))
            fParamsFound = true;
    }
    if (!fParamsFound)
        throw std::runtime_error("Sapling params don't exist");

    static_assert(
        sizeof(fs::path::value_type) == sizeof(codeunit),
        "librustzcash not configured correctly");
    auto sapling_spend_str = sapling_spend.native();
    auto sapling_output_str = sapling_output.native();

    //LogPrintf("Loading Sapling (Spend) parameters from %s\n", sapling_spend.string().c_str());

    librustzcash_init_zksnark_params(
        reinterpret_cast<const codeunit*>(sapling_spend_str.c_str()),
        sapling_spend_str.length(),
        "8270785a1a0d0bc77196f000ee6d221c9c9894f55307bd9357c3f0105d31ca63991ab91324160d8f53e2bbd3c2633a6eb8bdf5205d822e7f3f73edac51b2b70c",
        reinterpret_cast<const codeunit*>(sapling_output_str.c_str()),
        sapling_output_str.length(),
        "657e3d38dbb5cb5e7dd2970e8b03d69b4787dd907285b5a7f0790dcc8072f60bf593b32cc2d1c030e00ff5ae64bf84c5c3beb84ddc841d48264b4a171744d028",
        nullptr,    // sprout_path
        0,          // sprout_path_len
        ""          // sprout_hash
    );

    //std::cout << "### Sapling params initialized ###" << std::endl;
}

const fs::path& GetDataDir(bool fNetSpecific)
{
    LOCK(csPathCached);

    fs::path& path = fNetSpecific ? pathCachedNetSpecific : pathCached;

    // This can be called during exceptions by LogPrintf(), so we cache the
    // value so we don't have to do memory allocations after that.
    if (!path.empty())
        return path;

    if (gArgs.IsArgSet("-datadir")) {
        path = fs::system_complete(gArgs.GetArg("-datadir", ""));
        if (!fs::is_directory(path)) {
            path = "";
            return path;
        }
    } else {
        path = GetDefaultDataDir();
    }
    if (fNetSpecific)
        path /= BaseParams().DataDir();

    fs::create_directories(path);

    return path;
}

void ClearDatadirCache()
{
    LOCK(csPathCached);

    pathCached = fs::path();
    pathCachedNetSpecific = fs::path();
}

fs::path GetConfigFile()
{
    fs::path pathConfigFile(gArgs.GetArg("-conf", ISLAMIC_DIGITAL_COIN_CONF_FILENAME));
    return AbsPathForConfigVal(pathConfigFile, false);
}

fs::path GetMasternodeConfigFile()
{
    fs::path pathConfigFile(gArgs.GetArg("-mnconf", ISLAMIC_DIGITAL_COIN_MASTERNODE_CONF_FILENAME));
    return AbsPathForConfigVal(pathConfigFile);
}

void ArgsManager::ReadConfigFile()
{
    fs::ifstream streamConfig(GetConfigFile());
    if (!streamConfig.good()) {
        // Create empty islamic_digital_coin.conf if it does not exist
        FILE* configFile = fsbridge::fopen(GetConfigFile(), "a");
        if (configFile != NULL)
            fclose(configFile);
        return; // Nothing to read, so just return
    }

    {
        LOCK(cs_args);
        std::set<std::string> setOptions;
        setOptions.insert("*");

        for (boost::program_options::detail::config_file_iterator it(streamConfig, setOptions), end; it != end; ++it) {
            // Don't overwrite existing settings so command line settings override islamic_digital_coin.conf
            std::string strKey = std::string("-") + it->string_key;
            std::string strValue = it->value[0];
            InterpretNegatedOption(strKey, strValue);
            if (mapArgs.count(strKey) == 0)
                mapArgs[strKey] = strValue;
            mapMultiArgs[strKey].push_back(strValue);
        }
    }
    // If datadir is changed in .conf file:
    ClearDatadirCache();
}

fs::path AbsPathForConfigVal(const fs::path& path, bool net_specific)
{
    if (path.is_absolute()) {
        return path;
    }
    return fs::absolute(path, GetDataDir(net_specific));
}

#ifndef WIN32
fs::path GetPidFile()
{
    fs::path pathPidFile(gArgs.GetArg("-pid", ISLAMIC_DIGITAL_COIN_PID_FILENAME));
    return AbsPathForConfigVal(pathPidFile);
}

void CreatePidFile(const fs::path& path, pid_t pid)
{
    FILE* file = fsbridge::fopen(path, "w");
    if (file) {
        fprintf(file, "%d\n", pid);
        fclose(file);
    }
}
#endif

bool RenameOver(fs::path src, fs::path dest)
{
#ifdef WIN32
    return MoveFileExA(src.string().c_str(), dest.string().c_str(),
               MOVEFILE_REPLACE_EXISTING) != 0;
#else
    int rc = std::rename(src.string().c_str(), dest.string().c_str());
    return (rc == 0);
#endif /* WIN32 */
}

/**
 * Ignores exceptions thrown by Boost's create_directory if the requested directory exists.
 * Specifically handles case where path p exists, but it wasn't possible for the user to
 * write to the parent directory.
 */
bool TryCreateDirectory(const fs::path& p)
{
    try {
        return fs::create_directory(p);
    } catch (const fs::filesystem_error&) {
        if (!fs::exists(p) || !fs::is_directory(p))
            throw;
    }

    // create_directory didn't create the directory, it had to have existed already
    return false;
}

void FileCommit(FILE* fileout)
{
    fflush(fileout); // harmless if redundantly called
#ifdef WIN32
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(fileout));
    FlushFileBuffers(hFile);
#else
#if defined(__linux__) || defined(__NetBSD__)
    fdatasync(fileno(fileout));
#elif defined(__APPLE__) && defined(F_FULLFSYNC)
    fcntl(fileno(fileout), F_FULLFSYNC, 0);
#else
    fsync(fileno(fileout));
#endif
#endif
}

bool TruncateFile(FILE* file, unsigned int length)
{
#if defined(WIN32)
    return _chsize(_fileno(file), length) == 0;
#else
    return ftruncate(fileno(file), length) == 0;
#endif
}

/**
 * this function tries to raise the file descriptor limit to the requested number.
 * It returns the actual file descriptor limit (which may be more or less than nMinFD)
 */
int RaiseFileDescriptorLimit(int nMinFD)
{
#if defined(WIN32)
    return 2048;
#else
    struct rlimit limitFD;
    if (getrlimit(RLIMIT_NOFILE, &limitFD) != -1) {
        if (limitFD.rlim_cur < (rlim_t)nMinFD) {
            limitFD.rlim_cur = nMinFD;
            if (limitFD.rlim_cur > limitFD.rlim_max)
                limitFD.rlim_cur = limitFD.rlim_max;
            setrlimit(RLIMIT_NOFILE, &limitFD);
            getrlimit(RLIMIT_NOFILE, &limitFD);
        }
        return limitFD.rlim_cur;
    }
    return nMinFD; // getrlimit failed, assume it's fine
#endif
}

/**
 * this function tries to make a particular range of a file allocated (corresponding to disk space)
 * it is advisory, and the range specified in the arguments will never contain live data
 */
void AllocateFileRange(FILE* file, unsigned int offset, unsigned int length)
{
#if defined(WIN32)
    // Windows-specific version
    HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(file));
    LARGE_INTEGER nFileSize;
    int64_t nEndPos = (int64_t)offset + length;
    nFileSize.u.LowPart = nEndPos & 0xFFFFFFFF;
    nFileSize.u.HighPart = nEndPos >> 32;
    SetFilePointerEx(hFile, nFileSize, 0, FILE_BEGIN);
    SetEndOfFile(hFile);
#elif defined(MAC_OSX)
    // OSX specific version
    fstore_t fst;
    fst.fst_flags = F_ALLOCATECONTIG;
    fst.fst_posmode = F_PEOFPOSMODE;
    fst.fst_offset = 0;
    fst.fst_length = (off_t)offset + length;
    fst.fst_bytesalloc = 0;
    if (fcntl(fileno(file), F_PREALLOCATE, &fst) == -1) {
        fst.fst_flags = F_ALLOCATEALL;
        fcntl(fileno(file), F_PREALLOCATE, &fst);
    }
    ftruncate(fileno(file), fst.fst_length);
#elif defined(__linux__)
    // Version using posix_fallocate
    off_t nEndPos = (off_t)offset + length;
    posix_fallocate(fileno(file), 0, nEndPos);
#else
    // Fallback version
    // TODO: just write one byte per block
    static const char buf[65536] = {};
    fseek(file, offset, SEEK_SET);
    while (length > 0) {
        unsigned int now = 65536;
        if (length < now)
            now = length;
        fwrite(buf, 1, now, file); // allowed to fail; this function is advisory anyway
        length -= now;
    }
#endif
}

#ifdef WIN32
fs::path GetSpecialFolderPath(int nFolder, bool fCreate)
{
    WCHAR pszPath[MAX_PATH] = L"";

    if (SHGetSpecialFolderPathW(nullptr, pszPath, nFolder, fCreate)) {
        return fs::path(pszPath);
    }

    LogPrintf("SHGetSpecialFolderPathW() failed, could not obtain requested path.\n");
    return fs::path("");
}
#endif

fs::path GetTempPath()
{
    return fs::temp_directory_path();
}

double double_safe_addition(double fValue, double fIncrement)
{
    double fLimit = std::numeric_limits<double>::max() - fValue;

    if (fLimit > fIncrement)
        return fValue + fIncrement;
    else
        return std::numeric_limits<double>::max();
}

double double_safe_multiplication(double fValue, double fmultiplicator)
{
    double fLimit = std::numeric_limits<double>::max() / fmultiplicator;

    if (fLimit > fmultiplicator)
        return fValue * fmultiplicator;
    else
        return std::numeric_limits<double>::max();
}

void runCommand(std::string strCommand)
{
    if (strCommand.empty()) return;
#ifndef WIN32
    int nErr = ::system(strCommand.c_str());
#else
    int nErr = ::_wsystem(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t>().from_bytes(strCommand).c_str());
#endif
    if (nErr)
        LogPrintf("runCommand error: system(%s) returned %d\n", strCommand, nErr);
}

void SetupEnvironment()
{
// On most POSIX systems (e.g. Linux, but not BSD) the environment's locale
// may be invalid, in which case the "C" locale is used as fallback.
#if !defined(WIN32) && !defined(MAC_OSX) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
    try {
        std::locale(""); // Raises a runtime error if current locale is invalid
    } catch (const std::runtime_error&) {
        setenv("LC_ALL", "C", 1);
    }
#endif
    // The path locale is lazy initialized and to avoid deinitialization errors
    // in multithreading environments, it is set explicitly by the main thread.
    // A dummy locale is used to extract the internal default locale, used by
    // fs::path, which is then used to explicitly imbue the path.
    std::locale loc = fs::path::imbue(std::locale::classic());
#ifndef WIN32
    fs::path::imbue(loc);
#else
    fs::path::imbue(std::locale(loc, new std::codecvt_utf8_utf16<wchar_t>()));
#endif
}

bool SetupNetworking()
{
#ifdef WIN32
    // Initialize Windows Sockets
    WSADATA wsadata;
    int ret = WSAStartup(MAKEWORD(2,2), &wsadata);
    if (ret != NO_ERROR || LOBYTE(wsadata.wVersion ) != 2 || HIBYTE(wsadata.wVersion) != 2)
        return false;
#endif
    return true;
}

void SetThreadPriority(int nPriority)
{
#ifdef WIN32
    SetThreadPriority(GetCurrentThread(), nPriority);
#else // WIN32
#ifdef PRIO_THREAD
    setpriority(PRIO_THREAD, 0, nPriority);
#else  // PRIO_THREAD
    setpriority(PRIO_PROCESS, 0, nPriority);
#endif // PRIO_THREAD
#endif // WIN32
}

int GetNumCores()
{
    return std::thread::hardware_concurrency();
}


int ScheduleBatchPriority(void)
{
#ifdef SCHED_BATCH
    const static sched_param param{0};
    if (int ret = pthread_setschedparam(pthread_self(), SCHED_BATCH, &param)) {
        LogPrintf("Failed to pthread_setschedparam: %s\n", strerror(errno));
        return ret;
    }
    return 0;
#else
    return 1;
#endif
}
