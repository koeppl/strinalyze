//#include <chaiscript/chaiscript.hpp>

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/suffix_trees.hpp>
#include <string>
#include <sdsl/int_vector.hpp>
#include <iomanip>
//#include <sstring>
#include <sdsl/csa_wt.hpp>
#include <glog/logging.h>
#include "substring.hpp"
#include "index_iterator.hpp"
#include "checked_vector.hpp"

///
#include <gflags/gflags.h>

DEFINE_uint64(threads, 4, "Number of Threads");
DEFINE_string(examine, "", "String to examine");
DEFINE_bool(stripDollar, true, "Strip the delimiting character of the string");
DEFINE_bool(zeroBasedNumbering, true, "Start counting at zero for output");
///


using namespace sdsl;
using namespace std;

#define BOUNDS(x) x.begin(), x.end()

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

std::string intToString(uint64_t z) {
	const size_t length = static_cast<uint8_t>(std::log2(2+z));
	std::string ret(length,0);
	for(size_t i = 0; i < length; ++i)
		ret[i] =  (z & (1ULL<<i)) == 0 ? 'a' : 'b';
	return ret;
}

std::string lzclassic(size_t n) {
	if(n==1) return "a";
	if(n==2) return "b";
	if(n==3) return "aa";
	return lzclassic(n-2) + lzclassic(n-3) + lzclassic(n-2);
}

std::string lzl(size_t n) {
	if(n==1) return "a";
	if(n==2) return "b";
	if(n==3) return "a";
	if(n==4) return "aba";
	if(n==5) return "baaba";
	return lzl(n-2) + lzl(n-1);
}

std::string fibonacci(size_t n) {
	if( n == 1) return "b";
	if( n == 2) return "a";
	return fibonacci(n-1) + fibonacci(n-2);
}

#ifdef NDEBUG
typedef checked_vector<int> vektor_type;
#else
typedef vector<int> vektor_type;
//#define vektor_type sdsl::int_vector<>
#endif

typedef cst_sct3<> cst_t;

vektor_type create_sa(const char*const str, bool stripDollar) {
	csa_wt<> csa;
	sdsl::construct_im(csa, str, 1);
	vektor_type sa(csa.size() - stripDollar  );
	for (size_t i = 0; i < sa.size(); ++i) {
		sa[i] = csa[i + stripDollar ];
	}
	return sa;
}

vektor_type create_sa(const cst_t& cst, bool stripDollar) {
	vektor_type sa(cst.csa.size() - stripDollar );
	for (size_t i = 0; i < sa.size(); ++i) {
		sa[i] = cst.csa[i + stripDollar ];
	}
	return sa;
}

vektor_type create_lcp(const cst_t& cst, bool stripDollar) {
	vektor_type lcp(cst.csa.size() - stripDollar );
	for (size_t i = 0; i < lcp.size(); ++i) {
		lcp[i] = cst.lcp[i + stripDollar];
	}
	return lcp;
}

cst_t create_cst(const char*const str) {
	cst_t cst;
	sdsl::construct_im(cst, str, 1);
	return cst;
}

vektor_type inverse(const vektor_type& sa) {
	vektor_type isa(sa.size());
	for (size_t i = 0; i < sa.size(); ++i) {
		isa[sa[i]] = i;
	}
	return isa;
}

vektor_type psi_array(const vektor_type& sa, const vektor_type& isa) {
	vektor_type psi(sa.size());
	for (size_t i=0; i < sa.size(); ++i) {
		psi[i] = isa[((sa[i]+sa.size()-1) % sa.size()) ];
	}
	return psi;
}

/** Printing stuff **/
template<class T>
void print_array(size_t width, const char* label, const T& arr) {
	std::cout << std::setw(4) << label << " ";
	for(size_t i=0; i < arr.size(); ++i) {
		std::cout << std::setw(width) << arr[i] << " ";
	}
	std::cout << "\n";
}

template<class T>
void print_value(const char* label, const T& value) {
	std::cout << label << ": " << value << "\n";
}

template<class T>
class ArrayFunction {
	public:
	using class_type     = ArrayFunction<T>;
	using const_iterator = IndexIterator<class_type>;
	using value_type     = T;

	private:
	const size_t m_length;
	const std::function<size_t(size_t)> m_call;
	public:
	ArrayFunction(size_t length, std::function<T(size_t)> call) 
		: m_length(length),	m_call(call)
	{}
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

struct StringStats {
	const std::string text;
	const cst_t cst;
	const vektor_type sa;
	const vektor_type lcp;
	const vektor_type isa;
	const vektor_type psi;


	StringStats(const std::string&& ttext) 
		: text(ttext)
		, cst(create_cst(text.c_str()))
		, sa(create_sa(cst, FLAGS_stripDollar))
		, lcp(create_lcp(cst, FLAGS_stripDollar))
		, isa(inverse(sa))
		, psi(psi_array(sa, isa))
	{
	}
	size_t size() const {
		return sa.size();
	}
};


void print_analysis(const StringStats& stat, const bool isZeroBasedNumbering = true) {
	const size_t setwidth = static_cast<size_t>(std::log10(stat.text.length()+1)) +1;
	print_value("|T|", stat.text.size());
	print_array(setwidth, "i",   ArrayFunction<size_t>(stat.size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + i; } ));
	print_array(setwidth, "T",   stat.text);
	print_array(setwidth, "SA" , ArrayFunction<size_t>(stat.size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + stat.sa[i] ; } ));
	print_array(setwidth, "LCP", ArrayFunction<size_t>(stat.size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + stat.lcp[i]; } ));
	print_array(setwidth, "ISA", ArrayFunction<size_t>(stat.size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + stat.isa[i]; } ));
	print_array(setwidth, "psi", ArrayFunction<size_t>(stat.size(), [&] (size_t i) { return (isZeroBasedNumbering ? 0 : 1) + stat.psi[i]; } ));
	print_array(setwidth, "BWT", ArrayFunction<char>  (stat.size(), [&] (size_t i) { return stat.text[stat.sa[stat.psi[i]]]; } ));
	print_value("rotation_order", rotation_order(stat.sa,stat.isa));

	print_value("reverse_rotation_order", reverse_rotation_order(stat.sa, stat.isa)); 
/*	print_value("reverse_rotation_order", reverse_rotation_order( 
				Substring<vektor_type>(sa, 0, sa.size()-1),
				Substring<vektor_type>(isa, 0, isa.size()-1)));
*/
}



/*
#include<chrono>
#include<queue>
void generate() {
	constexpr size_t maxlen = 20;
	std::queue<std::string> q;
	std::queue<std::string> pq;
	std::thread threads[NUM_THREADS];
	std::mutex mu;
	std::mutex mu2;
	bool process = true;
	for(size_t i = 0; i < NUM_THREADS; ++i) {
//		threads[i] = std::thread(thread_func,mu, mu2, pq, process);
		threads[i] = std::thread([&] () {
	while(process || !pq.empty()) {
		if(pq.empty()) {
			std::this_thread::sleep_for (std::chrono::seconds(1));
			continue;
		}
		std::string text = [&] () {
			std::unique_lock<std::mutex> lock(mu2);
			if(pq.empty()) return std::string("");
			std::string ret = pq.front(); 
			pq.pop();
			return ret;
		}();
		if(text.length() == 0) return;
		const cst_t cst = create_cst(text.c_str());
		vektor_type sa = create_sa(cst);
		strip_dollar_from_sa(sa);
		vektor_type isa = inverse(sa);

		const int rot = rotation_order(sa,isa);
		if(rot > 0) {
			std::unique_lock<std::mutex> lock(mu);
			print_value("STRING", text);
			print_value("length", text.length());
			print_value("rotation_order", rot);
			print_value("a", std::count(BOUNDS(text), 'a'));
			print_value("b", std::count(BOUNDS(text), 'b'));
			print_analysis(text);
			continue;
		}

		const int invrot = reverse_rotation_order(sa,isa);
		if(invrot > 0) {
			std::unique_lock<std::mutex> lock(mu);
			print_value("STRING", text);
			print_value("length", text.length());
			print_value("inverse_rotation_order", invrot);
			print_value("a", std::count(BOUNDS(text), 'a'));
			print_value("b", std::count(BOUNDS(text), 'b'));
			continue;
		}
	}});
	}

	{
		std::unique_lock<std::mutex> lock(mu2);
		q.push("a");
		q.push("b");
		while(!q.empty()) {
			std::string text = q.front(); q.pop();
			if(text.length() < maxlen) {
				q.push(text + "a");
				q.push(text + "b");
				pq.push(text + "a");
				pq.push(text + "b");
			}
		}
	}
	
	for(size_t i = 0; i < NUM_THREADS; ++i)
		threads[i].join();
}
*/

/*
void run( std::string (*generator)(size_t)) {
	for(size_t iFibRound = 3; iFibRound < 15; iFibRound+=1) {

		print_value("==Round==", iFibRound);
		std::string s = generator(iFibRound);
		print_analysis(s, true, true);
		
		

	}
}
*/


#include<thread>
void parallel_filter(
		std::function<std::string(size_t)> generator, 
		const size_t left, const size_t right,
		std::function<bool(const StringStats&)> predicate, 
		std::function<void(const StringStats&)> output
		) {
	std::vector<std::thread> threads(FLAGS_threads);
	std::mutex mutexOutput;
	std::atomic_size_t index(left);
	auto runnable = [&] () {
		while(index.load() < right) {
			const StringStats stats = StringStats(generator(++index));
			if(stats.size() == 0) return;
			if(predicate(stats)) { 
				std::unique_lock<std::mutex> lock(mutexOutput);
				output(stats);
			}
		}};

	for(size_t i = 0; i < FLAGS_threads; ++i) {
		threads[i] = std::thread(runnable);
	}
	for(auto& thread : threads) {
		thread.join();
	}
}

namespace google {}
namespace gflags {}
int main(int argc, char **argv)
{
	{
		using namespace google;
		using namespace gflags;
//		SetUsageMessage(Summary);
		ParseCommandLineFlags(&argc, &argv, true);
	}
	if(!FLAGS_examine.empty()) {
		print_analysis(StringStats(std::move(FLAGS_examine)), FLAGS_zeroBasedNumbering);
		return EXIT_SUCCESS;
	}
	parallel_filter(
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

//run(lzclassic);
//	run(fibonacci);
//	run(lzl);
}
