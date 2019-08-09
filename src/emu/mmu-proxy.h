//
//  mmu-proxy.h
//

#ifndef rv_mmu_proxy_h
#define rv_mmu_proxy_h

namespace riscv {

	template <typename UX>
	struct proxy_memory
	{
		std::vector<std::pair<void*,size_t>> segments;
		addr_t heap_begin;
		addr_t heap_end;
		addr_t brk;
		bool log;

		void print_memory_map() {}

		proxy_memory() : segments(), heap_begin(0), heap_end(0), brk(0), log(false) {}
	};

	template <typename UX, typename MEMORY = proxy_memory<UX>>
	struct mmu_proxy
	{
		/*
		 * Define top of emulator address space, beginning of emulator text.
		 *
		 * MACOS_LDFLAGS = -Wl,-pagezero_size,0x1000 -Wl,-no_pie -image_base 0x7ffe00000000
		 * LINUX_LDFLAGS = -pie -fPIE -Wl,-Ttext-segment=0x7ffe00000000
		 */

		static const bool enfore_memory_top = false;

		typedef std::shared_ptr<MEMORY> memory_type;

		enum : addr_t {
#if ! defined __MINGW32__
			memory_top = (sizeof(UX) == 4 ? 0x80000000 : 0x7f0000000000),
#else
			// rv64 : mingw does not support such a huge malloc
			memory_top = (sizeof(UX) == 4 ? 0x80000000 : 0x80000000),
#endif
			stack_size = 0x100000 // 1 MiB
		};

		memory_type mem;

#if ! defined __MINGW32__
		/* MMU constructor */

		mmu_proxy() : mem(std::make_shared<MEMORY>()) {}
		mmu_proxy(memory_type mem) : mem(mem) {}
#else
		addr_t m;

		/* MMU constructor */

		mmu_proxy()
			: mem(std::make_shared<MEMORY>()),
			  m((addr_t) calloc(1, memory_top))
		{}
		mmu_proxy(memory_type mem)
			: mem(mem),
			  m((addr_t) calloc(1, memory_top))
		{}
		~mmu_proxy()
		{
			free((void*)m);
		}
#endif

		template <typename P> inst_t inst_fetch(P &proc, UX pc, typename P::ux &pc_offset)
		{
			/* record pc histogram using machine physical address */
			if (proc.log & proc_log_hist_pc) {
				size_t iters = proc.histogram_add_pc(pc);
				if (proc.log & proc_log_jit_trap) {
					switch (iters) {
						case P::hostspot_trace_skip:
							break;
						default:
							if (iters >= proc.trace_iters) {
								proc.raise(P::internal_cause_hotspot, pc);
							}
							break;
					}
				}
			}
#if ! defined __MINGW32__
			return riscv::inst_fetch(pc, pc_offset);
#else
			return riscv::inst_fetch(m + pc, pc_offset);
#endif
		}

		/* Note: in this simple proxy MMU model, stores beyond memory top wrap */

		template <typename P, typename T>
		void amo(P &proc, const amo_op a_op, UX va, T &val1, T val2)
		{
#if ! defined __MINGW32__
			val1 = UX(*(T*)addr_t(va & (memory_top - 1)));
#else
			val1 = UX(*(T*)addr_t(m + (va & (memory_top - 1))));
#endif
			val2 = amo_fn<UX>(a_op, val1, val2);
#if ! defined __MINGW32__
			*((T*)addr_t(va & (memory_top - 1))) = val2;
#else
			*((T*)addr_t(m + (va & (memory_top - 1)))) = val2;
#endif
		}

		template <typename P, typename T> void load(P &proc, UX va, T &val)
		{
			if (enfore_memory_top) {
#if ! defined __MINGW32__
				val = UX(*(T*)addr_t(va & (memory_top - 1)));
#else
				val = UX(*(T*)addr_t(m + (va & (memory_top - 1))));
#endif
			} else {
#if ! defined __MINGW32__
				val = UX(*(T*)addr_t(va));
#else
				val = UX(*(T*)addr_t(m + va));
#endif
			}
		}

		template <typename P, typename T> void store(P &proc, UX va, T val)
		{
			if (enfore_memory_top) {
#if ! defined __MINGW32__
				*((T*)addr_t(va & (memory_top - 1))) = val;
#else
				*((T*)addr_t(m + (va & (memory_top - 1)))) = val;
#endif
			} else {
#if ! defined __MINGW32__
				*((T*)addr_t(va)) = val;
#else
				*((T*)addr_t(m + va)) = val;
#endif
			}
		}

#if defined __MINGW32__
		// ~memcpy (from host to guest)
		template <typename P, typename T> void store(P &proc, UX va, T *val, size_t n)
		{
			for (size_t i=0; i<n; va+=sizeof(T), ++i ) {
				store<P,T>(proc, va, val[i]);
			}
		}

		// ~memset (to guest)
		template <typename P, typename T> void store(P &proc, UX va, T val, size_t n)
		{
			for (size_t i=0; i<n; va+=sizeof(T), ++i ) {
				store<P,T>(proc, va, val);
			}
		}

		template <typename P> addr_t base() const
		{
			return m;
		}

		template <typename P> addr_t base(UX va) const
		{
			return base<P>() + va;
		}
#endif
	};

	using mmu_proxy_rv32 = mmu_proxy<u32>;
	using mmu_proxy_rv64 = mmu_proxy<u64>;

}

#endif
