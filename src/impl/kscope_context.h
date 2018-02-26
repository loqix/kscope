/*

Copyright (c) 2018, ITHare.com
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef ITHARE_KSCOPE_SEED

namespace ithare { namespace kscope {

	//forward declarations
	template<class T, class Context, class InjectionRequirements,ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	class KscopeInjection;

	template<class T, ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	class KscopeLiteralContext;
	
	template<size_t which, class T, ITHARE_KSCOPE_SEEDTPARAM seed>
	struct KscopeLiteralContextVersion;


	template<class T, T C, class Context, ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	class KscopeLiteralFromContext {
		static_assert(std::is_integral<T>::value);
		static_assert(std::is_unsigned<T>::value);

		struct InjectionRequirements {
			static constexpr size_t exclude_version = size_t(-1);
			static constexpr bool is_constexpr = true;
			static constexpr bool only_bijections = false;
			static constexpr bool no_substrate_size_increase = false;
		};
		using Injection = KscopeInjection<T, Context, InjectionRequirements,ITHARE_KSCOPE_NEW_PRNG(seed, 1), cycles>;
	public:
		ITHARE_KSCOPE_FORCEINLINE constexpr KscopeLiteralFromContext() : val(Injection::template injection<ITHARE_KSCOPE_NEW_PRNG(seed, 2),kscope_flag_is_constexpr>(C)) {
		}
		ITHARE_KSCOPE_FORCEINLINE constexpr T value() const {
			return Injection::template surjection<ITHARE_KSCOPE_NEW_PRNG(seed, 3),kscope_flag_is_constexpr>(val);
		}

#ifdef ITHARE_KSCOPE_DBG_ENABLE_DBGPRINT
		static void dbg_print(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "KscopeLiteralFromContext<" << kscope_dbg_print_t<T>() << "," << kscope_dbg_print_c<T>(C) << "," << kscope_dbg_print_seed<seed>() << "," << cycles << ">" << std::endl;
			Injection::dbg_print(offset + 1);
		}
		static void dbgCheck() {
			typename Injection::return_type c = Injection::template injection<seed,0>(C);
			T cc = Injection::template surjection<seed,0>(c);
			assert(cc == C);
		}
#endif
	private:
		typename Injection::return_type val;
	};		

	template<class T>
	struct KscopeZeroLiteralContext {
		//to be used as a context for literals-created-from-literals
		using Type = T;
		constexpr static KSCOPECYCLES context_cycles = 0;
		constexpr static KSCOPECYCLES calc_cycles(KSCOPECYCLES inj, KSCOPECYCLES surj) {
			return surj;//for literals, ONLY surjection costs apply in runtime (as injection applies in compile-time)
		}
		constexpr static KSCOPECYCLES literal_cycles = 0;
		template<class T2,T2 C, ITHARE_KSCOPE_SEEDTPARAM seed>
		struct literal {
			using type = KscopeLiteralFromContext<T2, C, KscopeZeroLiteralContext<T2>, seed, literal_cycles>;
		};
		template<class T2,ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPECONSTFLAGS flags2>
		constexpr static T2 random_const(T2 upper_bound=0) {
			return kscope_random_const<T2,seed2,flags2>(upper_bound);
		}

		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr T final_injection(T x) {
			return x;
		}
		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr T final_surjection(T y) {
			return y;
		}

#ifdef ITHARE_KSCOPE_DBG_ENABLE_DBGPRINT
		static void dbg_print(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "KscopeZeroContext<" << kscope_dbg_print_t<T>() << ">" << std::endl;
		}
#endif
	};
	template<class T, class T0, ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	struct KscopeRecursiveContext<T, KscopeZeroLiteralContext<T0>, seed, cycles> {
		using recursive_context_type = KscopeZeroLiteralContext<T>;
		using intermediate_context_type = KscopeZeroLiteralContext<T>;
	};
	
	//KscopeVarContext
	template<class T, ITHARE_KSCOPE_SEEDTPARAM seed,KSCOPECYCLES cycles>
	struct KscopeVarContext {
		using Type = T;
		constexpr static KSCOPECYCLES context_cycles = 0;
		constexpr static KSCOPECYCLES calc_cycles(KSCOPECYCLES inj, KSCOPECYCLES surj) {
			return inj + surj;//for variables, BOTH injection and surjection are executed in runtime
		}

		constexpr static KSCOPECYCLES literal_cycles = std::min(cycles/2,50);//TODO: justify (or define?)
		template<class T2, T2 C, ITHARE_KSCOPE_SEEDTPARAM seed2>
		struct literal {
			using LiteralContext = KscopeLiteralContext<T2, ITHARE_KSCOPE_COMBINED_PRNG(seed,seed2), literal_cycles>;
			using type = KscopeLiteralFromContext<T2, C, LiteralContext, seed2, literal_cycles>;
		};
		template<class T2,ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPECONSTFLAGS flags2>
		constexpr static T2 random_const(T2 upper_bound=0) {
			return kscope_random_const<T2,seed2,flags2>(upper_bound);
		}

		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr T final_injection(T x) {
			return x;
		}
		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr T final_surjection(	T y) {
			return y;
		}

#ifdef ITHARE_KSCOPE_DBG_ENABLE_DBGPRINT
		static void dbg_print(size_t offset = 0, const char* prefix = "") {
			std::cout << std::string(offset, ' ') << prefix << "KscopeVarContext<" << kscope_dbg_print_t<T>() << ">" << std::endl;
		}
#endif
	};
	template<class T, class T0, ITHARE_KSCOPE_SEEDTPARAM seed0, KSCOPECYCLES cycles0, ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	struct KscopeRecursiveContext<T, KscopeVarContext<T0,seed0,cycles0>, seed, cycles> {
		using recursive_context_type = KscopeVarContext<T,seed,cycles>;
		using intermediate_context_type = KscopeVarContext<T,seed,cycles>;
	};
	
	template<class Descr, class T, ITHARE_KSCOPE_SEEDTPARAM seed, KSCOPECYCLES cycles>
	class KscopeExtensibleLiteralContext {
		using Traits = KscopeTraits<T>;
		constexpr static size_t which = kscope_random_choice_from_list<ITHARE_KSCOPE_NEW_PRNG(seed, 1)>(cycles, Descr::descr);
		using WhichType = KscopeLiteralContextVersion<which, T, seed>;

	public:
		using Type = T;
		constexpr static KSCOPECYCLES context_cycles = WhichType::context_cycles;
		constexpr static KSCOPECYCLES calc_cycles(KSCOPECYCLES inj, KSCOPECYCLES surj) {
			return surj;//for literals, ONLY surjection costs apply in runtime (as injection applies in compile-time)
		}
		template<class T2,ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPECONSTFLAGS flags2>
		constexpr static T2 random_const(T2 upper_bound=0) {
			return kscope_random_const<T2,seed2,flags2>(upper_bound);
		}

		constexpr static KSCOPECYCLES literal_cycles = 0;
		template<class T2, T2 C, ITHARE_KSCOPE_SEEDTPARAM seed2>
		struct literal {
			using type = KscopeLiteralFromContext<T2, C, KscopeZeroLiteralContext<T2>, ITHARE_KSCOPE_COMBINED_PRNG(seed,seed2), literal_cycles>;
		};

		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr /* only if flags & kscope_flag_is_constexpr */ T final_injection(T x) {
			ITHARE_KSCOPE_DECLAREPRNG_INFUNC seedc = ITHARE_KSCOPE_COMBINED_PRNG(seed,seed2);
			return WhichType::template final_injection<seedc,flags>(x);
		}
		template<ITHARE_KSCOPE_SEEDTPARAM seed2,KSCOPEFLAGS flags>
		ITHARE_KSCOPE_FORCEINLINE static constexpr /* only if flags & kscope_flag_is_constexpr */ T final_surjection(T y) {
			ITHARE_KSCOPE_DECLAREPRNG_INFUNC seedc = ITHARE_KSCOPE_COMBINED_PRNG(seed,seed2);
			return WhichType::template final_surjection<seedc,flags>(y);
		}


	public:
#ifdef ITHARE_KSCOPE_DBG_ENABLE_DBGPRINT
		static void dbg_print(size_t offset = 0, const char* prefix = "") {
			size_t dbgWhich = kscope_random_choice_from_list<ITHARE_KSCOPE_NEW_PRNG(seed, 1)>(cycles, Descr::descr);
			std::cout << std::string(offset, ' ') << prefix << "KscopeExtensibleLiteralContext<ndescr=" << kscope_arraysz(Descr::descr) << "," << kscope_dbg_print_t<T>() << "," << kscope_dbg_print_seed<seed>() << "," << cycles << ">: which=" << which << " dbgWhich=" << dbgWhich << std::endl;
			WhichType::dbg_print(offset + 1);
		}
#endif
	};
	//no RecursiveContext for this one (it will be defined at the point of specializing descr)
	
}}; //namespace ithare::kscope
 
#endif //ITHARE_KSCOPE_SEED