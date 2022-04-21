/*
 *  This Source Code Form is subject to the terms of the Mozilla Public License,
 *  v. 2.0. If a copy of the MPL was not distributed with this file, You can
 *  obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  The original code is copyright (c) 2022, open.mp team and contributors.
 */

/// \file
/// \brief \b [Internal] Generates a huffman encoding tree, used for string and global compression.
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
#include "huffman_tree_node.hpp"
#include <list>

namespace Encoding {
namespace DataStructures {
    /// This generates special cases of the huffman encoding tree using 8 bit keys with the additional condition that unused combinations of 8 bits are treated as a frequency of 1
    class HuffmanEncodingTree {

    public:
        HuffmanEncodingTree();
        ~HuffmanEncodingTree();

        /// Pass an array of bytes to array and a preallocated BitStream to receive the output
        /// \param [in] input Array of bytes to encode
        /// \param [in] sizeInBytes size of \a input
        /// \param [out] output The bitstream to write to
        void EncodeArray(unsigned char* input, unsigned sizeInBytes, NetworkBitStream* output);

        // Decodes an array encoded by EncodeArray()
        unsigned DecodeArray(NetworkBitStream* input, unsigned& sizeInBits, unsigned maxCharsToWrite, unsigned char* output, bool skip = true);
        void DecodeArray(unsigned char* input, unsigned sizeInBits, NetworkBitStream* output);

        /// Given a frequency table of 256 elements, all with a frequency of 1 or more, generate the tree
        void GenerateFromFrequencyTable(unsigned int frequencyTable[256]);

        /// Free the memory used by the tree
        void FreeMemory(void);

    private:
        /// The root node of the tree

        HuffmanEncodingTreeNode* root;

        /// Used to hold bit encoding for one character

        struct CharacterEncoding {
            unsigned char* encoding;
            unsigned short bitLength;
        };

        CharacterEncoding encodingTable[256];

        void InsertNodeIntoSortedList(HuffmanEncodingTreeNode* node, std::list<HuffmanEncodingTreeNode*>& huffmanEncodingTreeNodeList) const;
    };
}
}
