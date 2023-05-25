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

#include "DataBlockEncryptor.h"
#include "CheckSum.h"
#include "GetTime.h"
#include "Rand.h"
#include <assert.h>
#include <string.h>
#include "rijndael.h"
#include "Types.h"
#include "RakAssert.h"

using namespace RakNet;

DataBlockEncryptor::DataBlockEncryptor()
{
	keySet = false;
}

DataBlockEncryptor::~DataBlockEncryptor()
{}

bool DataBlockEncryptor::IsKeySet( void ) const
{
	return keySet;
}

void DataBlockEncryptor::SetKey( const unsigned char key[ 16 ] )
{
	keySet = true;
	//secretKeyAES128.set_key( key );
	makeKey(&keyEncrypt, DIR_ENCRYPT, 16, (char*)key);
	makeKey(&keyDecrypt, DIR_DECRYPT, 16, (char*)key);
	cipherInit(&cipherInst, MODE_ECB, 0); // ECB is not secure except that I chain manually farther down.
}

void DataBlockEncryptor::UnsetKey( void )
{
	keySet = false;
}

void DataBlockEncryptor::Encrypt( unsigned char *input, int inputLength, unsigned char *output, int *outputLength )
{
	unsigned index, byteIndex, lastBlock;
	unsigned int checkSum;
	unsigned char paddingBytes;
	unsigned char encodedPad;
	unsigned char randomChar;
	CheckSum checkSumCalculator;



	RakAssert( keySet );


	RakAssert( input && inputLength );


	// randomChar will randomize the data so the same data sent twice will not look the same
	randomChar = (unsigned char) randomMT();

	// 16-(((x-1) % 16)+1)

	// # of padding bytes is 16 -(((input_length + extra_data -1) % 16)+1)
	paddingBytes = (unsigned char) ( 16 - ( ( ( inputLength + sizeof( randomChar ) + sizeof( checkSum ) + sizeof( encodedPad ) - 1 ) % 16 ) + 1 ) );

	// Randomize the pad size variable
	encodedPad = (unsigned char) randomMT();
	encodedPad <<= 4;
	encodedPad |= paddingBytes;

	*outputLength = inputLength + sizeof( randomChar ) + sizeof( checkSum ) + sizeof( encodedPad ) + paddingBytes;

	// Write the data first, in case we are overwriting ourselves

	if ( input == output )
		memmove( output + sizeof( checkSum ) + sizeof( randomChar ) + sizeof( encodedPad ) + paddingBytes, input, inputLength );
	else
		memcpy( output + sizeof( checkSum ) + sizeof( randomChar ) + sizeof( encodedPad ) + paddingBytes, input, inputLength );

	// Write the random char
	memcpy( output + sizeof( checkSum ), ( char* ) & randomChar, sizeof( randomChar ) );

	// Write the pad size variable
	memcpy( output + sizeof( checkSum ) + sizeof( randomChar ), ( char* ) & encodedPad, sizeof( encodedPad ) );

	// Write the padding
	for ( index = 0; index < paddingBytes; index++ )
		*( output + sizeof( checkSum ) + sizeof( randomChar ) + sizeof( encodedPad ) + index ) = (unsigned char) randomMT();

	// Calculate the checksum on the data
	checkSumCalculator.Add( output + sizeof( checkSum ), inputLength + sizeof( randomChar ) + sizeof( encodedPad ) + paddingBytes );

	checkSum = checkSumCalculator.Get();

	// Write checksum
#ifdef HOST_ENDIAN_IS_BIG
	output[0] = checkSum&0xFF;
	output[1] = (checkSum>>8)&0xFF;
	output[2] = (checkSum>>16)&0xFF;
	output[3] = (checkSum>>24)&0xFF;
#else
	memcpy( output, ( char* ) & checkSum, sizeof( checkSum ) );
#endif

	// AES on the first block
//	secretKeyAES128.encrypt16( output );
	blockEncrypt(&cipherInst, &keyEncrypt, output, 16, output);

	lastBlock = 0;

	// Now do AES on every other block from back to front
	for ( index = *outputLength - 16; index >= 16; index -= 16 )
	{
		for ( byteIndex = 0; byteIndex < 16; byteIndex++ )
			output[ index + byteIndex ] ^= output[ lastBlock + byteIndex ];

		//secretKeyAES128.encrypt16( output + index );
		blockEncrypt(&cipherInst, &keyEncrypt, output+index, 16, output+index);

		lastBlock = index;
	}
}

bool DataBlockEncryptor::Decrypt( unsigned char *input, int inputLength, unsigned char *output, int *outputLength )
{
	unsigned index, byteIndex;
	unsigned int checkSum;
	unsigned char paddingBytes;
	unsigned char encodedPad;
	unsigned char randomChar;
	CheckSum checkSumCalculator;


	RakAssert( keySet );


	if ( input == 0 || inputLength < 16 || ( inputLength % 16 ) != 0 )
	{
		return false;
	}

	// Unchain in reverse order
	for ( index = 16; ( int ) index <= inputLength - 16;index += 16 )
	{
	//	secretKeyAES128.decrypt16( input + index );
		blockDecrypt(&cipherInst, &keyDecrypt, input + index, 16, input + index);

		for ( byteIndex = 0; byteIndex < 16; byteIndex++ )
		{
			if ( index + 16 == ( unsigned ) inputLength )
				input[ index + byteIndex ] ^= input[ byteIndex ];
			else
				input[ index + byteIndex ] ^= input[ index + 16 + byteIndex ];
		}
	};

	// Decrypt the first block
	//secretKeyAES128.decrypt16( input );
	blockDecrypt(&cipherInst, &keyDecrypt, input, 16, input);

	// Read checksum
#ifdef HOST_ENDIAN_IS_BIG
	checkSum = (unsigned int)input[0] | (unsigned int)(input[1]<<8) |
		(unsigned int)(input[2]<<16)|(unsigned int)(input[3]<<24);
#else
	memcpy( ( char* ) & checkSum, input, sizeof( checkSum ) );
#endif

	// Read the pad size variable
	memcpy( ( char* ) & encodedPad, input + sizeof( randomChar ) + sizeof( checkSum ), sizeof( encodedPad ) );

	// Ignore the high 4 bytes
	paddingBytes = encodedPad & 0x0F;


	// Get the data length
	*outputLength = inputLength - sizeof( randomChar ) - sizeof( checkSum ) - sizeof( encodedPad ) - paddingBytes;

	// Calculate the checksum on the data.
	checkSumCalculator.Add( input + sizeof( checkSum ), *outputLength + sizeof( randomChar ) + sizeof( encodedPad ) + paddingBytes );

	if ( checkSum != checkSumCalculator.Get() )
		return false;

	// Read the data
	if ( input == output )
		memmove( output, input + sizeof( randomChar ) + sizeof( checkSum ) + sizeof( encodedPad ) + paddingBytes, *outputLength );
	else
		memcpy( output, input + sizeof( randomChar ) + sizeof( checkSum ) + sizeof( encodedPad ) + paddingBytes, *outputLength );


	return true;
}
