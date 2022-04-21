/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

/// \file
/// \brief \b Compresses/Decompresses ASCII strings and writes/reads them to BitStream class instances.  You can use this to easily serialize and deserialize your own strings.
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#pragma once

#include "bitstream.hpp"
#include "huffman_tree.hpp"

/// Forward declaration
namespace Encoding {

/// \brief Writes and reads strings to and from bitstreams.
///
/// Only works with ASCII strings.  The default compression is for English.
/// You can call GenerateTreeFromStrings to compress and decompress other languages efficiently as well.
class StringCompressor {
public:
    /// static function because only static functions can access static members
    /// The RakPeer constructor adds a reference to this class, so don't call this until an instance of RakPeer exists, or unless you call AddReference yourself.
    /// \return the unique instance of the StringCompressor
    static StringCompressor* Instance(void);

    /// Writes input to output, compressed.  Takes care of the null terminator for you.
    /// \param[in] input Pointer to an ASCII string
    /// \param[in] maxCharsToWrite The max number of bytes to write of \a input.  Use 0 to mean no limit.
    /// \param[out] output The bitstream to write the compressed string to
    /// \param[in] languageID Which language to use
    void EncodeString(const char* input, int maxCharsToWrite, NetworkBitStream* output);

    /// Writes input to output, uncompressed.  Takes care of the null terminator for you.
    /// \param[out] output A block of bytes to receive the output
    /// \param[in] maxCharsToWrite Size, in bytes, of \a output .  A NULL terminator will always be appended to the output string.  If the maxCharsToWrite is not large enough, the string will be truncated.
    /// \param[in] input The bitstream containing the compressed string
    bool DecodeString(char* output, int maxCharsToWrite, NetworkBitStream* input);

    /// Writes input to output, uncompressed.  Takes care of the null terminator for you.  Can be resumed by tracking stringBitLength.
    /// \param[out] output A block of bytes to receive the output
    /// \param[in] maxCharsToWrite Size, in bytes, of \a output .  A NULL terminator will always be appended to the output string.  If the maxCharsToWrite is not large enough, the string will be truncated.
    /// \param[in] input The bitstream containing the compressed string
    /// \param[in] stringBitLength How many bits were not previously decoded
    /// \param[in] skip If there is too much data to fit in the output, drop the remainder
    bool DecodeString(char* output, int maxCharsToWrite, NetworkBitStream* input, unsigned& stringBitLength, bool skip);

private:
    /// Private Constructor
    StringCompressor();

    /// Pointer to the huffman encoding trees.
    DataStructures::HuffmanEncodingTree huffmanEncodingTree;

    /// Singleton instance
    static StringCompressor* instance;
};
}

/// Define to more easily reference the string compressor instance.
/// The RakPeer constructor adds a reference to this class, so don't call this until an instance of RakPeer exists, or unless you call AddReference yourself.
#define stringCompressor Encoding::StringCompressor::Instance()
