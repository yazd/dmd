
// Copyright (c) 1999-2011 by Digital Mars
// All Rights Reserved
// written by Walter Bright
// http://www.digitalmars.com
// License for redistribution is by either the Artistic License
// in artistic.txt, or the GNU General Public License in gnu.txt.
// See the included readme.txt for details.

#ifndef FILE_H
#define FILE_H

#if __DMC__
#pragma once
#endif

#include <stddef.h>

#include "array.h"

template <typename TYPE> struct Array;
typedef Array<struct File> Files;

struct FileName;

struct File
{
    int ref;                    // != 0 if this is a reference to someone else's buffer
    unsigned char *buffer;      // data for our file
    size_t len;                 // amount of data in buffer[]
    void *touchtime;            // system time to use for file

    FileName *name;             // name of our file

    File(const char *);
    File(const FileName *);
    ~File();

    char *toChars();

    /* Read file, return !=0 if error
     */

    int read();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void readv();

    /* Read file, return !=0 if error
     */

    int mmread();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void mmreadv();

    /* Write file, return !=0 if error
     */

    int write();

    /* Write file, either succeed or fail
     * with error message & exit.
     */

    void writev();

    /* Return !=0 if file exists.
     *  0:      file doesn't exist
     *  1:      normal file
     *  2:      directory
     */

    /* Append to file, return !=0 if error
     */

    int append();

    /* Append to file, either succeed or fail
     * with error message & exit.
     */

    void appendv();

    /* Return !=0 if file exists.
     *  0:      file doesn't exist
     *  1:      normal file
     *  2:      directory
     */

    int exists();

    /* Given wildcard filespec, return an array of
     * matching File's.
     */

    static Files *match(char *);
    static Files *match(FileName *);

    // Compare file times.
    // Return   <0      this < f
    //          =0      this == f
    //          >0      this > f
    int compareTime(File *f);

    // Read system file statistics
    void stat();

    /* Set buffer
     */

    void setbuffer(void *buffer, size_t len)
    {
        this->buffer = (unsigned char *)buffer;
        this->len = len;
    }

    void checkoffset(size_t offset, size_t nbytes);

    void remove();              // delete file
};

#endif
