#include <string>
#include <iomanip>
#include <glog/logging.h>
#include "substring.hpp"
#include "checked_vector.hpp"

//SAIS
#if defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#endif
#include "sais.hxx"
#if defined(__GNUG__)
#pragma GCC diagnostic pop
#endif

#include <cmath>
#include <iostream>
#include <functional>
#include <algorithm>
#include "index_iterator.hpp"
#include "arrayfunctional.hpp"

#include <gflags/gflags.h>
DEFINE_uint64(threads, 4, "Number of Threads");
DEFINE_uint64(minlimit, 0, "Starting Number of Sequence");
DEFINE_uint64(maxlimit, 1ULL<<40, "Ending Number of Sequence");
DEFINE_string(ex, "", "String to examine");
DEFINE_string(generator, "", "Use a generator sequence");
DEFINE_string(appendString, "", "Append a string to the sequence");
DEFINE_string(prependString, "", "Prepend a string to the sequence");
DEFINE_bool(stripDollar, false, "Strip the delimiting character of the string");
DEFINE_bool(zeroindex, false, "Start counting indices at zero");
///

#define BOUNDS(x) x.begin(), x.end()

/** 
 * Tests whether a is a rotation of b.
 *
 * @brief i.e., whether there exists a k such that \f$a[k+i \mod n] = b[i] \forall 0 \le i \le n \f$.
 * 
 * @param a Array a[0..n]
 * @param b Array b[0..n]
 * 
 * @return either the rotation-width [1,n] or -1 on error. Returns zero if a = b.
 */
template<class T>
int rotation_order(const T& a , const T& b) {
	typename T::const_iterator azerop = std::find(BOUNDS(a), 0);
	typename T::const_iterator bzerop = std::find(BOUNDS(b), 0);
	const size_t length = a.size();
	if(azerop == a.end() || bzerop == b.end()) return -1;
	const std::ptrdiff_t azero = std::distance(a.begin(), azerop);
	const std::ptrdiff_t bzero = std::distance(b.begin(), bzerop);
	for(size_t i = 0; i < length; ++i)
	{
		if(a[ (azero+i ) % length] != b[(bzero+i) % length]) return -1;
	}
	return (azero < bzero) ? (bzero - azero) : (bzero + length - azero);
}

/** 
 * Tests whether a is a reversed rotation of b.
 *
 * @brief i.e., whether there exists a k such that \f$a[k-i \mod n] = b[i] \forall 0 \le i \le n \f$.
 * 
 * @param a Array a[0..n]
 * @param b Array b[0..n]
 * 
 * @return either the rotation-width [1,n] or -1 on error.
 */
template<class T>
int reverse_rotation_order(const T& a , const T& b) {
	typename T::const_iterator azerop = std::find(BOUNDS(a), 0);
	typename T::const_iterator bzerop = std::find(BOUNDS(b), 0);
	const size_t length = a.size();
	if(azerop == a.end() || bzerop == b.end()) return -1;
	const std::ptrdiff_t azero = std::distance(a.begin(), azerop);
	const std::ptrdiff_t bzero = std::distance(b.begin(), bzerop);
	for(size_t i = 0; i < length; ++i)
	{
		if(a[ (azero+i ) % length] != b[(length+bzero-i) % length]) return -1;
	}
	return (azero < bzero) ? (bzero - azero) : (bzero + length - azero);
}

/**
 * Constructs the LPF array
 * @param sa the suffix array
 * @param lcp the LCP array
 * @return the LPF array
 * @author Crochemore et al., "LPF computation revisited", IWOCA'09
 */
template<class lpf_t, class lcp_t, class isa_t>
lpf_t create_lpf(const lcp_t& lcp, const isa_t& isa) {
	const int n = lcp.size();
	lpf_t lcpcopy(n+1);
	for(int i = 0; i < n; ++i) { lcpcopy[i] = lcp[i]; }
	lcpcopy[n] = 0;
	lpf_t prev(n);
	lpf_t next(n);
	lpf_t lpf(n);
	constexpr int undef { static_cast<int>(-1) };
	for(int r = 0; r < n; ++r) {
		prev[r] = r-1;
		next[r] = r+1;
	}
	DCHECK_EQ(prev[0], undef);
	for(int j = n; j > 0; --j) {
		const int i = j-1; DCHECK_GT(j,0);
		const int r = isa[i];
		lpf[i] = std::max(lcpcopy[r], lcpcopy[next[r]] );
		DCHECK_LT(next[r],n+1);

		lcpcopy[next[r]] = std::min(lcpcopy[r], lcpcopy[next[r]]);
		if(prev[r] != undef) { next[prev[r]] = next[r]; }
		if(next[r] < n) { prev[next[r]] = prev[r]; }
	}
	return lpf;
}

/** 
 * Creates the Suffix Array of text, based on SAIS
 * 
 * @param text 
 * @param stripDollar Shall the delimiting $ be neglected? If not, it is the lexicographically smallest suffix.
 * 
 * @return The suffix array
 */
template<typename vektor_type, typename string_type>
vektor_type create_sa(const string_type& text, bool stripDollar) {
	using namespace saisxx_private;
	vektor_type sa(text.size()+!stripDollar);
	saisxx<typename string_type::const_iterator, typename vektor_type::iterator, int>(text.begin(), sa.begin(), text.size()+!stripDollar);
	return sa;
}

/** 
 * @brief Kasai's LCP array construction algorithm
 * 
 * @param text 
 * @param sa text's suffix array, maybe a subarray of the actual suffix array
 * @param isa sa's inverse
 * 
 * @return LCP-array of text wrt. sa
 */
template<typename vektor_type, typename string_type>
vektor_type create_lcp(const string_type& text, const vektor_type& sa, const vektor_type& isa) {
	vektor_type lcp(sa.size());
	lcp[0] = 0;
	size_t h = 0;
	for(size_t i = 0; i < lcp.size(); ++i) {
		if(isa[i] == 0) continue;
		const size_t j = sa[ isa[i] -1 ];
		while(text[i+h] == text[j+h]) ++h;
		lcp[isa[i]] = h;
		h = h > 0 ? h-1 : 0;
	}
	return lcp;
}

/** Generates the inverse of an array
 */
template<typename vektor_type>
vektor_type inverse(const vektor_type& sa) {
	vektor_type isa(sa.size());
	for (size_t i = 0; i < sa.size(); ++i) {
		isa[sa[i]] = i;
	}
	return isa;
}


/** 
 * Generates \$f LF[i] = isa[ sa[i] - 1 \mod n] \forall 0 \le i \le n = \abs{sa} \$f
 * 
 * @param sa the suffix array
 * @param isa sa's inverse
 * 
 * @return LF-array
 */
template<class vektor_type>
vektor_type lf_array(const vektor_type& sa, const vektor_type& isa) {
	vektor_type LF(sa.size());
	for (size_t i=0; i < sa.size(); ++i) {
		LF[i] = isa[((sa[i]+sa.size()-1) % sa.size()) ];
	}
	return LF;
}

/** 
 * Generates \$f \psi[i] = isa[ sa[i] + 1 \mod n] \forall 0 \le i \le n = \abs{sa} \$f
 * 
 * @param sa the suffix array
 * @param isa sa's inverse
 * 
 * @return \$f \psi \$f-array
 */
template<class vektor_type>
vektor_type psi_array(const vektor_type& sa, const vektor_type& isa) {
	vektor_type psi(sa.size());
	for (size_t i=0; i < sa.size(); ++i) {
		psi[i] = isa[((sa[i]+sa.size()+1) % sa.size()) ];
	}
	return psi;
}

/** Printing stuff **/
template<class vektor_type>
void print_array(size_t width, const char* label, const vektor_type& arr) {
	std::cout << std::setw(4) << label << " ";
	for(size_t i=0; i < arr.size(); ++i) {
		std::cout << std::setw(width) << arr[i] << " ";
	}
	std::cout << "\n";
}
const char*const dollarSymbol = "$";

template<class vektor_type>
void print_value(const char* label, const vektor_type& value) {
	std::cout << (label == 0 ? dollarSymbol : label) << ": " << value << "\n";
}
void print_ending() {
	std::cout << "------------------" << "\n";
}




struct StringStats {
#ifdef NDEBUG
		typedef checked_vector<int> vektor_type;
#else
		typedef std::vector<int> vektor_type;
		//#define vektor_type sdsl::int_vector<>
#endif

	const std::string text;
//	const cst_t cst;
	const vektor_type sa;
	const vektor_type isa;
	const vektor_type lcp;
	const vektor_type lpf;
	const vektor_type psi;
	const vektor_type lf;


	StringStats(const std::string&& ttext) 
		: text(ttext)
		, sa(create_sa<vektor_type>(text, FLAGS_stripDollar))
		, isa(inverse<vektor_type>(sa))
		, lcp(create_lcp<vektor_type>(text, sa, isa))
		, lpf(create_lpf<vektor_type,vektor_type,vektor_type>(lcp, isa))
		, psi(psi_array<vektor_type>(sa, isa))
		, lf(lf_array<vektor_type>(sa, isa))
	{
	}
	size_t size() const {
		return sa.size();
	}
	void print(const bool isZeroBasedNumbering = true) const {
		const size_t setwidth = static_cast<size_t>(std::log10(text.length()+1)) +1;
		print_value("T", text);
		print_value("|T|", text.size());
		print_array(setwidth, "i",   ArrayFunctional<size_t>(size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + i; } ));
		if(FLAGS_stripDollar) {
			print_array(setwidth, "T",   text);
		} else {
			print_array(setwidth, "T", ArrayFunctional<char>  (size(), [&] (size_t i) { return text[i] == 0 ? '$' : text[i]; } ));
		}
		print_array(setwidth, "SA" , ArrayFunctional<size_t>(size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + sa[i] ; } ));
		print_array(setwidth, "LCP", ArrayFunctional<size_t>(size(), [&] (size_t i) { return lcp[i]; } ));
		print_array(setwidth, "PLCP", ArrayFunctional<size_t>(size(), [&] (size_t i) { return lcp[isa[i]]; } ));
		print_array(setwidth, "LPF", ArrayFunctional<size_t>(size(), [&] (size_t i) { return lpf[i]; } ));
		print_array(setwidth, "ISA", ArrayFunctional<size_t>(size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + isa[i]; } ));
		print_array(setwidth, "psi", ArrayFunctional<size_t>(size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + psi[i]; } ));
		print_array(setwidth, "LF", ArrayFunctional<size_t>(size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + lf[i]; } ));
		print_array(setwidth, "BWT", ArrayFunctional<char>  (size(), [&] (size_t i) { return text[sa[lf[i]]] == 0 ? '$' : text[sa[lf[i]]]; } ));
		print_value("a", std::count(BOUNDS(text), 'a'));
		print_value("b", std::count(BOUNDS(text), 'b'));
		print_value("rotation_order", rotation_order(sa,isa));
		print_value("reverse_rotation_order", reverse_rotation_order(sa, isa)); 
	}
};

#include <thread>
#include <mutex>
#include <atomic>
void map_parallel(
		std::function<std::string(size_t)> generator, 
		const size_t left, const size_t right,
		std::function<void(size_t, std::string&)> mapto
		) {
	std::vector<std::thread> threads(FLAGS_threads);
	std::atomic_size_t index(left);
	auto runnable = [&] () {
		while(index.load() < right) {
			const size_t mindex = ++index;
			std::string text = generator(mindex);
			mapto(mindex, text);
		}};

	for(size_t i = 0; i < FLAGS_threads; ++i) {
		threads[i] = std::thread(runnable);
	}
	for(auto& thread : threads) {
		thread.join();
	}
}


/**
 * Creates a standard word by a binary number.
 * We use the mappings
 * L(u,v) = (u,uv)
 * R(u,v) = (vu,v)
 * where we walk down a tree 
 * 0 = L, left child 
 * 1 = R, right child
 */
std::string intToStandardWord(uint64_t z) {
	const size_t length = static_cast<uint8_t>(std::log2(2+z));
	std::string u = "a";
	std::string v = "b";
	for(size_t i = 1; i < length; ++i) {
		if( (z & (1ULL<<i)) == 0) v = u + v; 
		else u = v + u;
	}
	
	return (z & 1) ? u : v;

	std::string ret(length,0);
	for(size_t i = 0; i < length; ++i)
		ret[i] =  (z & (1ULL<<i)) == 0 ? 'a' : 'b';
	return ret;
}

/**
 * Creates the binary string representation of z, stripping the most-significant bit.
 * 
 */
std::string intToString(uint64_t z) {
	const size_t length = static_cast<uint8_t>(std::log2(2+z));
	std::string ret(length,0);
	for(size_t i = 0; i < length; ++i)
		ret[i] =  (z & (1ULL<<i)) == 0 ? 'a' : 'b';
	return ret;
}

/** 
 * LZ77-Factorization of the Fibonacci words
 * This factorization is a palindromic factorization
 */
std::string fib_lz77(size_t n) {
	if(n==1) return "a";
	if(n==2) return "b";
	if(n==3) return "aa";
	return fib_lz77(n-2) + fib_lz77(n-3) + fib_lz77(n-2);
}

/** l-Factorization of the Fibonacci words
 */
std::string fib_lzl(size_t n) {
	if(n==1) return "a";
	if(n==2) return "b";
	if(n==3) return "a";
	if(n==4) return "aba";
	if(n==5) return "baaba";
	return fib_lzl(n-2) + fib_lzl(n-1);
}

std::string fibonacci_word(size_t n) {
	if( n == 1) return "b";
	if( n == 2) return "a";
	return fibonacci_word(n-1) + fibonacci_word(n-2);
}
std::string rabbit_sequence(size_t n) {
	if( n == 1) return "a";
	if( n == 2) return "b";
	return rabbit_sequence(n-1) + rabbit_sequence(n-2);
}

/** 
 * Prepends an arbitrary string in front of the generated sequence.
 */
class Prepender {
	const std::function<std::string(size_t)> m_func;
	const std::string& m_data;
	public:
	Prepender(std::function<std::string(size_t)>&& func, const std::string& s) 
		: m_func(func), m_data(s) {}
	std::string operator()(size_t n) {
		return m_data + m_func(n);
	}
};

/** 
 * Appends an arbitrary string in front of the generated sequence.
 */
class Appender {
	const std::function<std::string(size_t)> m_func;
	const std::string& m_data;
	public:
	Appender(std::function<std::string(size_t)>&& func, const std::string& s) 
		: m_func(func), m_data(s) {}
	std::string operator()(size_t n) {
		return m_func(n) + m_data;
	}
};


constexpr const char*const usage_message = "You need to provide either a string with -ex or a string generator with -g.";

namespace google {}
namespace gflags {}
void help(const char*const prgname) {
		using namespace google;
		using namespace gflags;
        std::vector<CommandLineFlagInfo> info;
        GetAllFlags(&info);

        std::cout << prgname << " (options) {-ex (string) | -g [frls7] }" << std::endl;
        std::cout << usage_message << std::endl << std::endl;
        std::cout
            << std::setw(20) << std::setiosflags(std::ios::left) << "Parameter"
            << std::setw(10) << "Type"
            << std::setw(20) << "Default"
            << std::setw(20) << "Current"
            << "Description" << std::endl;
        std::cout << std::endl;
         for(auto it = info.cbegin(); it != info.cend(); ++it) {
             if(it->filename != __FILE__) continue;
                 std::cout
                    << std::setw(20) << std::setiosflags(std::ios::left)<< (std::string("--")+ it->name)
                    << std::setw(10) << it->type
                    << std::setw(20) << (std::string("(") + it->default_value + ")")
                    << std::setw(20) << (std::string("(") + it->current_value + ")")
                    << it->description << std::endl;
         }
}


int main(int argc, char **argv)
{
	if(argc == 1 || strcmp(argv[1],"-h") == 0 || strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-help") == 0) {
		help(argv[0]);
		return 0;
	}
	{
		using namespace google;
		using namespace gflags;
		SetUsageMessage(usage_message);
		ParseCommandLineFlags(&argc, &argv, true);
	}
	if(!FLAGS_ex.empty()) {
		StringStats(std::move(FLAGS_ex)).print(FLAGS_zeroindex);
		return EXIT_SUCCESS;
	}
	std::function<std::string(size_t)> generator = intToString;
	if(!FLAGS_generator.empty()) {
		switch(FLAGS_generator.at(0)) {
			case 'f':
				generator = fibonacci_word;
				break;
			case 'r':
				generator = rabbit_sequence;
				break;
			case 'l':
				generator = fib_lzl;
				break;
			case 's':
				generator = intToStandardWord;
				break;
			case '7':
				generator = fib_lz77;
				break;
			default:
				help(argv[0]);
				return 0;
				break;
		}
	} else {
		help(argv[0]);
		return 0;
	}

	if(!FLAGS_appendString.empty())
		generator = Appender(std::move(generator), FLAGS_appendString);
	if(!FLAGS_prependString.empty())
		generator = Prepender(std::move(generator), FLAGS_prependString);

	std::mutex mutexOutput;
	map_parallel(
			generator,
			FLAGS_minlimit,
			FLAGS_maxlimit,
			[&mutexOutput] (size_t index, std::string& str) {
				if(str.empty()) return;
				const StringStats stats = StringStats(std::move(str));
				if(stats.size() == 0) return;
				// const int rotation = rotation_order(stats.sa,stats.isa);
				// const int reverse_rotation = reverse_rotation_order(stats.sa,stats.isa);
			//	if(rotation >= 0 || reverse_rotation >= 0) 
				{
					std::unique_lock<std::mutex> lock(mutexOutput);
					print_value("index", index);
					stats.print(FLAGS_zeroindex);
					print_ending();
				}

			/* 
				if(str.empty()) return;
				const StringStats stats = StringStats(std::move(str));
				if(stats.size() == 0) return;
				const auto& sa = stats.sa;
				const size_t q = sa[0];
				const size_t m = (sa.size() + sa[1]) - sa[0];
				for(size_t i = 2; i < sa.size(); ++i)
					if(sa[i] != static_cast<int>( (sa[i-1]+m) % sa.size()) ) return;
				const int rotation = rotation_order(stats.sa,stats.isa);
				if(rotation >= 0)
				{
					std::unique_lock<std::mutex> lock(mutexOutput);
					stats.print(FLAGS_zeroindex);
					print_value("q", q);
					print_value("m", m);
					print_ending();
				}
				*/
			});

	/*
	map_parallel(
			intToString,
			0, 1ULL<<20,
			[] (const StringStats& stats) { //predicate
				return rotation_order(stats.sa,stats.isa) >= 0;
			},
			[] (const StringStats& stats) { // output
				const int rotation = rotation_order(stats.sa,stats.isa);
				const int reverse_rotation = reverse_rotation_order(stats.sa,stats.isa);
				const std::string& text = stats.text;
				print_value("STRING", text);
				print_value("length", stats.sa.size());
				print_value("rotation_order", rotation);
				print_value("inverse_rotation_order", reverse_rotation);
				print_value("a", std::count(BOUNDS(text), 'a'));
				print_value("b", std::count(BOUNDS(text), 'b'));
			});
*/
//run(lzclassic);
//	run(fibonacci);
//	run(lzl);
}
