/** 
 * @file checked_vector.hpp
 * @brief Out-of-bounds save std::vector implementation
 * @author Dominik Köppl
 * 
 * @date 2015-02-20
 */

#ifndef CHECKED_VECTOR
#define CHECKED_VECTOR

#include <vector>
#include <cstddef>

/** Derives std::vector and adds checks for out-of-bounds
 */
	template<class T>
	class checked_vector : public std::vector<T>
	{
		public:
		checked_vector(size_t _size) : std::vector<T>(_size) {}
		checked_vector(const checked_vector<T>& pol) : std::vector<T>(pol) {}
		checked_vector() : std::vector<T>() {}
//		checked_vector(typename std::vector<T>::size_type n, const typename std::vector<T>::value_type& val = std::vector<T>::value_type()) 
//			: std::vector<T>(n, val) { }
#ifndef NDEBUG
		T& operator[](size_t n) override { 
			DCHECK_LT(n, this->size());
			return std::vector<T>::at(n); 
		}
		const T& operator[](size_t n) const override { 
			DCHECK_LT(n, this->size());
			return std::vector<T>::at(n);
		}
#endif
	};

#endif//guard

