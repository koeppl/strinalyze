#ifndef SUBSTRING_HPP
#define SUBSTRING_HPP

#include <cstddef>
#include <ostream>

template<class T>
class Substring {
	const T& m_v;
	const size_t m_begin;
	const size_t m_end;

	public:
	typedef typename T::size_type size_type;
	typedef typename T::value_type value_type;
	typedef typename T::reference reference;
	typedef typename T::const_reference const_reference;
	typedef typename T::iterator        iterator;
	typedef typename T::const_iterator  const_iterator;

	Substring(const T& v, size_t begin, size_t end) : m_v(v), m_begin(begin), m_end(end) 
	{
		DCHECK_GT(m_end, m_begin) << "Bounds mismatch";
		DCHECK_LE(m_end, m_v.size()) << "Bounds [" << begin << "," << end << "] do not match bounds of v with size " << v.size();
	}
	Substring(const Substring<T>& v, size_t begin, size_t end) : m_v(v.m_v), m_begin(begin+v.m_begin), m_end(end+v.m_begin) 
	{
		DCHECK_GT(m_end, m_begin) << "Bounds mismatch";
		DCHECK_LE(m_end, m_v.size()) << "Bounds [" << begin << "," << end << "] do not match bounds of v with size " << v.size();
	}

	const iterator begin() {
		return m_v.begin() + m_begin;
	}

	const iterator end() {
		return m_v.begin() + m_end;
	}

	const const_iterator begin() const {
		return m_v.begin() + m_begin;
	}

	const const_iterator end() const {
		return m_v.begin() + m_end;
	}



	size_type size() const {
		return m_end - m_begin; 
	}
	inline reference operator[](const size_type& i) {
		DCHECK_LT(i+m_begin, m_v.size()) << "i=" << i << "not in bounds [" << m_begin << "," << m_end << "]" << m_v.size();
		return m_v[m_begin+i];
	}
	inline const_reference operator[](const size_type& i) const {
		DCHECK_LT(i+m_begin, m_v.size()) << "i=" << i << "not in bounds [" << m_begin << "," << m_end << "]" << m_v.size();
		return m_v[m_begin+i];
	}
};

template<class t_bv>
//inline typename std::enable_if<std::is_same<typename t_bv::index_category ,bv_tag>::value, std::ostream&>::type
std::ostream& 
operator<<(std::ostream& os, const Substring<t_bv>& sbv) {
	for(const auto& a : sbv) os << a;
    return os;
}

#endif /* SUBSTRING_HPP */
