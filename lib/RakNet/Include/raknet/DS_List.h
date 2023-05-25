/// \file
/// \brief \b [Internal] Array based list.  Usually the Queue class is used instead, since it has all the same functionality and is only worse at random access.
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

#ifndef __LIST_H
#define __LIST_H 

#include <assert.h>
#include <string.h> // memmove
#include "Export.h"
#include "RakAssert.h"

/// Maximum unsigned long
static const unsigned int MAX_UNSIGNED_LONG = 4294967295U;

namespace RakNet
{
	/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
	/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
	namespace DataStructures
	{
		/// \brief Array based implementation of a list.
		template <class list_type>
		class RAK_DLL_EXPORT List
		{	
		public:
			/// Default constructor
			List();

			/// Destructor
			~List();
			
			/// Copy constructor
			/// \param[in]  original_copy The list to duplicate 
			List( const List& original_copy );
			
			/// Assign one list to another
			List& operator= ( const List& original_copy );
			
			/// Access an element by its index in the array 
			/// \param[in]  position The index into the array. 
			/// \return The element at position \a position. 
			list_type& operator[] ( const unsigned int position ) const;
			
			/// Insert an element at position \a position in the list 
			/// \param[in] input The new element. 
			/// \param[in] position The position of the new element. 		
			void Insert( const list_type input, const unsigned int position );
			
			/// Insert at the end of the list.
			/// \param[in] input The new element. 
			void Insert( const list_type input );
			
			/// Replace the value at \a position by \a input.  If the size of
			/// the list is less than @em position, it increase the capacity of
			/// the list and fill slot with @em filler.
			/// \param[in] input The element to replace at position @em position. 
			/// \param[in] filler The element use to fill new allocated capacity. 
			/// \param[in] position The position of input in the list. 		
			void Replace( const list_type input, const list_type filler, const unsigned int position );
			
			/// Replace the last element of the list by \a input .
			/// \param[in] input The element used to replace the last element. 
			void Replace( const list_type input );
			
			/// Delete the element at position \a position. 
			/// \param[in] position The index of the element to delete 
			void RemoveAtIndex( const unsigned int position );
			
			/// Delete the element at the end of the list 
			void Del(const unsigned num=1);
			
			/// Returns the index of the specified item or MAX_UNSIGNED_LONG if not found
			/// \param[in] input The element to check for 
			/// \return The index or position of @em input in the list. 
			/// \retval MAX_UNSIGNED_LONG The object is not in the list
			/// \retval [Integer] The index of the element in the list
			unsigned int GetIndexOf( const list_type input );
			
			/// \return The number of elements in the list
			unsigned int Size( void ) const;
			
			/// Clear the list		
			void Clear( bool doNotDeallocate=false );
			
			/// Frees overallocated members, to use the minimum memory necessary
			/// \attention 
			/// This is a slow operation		
			void Compress( void );
			
		private:
			/// An array of user values
			list_type* listArray = nullptr;
			
			/// Number of elements in the list 		
			unsigned int list_size = 0;
			
			/// Size of \a array 		
			unsigned int allocation_size = 0;
		};

		template <class list_type>
			List<list_type>::List()
		{
			allocation_size = 0;
			listArray = 0;
			list_size = 0;
		}

		template <class list_type>
			List<list_type>::~List()
		{
			if (allocation_size>0)
				delete [] listArray;
		}

		template <class list_type>
			List<list_type>::List( const List& original_copy )
		{
			// Allocate memory for copy

			if ( original_copy.list_size == 0 )
			{
				list_size = 0;
				allocation_size = 0;
				listArray = 0;
			}
			else
			{
				listArray = new list_type [ original_copy.list_size ];

				//for ( unsigned int counter = 0; counter < original_copy.list_size; ++counter )
				//	listArray[ counter ] = original_copy.listArray[ counter ];

				// Don't call constructors, assignment operators, etc.
				memcpy(reinterpret_cast<void*>(listArray), original_copy.listArray, original_copy.list_size*sizeof(list_type));

				list_size = allocation_size = original_copy.list_size;
			}
		}

		template <class list_type>
			List<list_type>& List<list_type>::operator= ( const List& original_copy )
		{
			if ( ( &original_copy ) != this )
			{
				Clear();

				// Allocate memory for copy

				if ( original_copy.list_size == 0 )
				{
					list_size = 0;
					allocation_size = 0;
					listArray = 0;
				}

				else
				{
					listArray = new list_type [ original_copy.list_size ];

					//for ( unsigned int counter = 0; counter < original_copy.list_size; ++counter )
					//	listArray[ counter ] = original_copy.listArray[ counter ];
					// Don't call constructors, assignment operators, etc.
					memcpy(reinterpret_cast<void*>(listArray), original_copy.listArray, original_copy.list_size*sizeof(list_type));

					list_size = allocation_size = original_copy.list_size;
				}
			}

			return *this;
		}


		template <class list_type>
			inline list_type& List<list_type>::operator[] ( const unsigned int position ) const
		{
	#ifdef _DEBUG
			assert ( position < list_size );
	#endif
			return listArray[ position ];
		}

		template <class list_type>
			void List<list_type>::Insert( const list_type input, const unsigned int position )
		{

			RakAssert( position <= list_size );


			// Reallocate list if necessary
			if ( allocation_size == 0 )
			{
				allocation_size = 16;
				listArray = new list_type [ allocation_size ];
			}
			else if ( list_size == allocation_size )
			{
				// allocate twice the currently allocated memory
				allocation_size *= 2;
				list_type * new_array = new list_type [ allocation_size ];

				// Don't call constructors, assignment operators, etc.
				memcpy(reinterpret_cast<void*>(new_array), listArray, list_size*sizeof(list_type));

				// set old array to point to the newly allocated and twice as large array
				delete[] listArray;

				listArray = new_array;
			}

			// Don't call constructors, assignment operators, etc.
			memmove(reinterpret_cast<void*>(listArray+position+1), listArray+position, (list_size-position)*sizeof(list_type));

			// Insert the new item at the correct spot
			listArray[ position ] = input;

			++list_size;

		}


		template <class list_type>
			void List<list_type>::Insert( const list_type input )
		{
			// Reallocate list if necessary

			if ( allocation_size == 0 )
			{
				allocation_size = 16;
				listArray = new list_type [ allocation_size ];
			}
			else if ( list_size == allocation_size )
			{
				// allocate twice the currently allocated memory
				allocation_size *= 2;
				list_type * new_array = new list_type [ allocation_size ];

				// Don't call constructors, assignment operators, etc.
				memcpy(reinterpret_cast<void*>(new_array), listArray, list_size*sizeof(list_type));

				// set old array to point to the newly allocated and twice as large array
				delete[] listArray;

				listArray = new_array;
			}

			// Insert the new item at the correct spot
			listArray[ list_size ] = input;

			++list_size;
		}

		template <class list_type>
			inline void List<list_type>::Replace( const list_type input, const list_type filler, const unsigned int position )
		{
			if ( ( list_size > 0 ) && ( position < list_size ) )
			{
				// Direct replacement
				listArray[ position ] = input;
			}
			else
			{
				if ( position >= allocation_size )
				{
					// Reallocate the list to size position and fill in blanks with filler
					list_type * new_array;
					allocation_size = position + 1;

					new_array = new list_type [ allocation_size ];

					// copy old array over

					//for ( unsigned int counter = 0; counter < list_size; ++counter )
					//	new_array[ counter ] = listArray[ counter ];

					if (listArray)
					{
						// Don't call constructors, assignment operators, etc.
						memcpy(reinterpret_cast<void*>(new_array), listArray, list_size*sizeof(list_type));

						// set old array to point to the newly allocated array
						delete[] listArray;
					}

					listArray = new_array;
				}

				// Fill in holes with filler
				while ( list_size < position )
					listArray[ list_size++ ] = filler;

				// Fill in the last element with the new item
				listArray[ list_size++ ] = input;



				RakAssert( list_size == position + 1 );



			}
		}

		template <class list_type>
			inline void List<list_type>::Replace( const list_type input )
		{
			if ( list_size > 0 )
				listArray[ list_size - 1 ] = input;
		}

		template <class list_type>
			void List<list_type>::RemoveAtIndex( const unsigned int position )
		{

			RakAssert( position < list_size );


			if ( position < list_size )
			{
				// Compress the array
				/*
				for ( unsigned int counter = position; counter < list_size - 1 ; ++counter )
				listArray[ counter ] = listArray[ counter + 1 ];
				*/
				memmove(reinterpret_cast<void*>(listArray+position), listArray+position+1, (list_size-1-position) * sizeof(list_type));

				Del();
			}
		}

		template <class list_type>
			inline void List<list_type>::Del( const unsigned num )
		{
			// Delete the last elements on the list.  No compression needed

			RakAssert(list_size>=num);

			list_size-=num;
		}

		template <class list_type>
			unsigned int List<list_type>::GetIndexOf( const list_type input )
		{
			for ( unsigned int i = 0; i < list_size; ++i )
				if ( listArray[ i ] == input )
					return i;

			return MAX_UNSIGNED_LONG;
		}

		template <class list_type>
			inline unsigned int List<list_type>::Size( void ) const
		{
			return list_size;
		}

		template <class list_type>
			void List<list_type>::Clear( bool doNotDeallocate )
		{
			if ( allocation_size == 0 )
				return;

			if (allocation_size>512 && doNotDeallocate==false)
			{
				delete [] listArray;
				allocation_size = 0;
				listArray = 0;
			}
			list_size = 0;
		}

		template <class list_type>
			void List<list_type>::Compress( void )
		{
			list_type * new_array;

			if ( allocation_size == 0 )
				return ;

			new_array = new list_type [ allocation_size ];

			// Don't call constructors, assignment operators, etc.
			memcpy(reinterpret_cast<void*>(new_array), listArray, list_size*sizeof(list_type));

			// set old array to point to the newly allocated array
			delete[] listArray;

			listArray = new_array;
		}
		
	} // End namespace
}

#endif
