/// \file
/// \brief \b [Internal] Quicksort ordered list.
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

#include "DS_List.h"
#include "Export.h"

#ifndef __ORDERED_LIST_H
#define __ORDERED_LIST_H

namespace RakNet
{
	/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
	/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
	namespace DataStructures
	{
		template <class key_type, class data_type>
		int defaultOrderedListComparison(const key_type &a, const data_type &b)
		{
			if (a<b) return -1;
			if (a==b) return 0;
			return 1;
		}

		/// \note IMPORTANT! If you use defaultOrderedListComparison then call IMPLEMENT_DEFAULT_COMPARISON or you will get an unresolved external linker error.
		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)=defaultOrderedListComparison<key_type, data_type> >
		class RAK_DLL_EXPORT OrderedList
		{
		public:
			static void IMPLEMENT_DEFAULT_COMPARISON(void) {defaultOrderedListComparison<key_type, data_type>(key_type(),data_type());}

			OrderedList();
			~OrderedList();
			OrderedList( const OrderedList& original_copy );
			OrderedList& operator= ( const OrderedList& original_copy );

			/// comparisonFunction must take a key_type and a data_type and return <0, ==0, or >0
			/// If the data type has comparison operators already defined then you can just use defaultComparison
			bool HasData(const key_type &key) const;
			unsigned GetIndexFromKey(const key_type &key, bool *objectExists) const;
			data_type GetElementFromKey(const key_type &key);
			unsigned Insert(const key_type &key, const data_type &data);
			unsigned Remove(const key_type &key);
			data_type& operator[] ( const unsigned int position ) const;
			void RemoveAtIndex(const unsigned index);
			void InsertAtIndex(const data_type &data, const unsigned index);
			void InsertAtEnd(const data_type &data);
			void Del(const unsigned num=1);
			void Clear(void);		
			unsigned Size(void) const;

		protected:
			List<data_type> orderedList;
		};

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		OrderedList<key_type, data_type, comparison_function>::OrderedList()
		{
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		OrderedList<key_type, data_type, comparison_function>::~OrderedList()
		{
			Clear();
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		OrderedList<key_type, data_type, comparison_function>::OrderedList( const OrderedList& original_copy )
		{
			orderedList=original_copy.orderedList;
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		OrderedList<key_type, data_type, comparison_function>& OrderedList<key_type, data_type, comparison_function>::operator= ( const OrderedList& original_copy )
		{
			orderedList=original_copy.orderedList;
			return *this;
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		bool OrderedList<key_type, data_type, comparison_function>::HasData(const key_type &key) const
		{
			bool objectExists;
			unsigned index;
			index = GetIndexFromKey(key, &objectExists);
			return objectExists;
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		data_type OrderedList<key_type, data_type, comparison_function>::GetElementFromKey(const key_type &key)
		{
			bool objectExists;
			unsigned index;
			index = GetIndexFromKey(key, &objectExists);
			RakAssert(objectExists);
			return orderedList[index];
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		unsigned OrderedList<key_type, data_type, comparison_function>::GetIndexFromKey(const key_type &key, bool *objectExists) const
		{
			int index, upperBound, lowerBound;
			int res;

			if (orderedList.Size()==0)
			{
				*objectExists=false;
				return 0;
			}

			upperBound=(int)orderedList.Size()-1;
			lowerBound=0;
			index = (int)orderedList.Size()/2;

	#ifdef _MSC_VER
		#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
	#endif
			while (1)
			{
				res = comparison_function(key,orderedList[index]);
				if (res==0)
				{
					*objectExists=true;
					return index;
				}
				else if (res<0)
				{
					upperBound=index-1;
				}
				else// if (res>0)
				{
					lowerBound=index+1;
				}

				index=lowerBound+(upperBound-lowerBound)/2;

				if (lowerBound>upperBound)
				{
					*objectExists=false;
					return lowerBound; // No match
				}
			}
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		unsigned OrderedList<key_type, data_type, comparison_function>::Insert(const key_type &key, const data_type &data)
		{
			bool objectExists;
			unsigned index;
			index = GetIndexFromKey(key, &objectExists);

			// Don't allow duplicate insertion.
			if (objectExists)
				return (unsigned)-1;

			if (index>=orderedList.Size())
			{
				orderedList.Insert(data);
				return orderedList.Size()-1;
			}
			else
			{
				orderedList.Insert(data,index);
				return index;
			}		
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		unsigned OrderedList<key_type, data_type, comparison_function>::Remove(const key_type &key)
		{
			bool objectExists;
			unsigned index;
			index = GetIndexFromKey(key, &objectExists);

			// Can't find the element to remove if this assert hits
			RakAssert(objectExists==true);
			if (objectExists==false)
				return 0;

			orderedList.RemoveAtIndex(index);
			return index;
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		void OrderedList<key_type, data_type, comparison_function>::RemoveAtIndex(const unsigned index)
		{
			orderedList.RemoveAtIndex(index);
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
			void OrderedList<key_type, data_type, comparison_function>::InsertAtIndex(const data_type &data, const unsigned index)
		{
			orderedList.Insert(data, index);
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
			void OrderedList<key_type, data_type, comparison_function>::InsertAtEnd(const data_type &data)
		{
			orderedList.Insert(data);
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
			void OrderedList<key_type, data_type, comparison_function>::Del(const unsigned num)
		{
			orderedList.Del(num);
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		void OrderedList<key_type, data_type, comparison_function>::Clear(void)
		{
			orderedList.Clear();
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		data_type& OrderedList<key_type, data_type, comparison_function>::operator[]( const unsigned int position ) const
		{
			return orderedList[position];
		}

		template <class key_type, class data_type, int (*comparison_function)(const key_type&, const data_type&)>
		unsigned OrderedList<key_type, data_type, comparison_function>::Size(void) const
		{
			return orderedList.Size();
		}
	}
}

#endif
