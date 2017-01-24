/** 
 * @file arrayfunctional.hpp
 * @brief Simulates an array by a function that takes an index
 * @author Dominik Köppl
 * 
 * @date 2015-02-20
 */

#ifndef ARRAYFUNCTIONAL_HPP
#define ARRAYFUNCTIONAL_HPP

#include "index_iterator.hpp"
/** 
 * Creates a container wrapper around a function.
 *
 * @brief The function m_call mimicks an array-like container that elements are 
 * instatenously and temporarily by a call of m_call
 *
 * @tparam T the return value 
 */
template<class T>
class ArrayFunctional {
	public:
	using class_type     = ArrayFunctional<T>;
	using const_iterator = IndexIterator<class_type>;
	using value_type     = T;

	private:
	const size_t m_length;
	const std::function<size_t(size_t)> m_call;
	public:

	/** 
	 * @param length The domain [0.. length] of the function passed
	 * @param call The function that generates the elements of this container
	 */
	ArrayFunctional(size_t length, std::function<T(size_t)> call) 
		: m_length(length),	m_call(call)
	{}

	/** 
	 * @sa std::vector::operator[]
	 */
	T operator[](size_t index) const {
		DCHECK_LT(index, m_length);
		return m_call(index);
	}
	size_t size() const {
		return m_length;
	}
	const_iterator begin() const {
		return IndexIterator<class_type>(*this, 0);
	}
	const_iterator end() const {
		return IndexIterator<class_type>(*this, m_length);
	}
	bool operator==(const class_type&) const {
		return true; // dummy
	}
};

#endif /* ARRAYFUNCTIONAL_HPP */
