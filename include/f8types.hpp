//-------------------------------------------------------------------------------------------------
#if 0

fix8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _F8_TYPES_HPP_
#define _F8_TYPES_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8 {

typedef std::string f8String;
typedef std::ostringstream f8ostrstream;

const unsigned char default_field_separator(0x1);

//-------------------------------------------------------------------------------------------------
/// Fast map for statically generated data types. Assumes table is sorted. Complexity is O(logN).
/*! \tparam Key the key
    \tparam Val the value */
template<typename Key, typename Val>
class GeneratedTable
{
	/// A pair structure to statically declare the data set
	struct Pair
	{
		Key _key;
		Val _value;

		/// Sort functor
		struct Less
		{
			bool operator()(const Pair &p1, const Pair &p2) const { return p1._key < p2._key; }
		};
	};

	/// The actual data set
	static const Pair _pairs[];

	/// The number of elements in the data set
	static const size_t _pairsz;

	typedef Val NotFoundType;
	/// The value to return when the key is not found
	static const NotFoundType _noval;

	typedef typename std::pair<const Pair *, const Pair *> PResult;

public:
	/*! Find a key (reference). If not found, throw InvalidMetadata.
	  Ye Olde Binary Chop
	  \param key the key to find
	  \return value found (reference) */
	static const Val& find_ref(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		if (res.first != res.second)
			return res.first->_value;
		static const std::string error_str("Invalid metadata or entry not found");
		throw InvalidMetadata(error_str);
	}

	/*! Find a key (value).
	  \param key the key to find
	  \return value found (value) or _noval if not found */
	static const Val find_val(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? res.first->_value : _noval;
	}

	/*! Find a key (pointer).
	  \param key the key to find
	  \return value found (pointer) or 0 if not found */
	static const Val *find_ptr(const Key& key)
	{
		const Pair what = { key };
		PResult res(std::equal_range (_pairs, _pairs + _pairsz, what, typename Pair::Less()));
		return res.first != res.second ? &res.first->_value : 0;
	}

	///Ctor.
	GeneratedTable() {}

	/*! Get the number of elements in the data set.
	  \return number of elements */
	static size_t size() { return _pairsz; }
};

//-------------------------------------------------------------------------------------------------
/// A specialised map to enable native static initalisation.
/*! \tparam Key the key
   \tparam Val the value
   \tparam Compare the comparitor */
template<typename Key, typename Val, typename Compare=std::less<Key> >
struct StaticTable
{
	typedef typename std::map<Key, Val, Compare> TypeMap;
	typedef typename TypeMap::value_type TypePair;
	typedef Val NotFoundType;

	/// The actual data set
	static const TypePair _valueTable[];

	/// The container
	static const TypeMap _valuemap;

	/// The value to return when the key is not found
	static const NotFoundType _noval;

	/// Ctor.
	StaticTable() {}

	/*! Find a key (value).
	  \param key the key to find
	  \return value found (value) or _noval if not found */
	static const Val find_value(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}

	/*! Find a key (reference).
	  \param key the key to find
	  \return value found (reference) or _noval if not found */
	static const Val& find_ref(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? itr->second : _noval;
	}

	/*! Find a key (pointer).
	  \param key the key to find
	  \return value found (pointer) or 0 if not found */
	static const Val *find_ptr(const Key& key)
	{
		typename TypeMap::const_iterator itr(_valuemap.find(key));
		return itr != _valuemap.end() ? &itr->second : 0;
	}

	/*! Get the number of elements in the data set.
	  \return number of elements */
	static size_t get_count() { return _valuemap.size(); }

	/*! Get a pointer to the end of the data set. Used as an end inputiterator for initialisation.
	  \return pointer to end of table */
	static const TypePair *get_table_end() { return _valueTable + sizeof(_valueTable)/sizeof(TypePair); }
};

//-------------------------------------------------------------------------------------------------
/// Presorted set designed for infrequent insertions but super fast initialisation from a sorted static array.
/// Search complexity is O(logN), ctor complexity approaches O(1), insert is O(N).
/*! \tparam K the key
    \tparam T the value type
    \tparam Comp the comparitor */
template<typename K, typename T, typename Comp>
class presorted_set
{
public:
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef std::pair<iterator, bool> result;

private:
	const size_t _reserve;
	size_t _sz, _rsz;
	T *_arr;

	typedef std::pair<iterator, iterator> internal_result;
	typedef std::pair<const_iterator, const_iterator> const_internal_result;

	/*! Calculate the amount of space to reserve in set
	  \param sz number of elements currently in set; if 0 retun reserve elements as size to reserve
	  \param res percentage of sz to keep in reserve
	  \return number of elements to additionally reserve (at least 1) */
	static size_t calc_reserve(size_t sz, size_t res)
	{
		if (!sz)  // special case - reserve means number to reserve, not %
			return res;
		const size_t val(sz * res / 100);
		return val ? val : 1;
	}

public:
	/*! ctor - initialise from static sorted set
	  \param arr_start pointer to start of static array to copy elements from
	  \param sz number of elements in set to copy
	  \param reserve percentage of sz to keep in reserve */
	presorted_set(const_iterator arr_start, const size_t sz, const size_t reserve=RESERVE_PERCENT) : _reserve(reserve),
		_sz(sz), _rsz(_sz + calc_reserve(_sz, _reserve)), _arr(new T[_rsz])
			{ memcpy(_arr, arr_start, _sz * sizeof(T)); }

	/*! ctor - initialise an empty set; defer memory allocation;
	  \param sz number of elements to initially allocate
	  \param reserve percentage of sz to keep in reserve */
	explicit presorted_set(const size_t sz=0, const size_t reserve=RESERVE_PERCENT) : _reserve(reserve),
		_sz(sz), _rsz(_sz + calc_reserve(_sz, _reserve)), _arr() {}

	/// dtor
	~presorted_set() { delete[] _arr; }

	/*! Find an element with the given value
	  \param what element to find
	  \param answer true if element is found
	  \return pointer to found element or pointer to location where element would be inserted */
	iterator find(const T what, bool& answer)
	{
		const internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		answer = res.first != res.second;
		return res.first;
	}

	/*! Find an element with the given key
	  \param key to find
	  \param answer true if element is found
	  \return pointer to found element or pointer to location where element would be inserted */
	iterator find(const K key, bool& answer) { return find(T(key), answer); }

	/*! Find an element with the given key (const version)
	  \param key to find
	  \return pointer to found element or end() */
	const_iterator find(const K key) const { return find(T(key)); }

	/*! Find an element with the given value (const version)
	  \param what element to find
	  \return pointer to found element or end() */
	const_iterator find(const T what) const
	{
		const const_internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		return res.first != res.second ? res.first : end();
	}

	/*! Find an element with the given key
	  \param key to find
	  \return pointer to found element or end() */
	iterator find(const K key) { return find(T(key)); }

	/*! Find an element with the given value
	  \param what value to find
	  \return pointer to found element or end() */
	iterator find(const T what)
	{
		internal_result res(std::equal_range (_arr, _arr + _sz, what, Comp()));
		return res.first != res.second ? res.first : end();
	}

	/*! Insert an element into the set
	  \param what pointer to element to insert
	  \return result with iterator to insert location and true or end() and false */
	result insert(const_iterator what)
	{
		if (!_sz)
		{
			_arr = new T[_rsz];
			memcpy(_arr, what, sizeof(T));
			++_sz;
			return result(_arr, true);
		}

		bool answer;
		iterator where(find(*what, answer));
		if (answer)
			return result(end(), false);

		if (_sz < _rsz)
		{
			memmove(where + 1, where, (end() - where) * sizeof(T));
			memcpy(where, what, sizeof(T));
		}
		else
		{
			iterator new_arr(new T[_rsz = _sz + calc_reserve(_sz, _reserve)]);
			size_t wptr(where - _arr);
			if (wptr > 0)
				memcpy(new_arr, _arr, sizeof(T) * wptr);
			memcpy(new_arr + wptr, what, sizeof(T));
			memcpy(new_arr + wptr + 1, where, (end() - where) * sizeof(T));
			delete[] _arr;
			_arr = new_arr;
		}
		++_sz;
		return result(where, true);
	}

	/*! Find the distance between two iterators
	  \param what_begin start iterator
	  \param what_end end iterator (must be >= to what_begin
	  \return distance in elements */
	static size_t distance(const_iterator what_begin, const_iterator what_end)
		{ return what_end - what_begin; }

	/*! Insert a range of elements into the set
	  \param what_begin pointer to 1st element to insert
	  \param what_end pointer to nth element + 1 to insert */
	void insert(const_iterator what_begin, const_iterator what_end)
	{
		for (const_iterator ptr(what_begin); ptr < what_end; ++ptr)
			if (!insert(ptr).second)
				break;
	}

	/*! Clear the set (does not delete) */
	void clear() { _sz = 0; }

	/*! Obtain the number of elements in the set
	  \return the number of elements */
	size_t size() const { return _sz; }

	/*! Check if the set is empty
	  \return true if empty */
	bool empty() const { return _sz == 0; }

	/*! Obtain the number of elements that can be inserted before reallocating
	  \return reserved + sz */
	size_t rsize() const { return _rsz; }

	/*! Get a pointer to the first element
	  \return the first element */
	iterator begin() { return _arr; }

	/*! Get a pointer to the last element + 1
	  \return the last element + 1 */
	iterator end() { return _arr + _sz; }

	/*! Get a const pointer to the first element
	  \return the first element */
	const_iterator begin() const { return _arr; }

	/*! Get a const pointer to the last element + 1
	  \return the last element + 1 */
	const_iterator end() const { return _arr + _sz; }
};

} // FIX8

#endif // _F8_TYPES_HPP_
