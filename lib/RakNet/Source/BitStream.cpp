/// \file
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

#if defined(_MSC_VER) && _MSC_VER < 1299 // VC6 doesn't support template specialization
#include "BitStream_NoTemplate.cpp"
#else

#include "BitStream.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>
#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h"
#elif defined(_WIN32)
#include <winsock2.h> // htonl
#else
#include <arpa/inet.h>
#endif

// Was included for memset which now comes from string.h instead
/*
#if defined ( __APPLE__ ) || defined ( __APPLE_CC__ )
	#include <malloc/malloc.h>
#elif !defined(_COMPATIBILITY_2)
	#include <malloc.h>
#endif

	*/

// MSWin uses _copysign, others use copysign...
#ifndef _WIN32
#define _copysign copysign
#endif

using namespace RakNet;

#ifdef _MSC_VER
#pragma warning( push )
#endif

BitStream::BitStream()
{
	numberOfBitsUsed = 0;
	//numberOfBitsAllocated = 32 * 8;
	numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE * 8;
	readOffset = 0;
	//data = ( unsigned char* ) malloc( 32 );
	data = ( unsigned char* ) stackData;
	
	
//	RakAssert( data );

	//memset(data, 0, 32);
	copyData = true;
}

BitStream::BitStream( int initialBytesToAllocate )
{
	numberOfBitsUsed = 0;
	readOffset = 0;
	if (initialBytesToAllocate <= BITSTREAM_STACK_ALLOCATION_SIZE)
	{
		data = ( unsigned char* ) stackData;
		numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE * 8;
	}
	else
	{
		data = ( unsigned char* ) malloc( initialBytesToAllocate );
		numberOfBitsAllocated = initialBytesToAllocate << 3;
	}

	RakAssert( data );

	// memset(data, 0, initialBytesToAllocate);
	copyData = true;
}

BitStream::BitStream( unsigned char* _data, unsigned int lengthInBytes, bool _copyData )
{
	numberOfBitsUsed = lengthInBytes << 3;
	readOffset = 0;
	copyData = _copyData;
	numberOfBitsAllocated = lengthInBytes << 3;
	
	if ( copyData )
	{
		if ( lengthInBytes > 0 )
		{
			if (lengthInBytes < BITSTREAM_STACK_ALLOCATION_SIZE)
			{
				data = ( unsigned char* ) stackData;
				numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE << 3;
			}
			else
			{
				data = ( unsigned char* ) malloc( lengthInBytes );
			}

			RakAssert( data );

			memcpy( data, _data, lengthInBytes );
		}
		else
			data = 0;
	}
	else
		data = ( unsigned char* ) _data;
}

// Use this if you pass a pointer copy to the constructor (_copyData==false) and want to overallocate to prevent reallocation
void BitStream::SetNumberOfBitsAllocated( const unsigned int lengthInBits )
{

	RakAssert( lengthInBits >= ( unsigned int ) numberOfBitsAllocated );
	
	numberOfBitsAllocated = lengthInBits;
}

BitStream::~BitStream()
{
	if ( copyData && numberOfBitsAllocated > BITSTREAM_STACK_ALLOCATION_SIZE << 3)
		free( data );  // Use realloc and free so we are more efficient than delete and new for resizing
}

void BitStream::Reset( void )
{
	// Note:  Do NOT reallocate memory because BitStream is used
	// in places to serialize/deserialize a buffer. Reallocation
	// is a dangerous operation (may result in leaks).
	
	if ( numberOfBitsUsed > 0 )
	{
		//  memset(data, 0, BITS_TO_BYTES(numberOfBitsUsed));
	}
	
	// Don't free memory here for speed efficiency
	//free(data);  // Use realloc and free so we are more efficient than delete and new for resizing
	numberOfBitsUsed = 0;
	
	//numberOfBitsAllocated=8;
	readOffset = 0;
	
	//data=(unsigned char*)malloc(1);
	// if (numberOfBitsAllocated>0)
	//  memset(data, 0, BITS_TO_BYTES(numberOfBitsAllocated));
}

// Write an array or casted stream
void BitStream::Write( const char* input, const int numberOfBytes )
{
	if (numberOfBytes==0)
		return;

	// Optimization:
	if ((numberOfBitsUsed & 7) == 0)
	{
		AddBitsAndReallocate( BYTES_TO_BITS(numberOfBytes) );
		memcpy(data+BITS_TO_BYTES(numberOfBitsUsed), input, numberOfBytes);
		numberOfBitsUsed+=BYTES_TO_BITS(numberOfBytes);
	}
	else
	{
		WriteBits( ( unsigned char* ) input, numberOfBytes * 8, true );
	}
	
}
void BitStream::Write( BitStream *bitStream)
{
	Write(bitStream, bitStream->GetNumberOfBitsUsed());
}
void BitStream::Write( BitStream *bitStream, int numberOfBits )
{
	if (numberOfBits > bitStream->GetNumberOfUnreadBits())
		return;

	AddBitsAndReallocate( numberOfBits );
	int numberOfBitsMod8;

	while (numberOfBits-->0)
	{
		numberOfBitsMod8 = numberOfBitsUsed & 7;
		if ( numberOfBitsMod8 == 0 )
		{
			// New byte
			if (bitStream->data[ bitStream->readOffset >> 3 ] & ( 0x80 >> ( bitStream->readOffset % 8 ) ) )
			{
				// Write 1
				data[ numberOfBitsUsed >> 3 ] = 0x80;
			}
			else
			{
				// Write 0
				data[ numberOfBitsUsed >> 3 ] = 0;
			}
			++bitStream->readOffset;
		}
		else
		{
			// Existing byte
			if (bitStream->data[ bitStream->readOffset >> 3 ] & ( 0x80 >> ( bitStream->readOffset % 8 ) ) )
				data[ numberOfBitsUsed >> 3 ] |= 0x80 >> ( numberOfBitsMod8 ); // Set the bit to 1
			// else 0, do nothing
			++bitStream->readOffset;
		}

		numberOfBitsUsed++;
	}
}

// Read an array or casted stream
bool BitStream::Read( char* output, const int numberOfBytes )
{
	// Optimization:
	if ((readOffset & 7) == 0)
	{
		if (GetNumberOfUnreadBits() < (numberOfBytes << 3))
			return false;

		// Write the data
		memcpy( output, data + ( readOffset >> 3 ), numberOfBytes );

		readOffset += numberOfBytes << 3;
		return true;
	}
	else
	{
		return ReadBits( ( unsigned char* ) output, numberOfBytes * 8 );
	}
}

// Sets the read pointer back to the beginning of your data.
void BitStream::ResetReadPointer( void )
{
	readOffset = 0;
}

// Sets the write pointer back to the beginning of your data.
void BitStream::ResetWritePointer( void )
{
	numberOfBitsUsed = 0;
}

// Write a 0
void BitStream::Write0( void )
{
	AddBitsAndReallocate( 1 );
	
	// New bytes need to be zeroed
	if ( ( numberOfBitsUsed & 7 ) == 0 )
		data[ numberOfBitsUsed >> 3 ] = 0;
		
	numberOfBitsUsed++;
}

// Write a 1
void BitStream::Write1( void )
{
	AddBitsAndReallocate( 1 );
	
	int numberOfBitsMod8 = numberOfBitsUsed & 7;
	
	if ( numberOfBitsMod8 == 0 )
		data[ numberOfBitsUsed >> 3 ] = 0x80;
	else
		data[ numberOfBitsUsed >> 3 ] |= 0x80 >> ( numberOfBitsMod8 ); // Set the bit to 1
		
	numberOfBitsUsed++;
}

#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // warning C4100: <variable name> : unreferenced formal parameter
#endif
// Returns true if the next data read is a 1, false if it is a 0
bool BitStream::ReadBit( void )
{
	if (GetNumberOfUnreadBits() == 0) {
		return false;
	}

	bool res = ( bool ) ( data[ readOffset >> 3 ] & ( 0x80 >> ( readOffset & 7 ) ) );
	++readOffset;
	return res;
}

// Align the bitstream to the byte boundary and then write the specified number of bits.
// This is faster than DoWriteBits but wastes the bits to do the alignment and requires you to call
// SetReadToByteAlignment at the corresponding read position
void BitStream::WriteAlignedBytes( const unsigned char* input,
	const int numberOfBytesToWrite )
{

	RakAssert( numberOfBytesToWrite > 0 );

	
	AlignWriteToByteBoundary();
	Write((const char*) input, numberOfBytesToWrite);
}

// Read bits, starting at the next aligned bits. Note that the modulus 8 starting offset of the
// sequence must be the same as was used with DoWriteBits. This will be a problem with packet coalescence
// unless you byte align the coalesced packets.
bool BitStream::ReadAlignedBytes( unsigned char* output, const int numberOfBytesToRead )
{

	RakAssert( numberOfBytesToRead > 0 );

	
	if ( numberOfBytesToRead <= 0 )
		return false;
		
	// Byte align
	AlignReadToByteBoundary();

	if (GetNumberOfUnreadBits() < (numberOfBytesToRead << 3))
		return false;

	// Write the data
	memcpy( output, data + ( readOffset >> 3 ), numberOfBytesToRead );
	
	readOffset += numberOfBytesToRead << 3;
	
	return true;
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignWriteToByteBoundary( void )
{
	if ( numberOfBitsUsed )
		numberOfBitsUsed += 8 - ( (( numberOfBitsUsed - 1 ) & 7) + 1 );
}

// Align the next write and/or read to a byte boundary.  This can be used to 'waste' bits to byte align for efficiency reasons
void BitStream::AlignReadToByteBoundary( void )
{
	if ( readOffset )
		readOffset += 8 - ( (( readOffset - 1 ) & 7 ) + 1 );
}

// Write numberToWrite bits from the input source
void BitStream::WriteBits( const unsigned char *input, int numberOfBitsToWrite, const bool rightAlignedBits )
{
	if (numberOfBitsToWrite<=0)
		return;
	
	AddBitsAndReallocate( numberOfBitsToWrite );
	int offset = 0;
	unsigned char dataByte;
	int numberOfBitsUsedMod8;
	
	numberOfBitsUsedMod8 = numberOfBitsUsed & 7;
	
	// Faster to put the while at the top surprisingly enough
	while ( numberOfBitsToWrite > 0 )
		//do
	{
		dataByte = *( input + offset );
		
		if ( numberOfBitsToWrite < 8 && rightAlignedBits )   // rightAlignedBits means in the case of a partial byte, the bits are aligned from the right (bit 0) rather than the left (as in the normal internal representation)
			dataByte <<= 8 - numberOfBitsToWrite;  // shift left to get the bits on the left, as in our internal representation
			
		// Writing to a new byte each time
		if ( numberOfBitsUsedMod8 == 0 )
			* ( data + ( numberOfBitsUsed >> 3 ) ) = dataByte;
		else
		{
			// Copy over the new data.
			*( data + ( numberOfBitsUsed >> 3 ) ) |= dataByte >> ( numberOfBitsUsedMod8 ); // First half
			
			if ( 8 - ( numberOfBitsUsedMod8 ) < 8 && 8 - ( numberOfBitsUsedMod8 ) < numberOfBitsToWrite )   // If we didn't write it all out in the first half (8 - (numberOfBitsUsed%8) is the number we wrote in the first half)
			{
				*( data + ( numberOfBitsUsed >> 3 ) + 1 ) = (unsigned char) ( dataByte << ( 8 - ( numberOfBitsUsedMod8 ) ) ); // Second half (overlaps byte boundary)
			}
		}
		
		if ( numberOfBitsToWrite >= 8 )
			numberOfBitsUsed += 8;
		else
			numberOfBitsUsed += numberOfBitsToWrite;
		
		numberOfBitsToWrite -= 8;
		
		offset++;
	}
	// } while(numberOfBitsToWrite>0);
}

// Set the stream to some initial data.  For internal use
void BitStream::SetData( unsigned char *input )
{
	data=input;
	copyData=false;
}

// Assume the input source points to a native type, compress and write it
void BitStream::WriteCompressed( const unsigned char* input,
	const int size, const bool unsignedData )
{
	int currentByte = ( size >> 3 ) - 1; // PCs
	
	unsigned char byteMatch;
	
	if ( unsignedData )
	{
		byteMatch = 0;
	}
	
	else
	{
		byteMatch = 0xFF;
	}
	
	// Write upper bytes with a single 1
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit. Otherwise write a 0 bit and then write the remaining bytes
	while ( currentByte > 0 )
	{
		if ( input[ currentByte ] == byteMatch )   // If high byte is byteMatch (0 of 0xff) then it would have the same value shifted
		{
			bool b = true;
			Write( b );
		}
		else
		{
			// Write the remainder of the data after writing 0
			bool b = false;
			Write( b );
			
			WriteBits( input, ( currentByte + 1 ) << 3, true );
			//  currentByte--;
			
			
			return ;
		}
		
		currentByte--;
	}
	
	// If the upper half of the last byte is a 0 (positive) or 16 (negative) then write a 1 and the remaining 4 bits.  Otherwise write a 0 and the 8 bites.
	if ( ( unsignedData && ( ( *( input + currentByte ) ) & 0xF0 ) == 0x00 ) ||
		( unsignedData == false && ( ( *( input + currentByte ) ) & 0xF0 ) == 0xF0 ) )
	{
		bool b = true;
		Write( b );
		WriteBits( input + currentByte, 4, true );
	}
	
	else
	{
		bool b = false;
		Write( b );
		WriteBits( input + currentByte, 8, true );
	}
}

// Read numberOfBitsToRead bits to the output source
// alignBitsToRight should be set to true to convert internal bitstream data to userdata
// It should be false if you used DoWriteBits with rightAlignedBits false
bool BitStream::ReadBits( unsigned char* output, int numberOfBitsToRead, const bool alignBitsToRight )
{

	RakAssert( numberOfBitsToRead > 0 );

	if (numberOfBitsToRead<=0)
	  return false;
	
	if (GetNumberOfUnreadBits() < numberOfBitsToRead)
		return false;
		
	int readOffsetMod8;
	
	int offset = 0;
	
	memset( output, 0, BITS_TO_BYTES( numberOfBitsToRead ) );
	
	readOffsetMod8 = readOffset & 7;
	
	// do
	// Faster to put the while at the top surprisingly enough
	while ( numberOfBitsToRead > 0 )
	{
		*( output + offset ) |= *( data + ( readOffset >> 3 ) ) << ( readOffsetMod8 ); // First half
		
		if ( readOffsetMod8 > 0 && numberOfBitsToRead > 8 - ( readOffsetMod8 ) )   // If we have a second half, we didn't read enough bytes in the first half
			*( output + offset ) |= *( data + ( readOffset >> 3 ) + 1 ) >> ( 8 - ( readOffsetMod8 ) ); // Second half (overlaps byte boundary)
			
		numberOfBitsToRead -= 8;
		
		if ( numberOfBitsToRead < 0 )   // Reading a partial byte for the last byte, shift right so the data is aligned on the right
		{
		
			if ( alignBitsToRight )
				* ( output + offset ) >>= -numberOfBitsToRead;
				
			readOffset += 8 + numberOfBitsToRead;
		}
		else
			readOffset += 8;
			
		offset++;
		
	}
	
	//} while(numberOfBitsToRead>0);
	
	return true;
}

// Assume the input source points to a compressed native type. Decompress and read it
bool BitStream::ReadCompressed( unsigned char* output,
	const int size, const bool unsignedData )
{
	int currentByte = ( size >> 3 ) - 1;
	
	
	unsigned char byteMatch, halfByteMatch;
	
	if ( unsignedData )
	{
		byteMatch = 0;
		halfByteMatch = 0;
	}
	
	else
	{
		byteMatch = 0xFF;
		halfByteMatch = 0xF0;
	}
	
	// Upper bytes are specified with a single 1 if they match byteMatch
	// From high byte to low byte, if high byte is a byteMatch then write a 1 bit. Otherwise write a 0 bit and then write the remaining bytes
	while ( currentByte > 0 )
	{
		// If we read a 1 then the data is byteMatch.
		
		bool b;
		
		if ( Read( b ) == false )
			return false;
			
		if ( b )   // Check that bit
		{
			output[ currentByte ] = byteMatch;
			currentByte--;
		}
		else
		{
			// Read the rest of the bytes
			
			if ( ReadBits( output, ( currentByte + 1 ) << 3 ) == false )
				return false;
				
			return true;
		}
	}
	
	// All but the first bytes are byteMatch.  If the upper half of the last byte is a 0 (positive) or 16 (negative) then what we read will be a 1 and the remaining 4 bits.
	// Otherwise we read a 0 and the 8 bytes
	//RakAssert(readOffset+1 <=numberOfBitsUsed); // If this assert is hit the stream wasn't long enough to read from
	if ( readOffset + 1 > numberOfBitsUsed )
		return false;
		
	bool b;
	
	if ( Read( b ) == false )
		return false;
		
	if ( b )   // Check that bit
	{
	
		if ( ReadBits( output + currentByte, 4 ) == false )
			return false;
			
		output[ currentByte ] |= halfByteMatch; // We have to set the high 4 bits since these are set to 0 by ReadBits
	}
	else
	{
		if ( ReadBits( output + currentByte, 8 ) == false )
			return false;
	}
	
	return true;
}

// Reallocates (if necessary) in preparation of writing numberOfBitsToWrite
void BitStream::AddBitsAndReallocate( const int numberOfBitsToWrite )
{
	if (numberOfBitsToWrite <= 0)
		return;

	int newNumberOfBitsAllocated = numberOfBitsToWrite + numberOfBitsUsed;
	
	if ( numberOfBitsToWrite + numberOfBitsUsed > 0 && ( ( numberOfBitsAllocated - 1 ) >> 3 ) < ( ( newNumberOfBitsAllocated - 1 ) >> 3 ) )   // If we need to allocate 1 or more new bytes
	{

		// If this assert hits then we need to specify true for the third parameter in the constructor
		// It needs to reallocate to hold all the data and can't do it unless we allocated to begin with
		RakAssert( copyData == true );


		// Less memory efficient but saves on news and deletes
		newNumberOfBitsAllocated = ( numberOfBitsToWrite + numberOfBitsUsed ) * 2;
//		int newByteOffset = BITS_TO_BYTES( numberOfBitsAllocated );
		// Use realloc and free so we are more efficient than delete and new for resizing
		int amountToAllocate = BITS_TO_BYTES( newNumberOfBitsAllocated );
		if (data==(unsigned char*)stackData)
		{
			 if (amountToAllocate > BITSTREAM_STACK_ALLOCATION_SIZE)
			 {
				 data = ( unsigned char* ) malloc( amountToAllocate );

				 // need to copy the stack data over to our new memory area too
				 memcpy ((void *)data, (void *)stackData, BITS_TO_BYTES( numberOfBitsAllocated )); 
			 }
		}
		else
		{
			data = ( unsigned char* ) realloc( data, amountToAllocate );
		}


		RakAssert( data ); // Make sure realloc succeeded

		//  memset(data+newByteOffset, 0,  ((newNumberOfBitsAllocated-1)>>3) - ((numberOfBitsAllocated-1)>>3)); // Set the new data block to 0
	}
	
	if ( newNumberOfBitsAllocated > numberOfBitsAllocated )
		numberOfBitsAllocated = newNumberOfBitsAllocated;
}

// Should hit if reads didn't match writes
void BitStream::AssertStreamEmpty( void )
{
	RakAssert( readOffset == numberOfBitsUsed );
}

void BitStream::PrintBits( void ) const
{
	if ( numberOfBitsUsed <= 0 )
	{
		printf( "No bits\n" );
		return ;
	}
	
	for ( int counter = 0; counter < BITS_TO_BYTES( numberOfBitsUsed ); counter++ )
	{
		int stop;
		
		if ( counter == ( numberOfBitsUsed - 1 ) >> 3 )
			stop = 8 - ( ( ( numberOfBitsUsed - 1 ) & 7 ) + 1 );
		else
			stop = 0;
			
		for ( int counter2 = 7; counter2 >= stop; counter2-- )
		{
			if ( ( data[ counter ] >> counter2 ) & 1 )
				putchar( '1' );
			else
				putchar( '0' );
		}
		
		putchar( ' ' );
	}
	
	putchar( '\n' );
}


// Exposes the data for you to look at, like PrintBits does.
// Data will point to the stream.  Returns the length in bits of the stream.
int BitStream::CopyData( unsigned char** _data ) const
{

	RakAssert( numberOfBitsUsed > 0 );

	
	*_data = new unsigned char [ BITS_TO_BYTES( numberOfBitsUsed ) ];
	memcpy( *_data, data, sizeof(unsigned char) * ( BITS_TO_BYTES( numberOfBitsUsed ) ) );
	return numberOfBitsUsed;
}

// Ignore data we don't intend to read
void BitStream::IgnoreBits( const int numberOfBits )
{
	readOffset += numberOfBits;
}

// Move the write pointer to a position on the array.  Dangerous if you don't know what you are doing!
void BitStream::SetWriteOffset( const int offset )
{
	numberOfBitsUsed = offset;
}

/*
int BitStream::GetWriteOffset( void ) const
{
	return numberOfBitsUsed;
}

// Returns the length in bits of the stream
int BitStream::GetNumberOfBitsUsed( void ) const
{
	return GetWriteOffset();
}

// Returns the length in bytes of the stream
int BitStream::GetNumberOfBytesUsed( void ) const
{
	return BITS_TO_BYTES( numberOfBitsUsed );
}

// Returns the number of bits into the stream that we have read
int BitStream::GetReadOffset( void ) const
{
	return readOffset;
}


// Sets the read bit index
void BitStream::SetReadOffset( int newReadOffset )
{
	readOffset=newReadOffset;
}

// Returns the number of bits left in the stream that haven't been read
int BitStream::GetNumberOfUnreadBits( void ) const
{
	return numberOfBitsUsed - readOffset;
}
// Exposes the internal data
unsigned char* BitStream::GetData( void ) const
{
	return data;
}

*/
// If we used the constructor version with copy data off, this makes sure it is set to on and the data pointed to is copied.
void BitStream::AssertCopyData( void )
{
	if ( copyData == false )
	{
		copyData = true;
		
		if ( numberOfBitsAllocated > 0 )
		{
			unsigned char * newdata = ( unsigned char* ) malloc( BITS_TO_BYTES( numberOfBitsAllocated ) );

			
			RakAssert( data );

			
			memcpy( newdata, data, BITS_TO_BYTES( numberOfBitsAllocated ) );
			data = newdata;
		}
		
		else
			data = 0;
	}
}
void BitStream::ReverseBytes(unsigned char *input, unsigned char *output, int length)
{
	for (int i=0; i < length; i++)
		output[i]=input[length-i-1];
}
bool BitStream::DoEndianSwap(void) const
{
#ifndef __BITSTREAM_NATIVE_END
	static bool swap=htonl(12345) == 12345;
	return swap;
#else
	return false;
#endif
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // #if _MSC_VER < 1299 
