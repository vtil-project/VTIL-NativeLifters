namespace vtil
{
	template<>
	struct register_cast<x86_reg>
	{
		register_desc operator()(x86_reg value)
		{
			auto [base, offset, size] = amd64::resolve_mapping(value);
			return register_desc(register_physical, base, size * 8, offset * 8);
		}
	};

	namespace lifter_x86
	{
		static inline const vtil::register_desc UNDEFINED = {vtil::register_virtual | vtil::register_volatile | vtil::register_local, 0, 64};
		static inline const vtil::register_desc FLAG_CF = {vtil::register_physical | vtil::register_flags, 0, 1, 0};
		static inline const vtil::register_desc FLAG_PF = {vtil::register_physical | vtil::register_flags, 0, 1, 2};
		static inline const vtil::register_desc FLAG_AF = {vtil::register_physical | vtil::register_flags, 0, 1, 4};
		static inline const vtil::register_desc FLAG_ZF = {vtil::register_physical | vtil::register_flags, 0, 1, 6};
		static inline const vtil::register_desc FLAG_SF = {vtil::register_physical | vtil::register_flags, 0, 1, 7};
		static inline const vtil::register_desc FLAG_DF = {vtil::register_physical | vtil::register_flags, 0, 1, 10};
		static inline const vtil::register_desc FLAG_OF = {vtil::register_physical | vtil::register_flags, 0, 1, 11};
	}
};