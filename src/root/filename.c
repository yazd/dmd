
// Copyright (c) 1999-2012 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#include "filename.h"

#include <stdint.h>
#include <ctype.h>

#include "outbuffer.h"
#include "array.h"
#include "file.h"

#if defined (__sun)
#include <alloca.h>
#endif

#if _MSC_VER ||__MINGW32__
#include <malloc.h>
#include <string>
#endif

#if _WIN32
#include <windows.h>
#include <direct.h>
#include <errno.h>
#endif

#if POSIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <utime.h>
#endif


/**************************************
 * Print error message and exit.
 */

void error(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    printf("Error: ");
    vprintf(format, ap);
    va_end( ap );
    printf("\n");
    fflush(stdout);

    exit(EXIT_FAILURE);
}

/****************************** FileName ********************************/

FileName::FileName(const char *str)
    : str(mem.strdup(str))
{
}

const char *FileName::combine(const char *path, const char *name)
{   char *f;
    size_t pathlen;
    size_t namelen;

    if (!path || !*path)
        return (char *)name;
    pathlen = strlen(path);
    namelen = strlen(name);
    f = (char *)mem.malloc(pathlen + 1 + namelen + 1);
    memcpy(f, path, pathlen);
#if POSIX
    if (path[pathlen - 1] != '/')
    {   f[pathlen] = '/';
        pathlen++;
    }
#elif _WIN32
    if (path[pathlen - 1] != '\\' &&
        path[pathlen - 1] != '/'  &&
        path[pathlen - 1] != ':')
    {   f[pathlen] = '\\';
        pathlen++;
    }
#else
    assert(0);
#endif
    memcpy(f + pathlen, name, namelen + 1);
    return f;
}

// Split a path into an Array of paths
Strings *FileName::splitPath(const char *path)
{
    char c = 0;                         // unnecessary initializer is for VC /W4
    const char *p;
    OutBuffer buf;
    Strings *array;

    array = new Strings();
    if (path)
    {
        p = path;
        do
        {   char instring = 0;

            while (isspace((utf8_t)*p))         // skip leading whitespace
                p++;
            buf.reserve(strlen(p) + 1); // guess size of path
            for (; ; p++)
            {
                c = *p;
                switch (c)
                {
                    case '"':
                        instring ^= 1;  // toggle inside/outside of string
                        continue;

#if MACINTOSH
                    case ',':
#endif
#if _WIN32
                    case ';':
#endif
#if POSIX
                    case ':':
#endif
                        p++;
                        break;          // note that ; cannot appear as part
                                        // of a path, quotes won't protect it

                    case 0x1A:          // ^Z means end of file
                    case 0:
                        break;

                    case '\r':
                        continue;       // ignore carriage returns

#if POSIX
                    case '~':
                        buf.writestring(getenv("HOME"));
                        continue;
#endif

#if 0
                    case ' ':
                    case '\t':          // tabs in filenames?
                        if (!instring)  // if not in string
                            break;      // treat as end of path
#endif
                    default:
                        buf.writeByte(c);
                        continue;
                }
                break;
            }
            if (buf.offset)             // if path is not empty
            {
                buf.writeByte(0);       // to asciiz
                array->push(buf.extractData());
            }
        } while (c);
    }
    return array;
}

int FileName::compare(RootObject *obj)
{
    return compare(str, ((FileName *)obj)->str);
}

int FileName::compare(const char *name1, const char *name2)
{
#if _WIN32
    return stricmp(name1, name2);
#else
    return strcmp(name1, name2);
#endif
}

bool FileName::equals(RootObject *obj)
{
    return compare(obj) == 0;
}

int FileName::equals(const char *name1, const char *name2)
{
    return compare(name1, name2) == 0;
}

/************************************
 * Return !=0 if absolute path name.
 */

int FileName::absolute(const char *name)
{
#if _WIN32
    return (*name == '\\') ||
           (*name == '/')  ||
           (*name && name[1] == ':');
#elif POSIX
    return (*name == '/');
#else
    assert(0);
#endif
}

/********************************
 * Return filename extension (read-only).
 * Points past '.' of extension.
 * If there isn't one, return NULL.
 */

const char *FileName::ext(const char *str)
{
    size_t len = strlen(str);

    const char *e = str + len;
    for (;;)
    {
        switch (*e)
        {   case '.':
                return e + 1;
#if POSIX
            case '/':
                break;
#endif
#if _WIN32
            case '\\':
            case ':':
            case '/':
                break;
#endif
            default:
                if (e == str)
                    break;
                e--;
                continue;
        }
        return NULL;
    }
}

const char *FileName::ext()
{
    return ext(str);
}

/********************************
 * Return mem.malloc'd filename with extension removed.
 */

const char *FileName::removeExt(const char *str)
{
    const char *e = ext(str);
    if (e)
    {   size_t len = (e - str) - 1;
        char *n = (char *)mem.malloc(len + 1);
        memcpy(n, str, len);
        n[len] = 0;
        return n;
    }
    return mem.strdup(str);
}

/********************************
 * Return filename name excluding path (read-only).
 */

const char *FileName::name(const char *str)
{
    size_t len = strlen(str);

    const char *e = str + len;
    for (;;)
    {
        switch (*e)
        {
#if POSIX
            case '/':
               return e + 1;
#endif
#if _WIN32
            case '/':
            case '\\':
                return e + 1;
            case ':':
                /* The ':' is a drive letter only if it is the second
                 * character or the last character,
                 * otherwise it is an ADS (Alternate Data Stream) separator.
                 * Consider ADS separators as part of the file name.
                 */
                if (e == str + 1 || e == str + len - 1)
                    return e + 1;
#endif
            default:
                if (e == str)
                    break;
                e--;
                continue;
        }
        return e;
    }
}

const char *FileName::name()
{
    return name(str);
}

/**************************************
 * Return path portion of str.
 * Path will does not include trailing path separator.
 */

const char *FileName::path(const char *str)
{
    const char *n = name(str);
    size_t pathlen;

    if (n > str)
    {
#if POSIX
        if (n[-1] == '/')
            n--;
#elif _WIN32
        if (n[-1] == '\\' || n[-1] == '/')
            n--;
#else
        assert(0);
#endif
    }
    pathlen = n - str;
    char *path = (char *)mem.malloc(pathlen + 1);
    memcpy(path, str, pathlen);
    path[pathlen] = 0;
    return path;
}

/**************************************
 * Replace filename portion of path.
 */

const char *FileName::replaceName(const char *path, const char *name)
{
    size_t pathlen;
    size_t namelen;

    if (absolute(name))
        return name;

    const char *n = FileName::name(path);
    if (n == path)
        return name;
    pathlen = n - path;
    namelen = strlen(name);
    char *f = (char *)mem.malloc(pathlen + 1 + namelen + 1);
    memcpy(f, path, pathlen);
#if POSIX
    if (path[pathlen - 1] != '/')
    {   f[pathlen] = '/';
        pathlen++;
    }
#elif _WIN32
    if (path[pathlen - 1] != '\\' &&
        path[pathlen - 1] != '/' &&
        path[pathlen - 1] != ':')
    {   f[pathlen] = '\\';
        pathlen++;
    }
#else
    assert(0);
#endif
    memcpy(f + pathlen, name, namelen + 1);
    return f;
}

/***************************
 * Free returned value with FileName::free()
 */

const char *FileName::defaultExt(const char *name, const char *ext)
{
    const char *e = FileName::ext(name);
    if (e)                              // if already has an extension
        return mem.strdup(name);

    size_t len = strlen(name);
    size_t extlen = strlen(ext);
    char *s = (char *)mem.malloc(len + 1 + extlen + 1);
    memcpy(s,name,len);
    s[len] = '.';
    memcpy(s + len + 1, ext, extlen + 1);
    return s;
}

/***************************
 * Free returned value with FileName::free()
 */

const char *FileName::forceExt(const char *name, const char *ext)
{
    const char *e = FileName::ext(name);
    if (e)                              // if already has an extension
    {
        size_t len = e - name;
        size_t extlen = strlen(ext);

        char *s = (char *)mem.malloc(len + extlen + 1);
        memcpy(s,name,len);
        memcpy(s + len, ext, extlen + 1);
        return s;
    }
    else
        return defaultExt(name, ext);   // doesn't have one
}

/******************************
 * Return !=0 if extensions match.
 */

int FileName::equalsExt(const char *ext)
{
    return equalsExt(str, ext);
}

int FileName::equalsExt(const char *name, const char *ext)
{
    const char *e = FileName::ext(name);
    if (!e && !ext)
        return 1;
    if (!e || !ext)
        return 0;
    return FileName::compare(e, ext) == 0;
}

/*************************************
 * Copy file from this to to.
 */

void FileName::CopyTo(FileName *to)
{
    File file(this);

#if _WIN32
    file.touchtime = mem.malloc(sizeof(WIN32_FIND_DATAA));      // keep same file time
#elif POSIX
    file.touchtime = mem.malloc(sizeof(struct stat)); // keep same file time
#else
    assert(0);
#endif
    file.readv();
    file.name = to;
    file.writev();
}

/*************************************
 * Search Path for file.
 * Input:
 *      cwd     if !=0, search current directory before searching path
 */

const char *FileName::searchPath(Strings *path, const char *name, int cwd)
{
    if (absolute(name))
    {
        return exists(name) ? name : NULL;
    }
    if (cwd)
    {
        if (exists(name))
            return name;
    }
    if (path)
    {

        for (size_t i = 0; i < path->dim; i++)
        {
            const char *p = (*path)[i];
            const char *n = combine(p, name);

            if (exists(n))
                return n;
        }
    }
    return NULL;
}


/*************************************
 * Search Path for file in a safe manner.
 *
 * Be wary of CWE-22: Improper Limitation of a Pathname to a Restricted Directory
 * ('Path Traversal') attacks.
 *      http://cwe.mitre.org/data/definitions/22.html
 * More info:
 *      https://www.securecoding.cert.org/confluence/display/seccode/FIO02-C.+Canonicalize+path+names+originating+from+untrusted+sources
 * Returns:
 *      NULL    file not found
 *      !=NULL  mem.malloc'd file name
 */

const char *FileName::safeSearchPath(Strings *path, const char *name)
{
#if _WIN32
    /* Disallow % / \ : and .. in name characters
     */
    for (const char *p = name; *p; p++)
    {
        char c = *p;
        if (c == '\\' || c == '/' || c == ':' || c == '%' ||
            (c == '.' && p[1] == '.'))
        {
            return NULL;
        }
    }

    return FileName::searchPath(path, name, 0);
#elif POSIX
    /* Even with realpath(), we must check for // and disallow it
     */
    for (const char *p = name; *p; p++)
    {
        char c = *p;
        if (c == '/' && p[1] == '/')
        {
            return NULL;
        }
    }

    if (path)
    {
        /* Each path is converted to a cannonical name and then a check is done to see
         * that the searched name is really a child one of the the paths searched.
         */
        for (size_t i = 0; i < path->dim; i++)
        {
            const char *cname = NULL;
            const char *cpath = canonicalName((*path)[i]);
            //printf("FileName::safeSearchPath(): name=%s; path=%s; cpath=%s\n",
            //      name, (char *)path->data[i], cpath);
            if (cpath == NULL)
                goto cont;
            cname = canonicalName(combine(cpath, name));
            //printf("FileName::safeSearchPath(): cname=%s\n", cname);
            if (cname == NULL)
                goto cont;
            //printf("FileName::safeSearchPath(): exists=%i "
            //      "strncmp(cpath, cname, %i)=%i\n", exists(cname),
            //      strlen(cpath), strncmp(cpath, cname, strlen(cpath)));
            // exists and name is *really* a "child" of path
            if (exists(cname) && strncmp(cpath, cname, strlen(cpath)) == 0)
            {
                ::free((void *)cpath);
                const char *p = mem.strdup(cname);
                ::free((void *)cname);
                return p;
            }
cont:
            if (cpath)
                ::free((void *)cpath);
            if (cname)
                ::free((void *)cname);
        }
    }
    return NULL;
#else
    assert(0);
#endif
}


int FileName::exists(const char *name)
{
#if POSIX
    struct stat st;

    if (stat(name, &st) < 0)
        return 0;
    if (S_ISDIR(st.st_mode))
        return 2;
    return 1;
#elif _WIN32
    DWORD dw;
    int result;

    dw = GetFileAttributesA(name);
    if (dw == -1L)
        result = 0;
    else if (dw & FILE_ATTRIBUTE_DIRECTORY)
        result = 2;
    else
        result = 1;
    return result;
#else
    assert(0);
#endif
}

void FileName::ensurePathExists(const char *path)
{
    //printf("FileName::ensurePathExists(%s)\n", path ? path : "");
    if (path && *path)
    {
        if (!exists(path))
        {
            const char *p = FileName::path(path);
            if (*p)
            {
#if _WIN32
                size_t len = strlen(path);
                if ((len > 2 && p[-1] == ':' && strcmp(path + 2, p) == 0) ||
                    len == strlen(p))
                {   mem.free((void *)p);
                    return;
                }
#endif
                ensurePathExists(p);
                mem.free((void *)p);
            }
#if _WIN32
            char sep = '\\';
#elif POSIX
            char sep = '/';
#endif
            if (path[strlen(path) - 1] != sep)
            {
                //printf("mkdir(%s)\n", path);
#if _WIN32
                int r = _mkdir(path);
#endif
#if POSIX
                int r = mkdir(path, 0777);
#endif
                if (r)
                {
                    /* Don't error out if another instance of dmd just created
                     * this directory
                     */
                    if (errno != EEXIST)
                        error("cannot create directory %s", path);
                }
            }
        }
    }
}

void FileName::ensurePathToNameExists(const char *name)
{
    const char *pt = path(name);
    if (*pt)
        ensurePathExists(pt);
    free(pt);
}


/******************************************
 * Return canonical version of name in a malloc'd buffer.
 * This code is high risk.
 */
const char *FileName::canonicalName(const char *name)
{
#if __linux__
    // Lovely glibc extension to do it for us
    return canonicalize_file_name(name);
#elif POSIX
  #if _POSIX_VERSION >= 200809L || defined (__linux__)
    // NULL destination buffer is allowed and preferred
    return realpath(name, NULL);
  #else
    char *cname = NULL;
    #if PATH_MAX
        /* PATH_MAX must be defined as a constant in <limits.h>,
         * otherwise using it is unsafe due to TOCTOU
         */
        size_t path_max = (size_t)PATH_MAX;
        if (path_max > 0)
        {
            /* Need to add one to PATH_MAX because of realpath() buffer overflow bug:
             * http://isec.pl/vulnerabilities/isec-0011-wu-ftpd.txt
             */
            cname = (char *)malloc(path_max + 1);
            if (cname == NULL)
                return NULL;
        }
    #endif
    return realpath(name, cname);
  #endif
#elif _WIN32
    /* Apparently, there is no good way to do this on Windows.
     * GetFullPathName isn't it, but use it anyway.
     */
    DWORD result = GetFullPathName(name, 0, NULL, NULL);
    if (result)
    {
        char *buf = (char *)malloc(result);
        result = GetFullPathName(name, result, buf, NULL);
        if (result == 0)
        {
            ::free(buf);
            return NULL;
        }
        return buf;
    }
    return NULL;
#else
    assert(0);
    return NULL;
#endif
}

/********************************
 * Free memory allocated by FileName routines
 */
void FileName::free(const char *str)
{
    if (str)
    {   assert(str[0] != 0xAB);
        memset((void *)str, 0xAB, strlen(str) + 1);     // stomp
    }
    mem.free((void *)str);
}

char *FileName::toChars()
{
    return (char *)str;         // toChars() should really be const
}
