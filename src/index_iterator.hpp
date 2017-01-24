/** 
 * @file index_iterator.hpp
 * @brief Iterator based on an index value
 * @author Dominik Köppl
 * 
 */
#ifndef INDEX_ITERATOR_HPP
#define INDEX_ITERATOR_HPP


/*!
 * Iterator that is an actual wrapper around an index. 
 * Can be further enhanced as a RandomAccessIterator.
 * @tparam T container class with operator[]
 */
template<class T>
class IndexIterator : public std::iterator<std::forward_iterator_tag, T>
{
	public:
	typedef typename T::value_type value_type;
	private:
	const T& m_parent;
	size_t m_index;
	public:
		IndexIterator(const T& parent, size_t index = 0) : m_parent(parent), m_index(index) {}
		IndexIterator(const IndexIterator& mit) : m_parent(mit.m_parent), m_index(mit.m_index) {}
		IndexIterator& operator++() {
			++m_index; 
			DCHECK_LE(m_index, m_parent.size()); //LE for end()
			return *this; 
		}
		IndexIterator operator++(int) {
			IndexIterator tmp(*this); 
			operator++(); 
			return tmp;
		}
		value_type operator*() {return m_parent[m_index];}
		bool operator==(const IndexIterator& rhs) {return m_parent == rhs.m_parent && m_index==rhs.m_index;}
		bool operator!=(const IndexIterator& rhs) {return !(*this == rhs);}
};

#endif /* INDEX_ITERATOR_HPP */
