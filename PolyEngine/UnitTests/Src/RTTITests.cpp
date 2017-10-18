#include <catch.hpp>

enum ePropertyFlag {
	NONE           = 0b0000,
	DONT_SERIALIZE = 0b0010,
	EIGHT          = 0b1000
};

// ################################################################## Fowler–Noll–Vo hash function (FNV-1a variant)

constexpr uint64_t fnv1a(size_t n, const char* data, uint64_t hash = 0xcbf29ce484222325) {
	return n == 0 ? hash : fnv1a(n - 1, data + 1, (hash ^ *data) * 0x100000001b3);
}

// ################################################################## COMPILE-TIME STRING

namespace ct {
	template<const char... Chars>
	struct string {
		static constexpr const char chars[sizeof...(Chars) + 1] = {Chars..., '\0'};
		static constexpr const size_t len = sizeof...(Chars);
		static constexpr uint64_t hash = fnv1a(len, chars);
	};
	template<const char... Chars> constexpr const char string<Chars...>::chars[sizeof...(Chars) + 1];

	template<size_t N> constexpr size_t strlen(const char (&)[N]) { return N - 1; }

	template<typename StrData>
	struct build_string {
		template<typename> struct builder;

		template<std::size_t... Indices>
		struct builder<std::index_sequence<Indices...>> {
			using result = ct::string<StrData::chars[Indices]...>;
		};

		using result = typename builder<std::make_index_sequence<StrData::len>>::result;
	};

	template<typename StrData>
	struct build_string_non_static {
		template<typename> struct builder;

		template<std::size_t... Indices>
		struct builder<std::index_sequence<Indices...>> {
			using result = ct::string<StrData{}.chars[Indices]...>;
		};

		using result = typename builder<std::make_index_sequence<StrData{}.len>>::result;
	};
}

// ################################################################## IMPLEMENTATION (HELPERS)

template<typename T, typename B> T prop_type(T B::*); // property pointer type -> property type
template<typename T, typename B> B base_type(T B::*); // property pointer type -> property holder class type

template<typename PropInfo, typename Func>
bool invoke_if(std::false_type, PropInfo, Func&&) { return false; };

template<typename PropInfo, typename Func>
bool invoke_if(std::true_type, PropInfo p, Func&& f) { f(p); return true; };

struct RTTI;
template<typename T> RTTI* parent_ctti() { return (RTTI*)T::CTTI::Instance(); }
template<> RTTI* parent_ctti<void>() { return nullptr; }

// ################################################################## IMPLEMENTATION (TYPES)

template<typename...> struct TypeList {};

template<typename PtrType, PtrType Ptr, size_t NameLen, const char Name[NameLen], ePropertyFlag Flags>
struct Property {
	using Type = decltype(prop_type(Ptr));
	static constexpr PtrType ptr = Ptr;
	static constexpr const char* name = Name;
	static constexpr size_t name_len = NameLen;
	static constexpr size_t hash = fnv1a(name_len, name);
	static constexpr auto flags = Flags;
	static Type get(decltype(base_type(Ptr))* base) { return base->*Ptr; }
};

struct RTTI {
	constexpr RTTI(RTTI* baseTypeInfo) : baseTypeInfo(baseTypeInfo) {}
	const RTTI* const baseTypeInfo;
};

// ################################################################## IMPLEMENTATION (PROPERTY ACCESS)

template<typename Func> void for_each_prop_pimple(TypeList<>, Func&&) {};

template<typename TI0, typename... TIs, typename Func>
void for_each_prop_pimple(TypeList<TI0, TIs...>, Func&& f) {
	f(TI0{});
	for_each_prop_pimple(TypeList<TIs...>{}, std::forward<Func>(f));
};

template<typename Reflected, typename Func>
void for_each_prop(Func&& f) { for_each_prop_pimple(typename Reflected::CTTI::Properties{}, std::forward<Func>(f)); };

template<typename Func> void for_each_prop_recursive_pimple(void*, Func&&) {};

template<typename Reflected, typename Func>
void for_each_prop_recursive_pimple(Reflected*, Func&& f) {
	for_each_prop_recursive_pimple((typename Reflected::CTTI::BaseClass*){nullptr}, std::forward<Func>(f));
	for_each_prop<Reflected>(std::forward<Func>(f));
};

template<typename Reflected, typename Func>
void for_each_prop_recursive(Func&&f) { for_each_prop_recursive_pimple((Reflected*){nullptr}, std::forward<Func>(f)); };

template<typename Reflected, typename T, typename Func>
void for_prop(const char* name, Func&& f) {
	size_t hash = fnv1a(std::strlen(name), name);
	bool found = false;
	for_each_prop<Reflected>([&](auto prop_info) {
		using Property = decltype(prop_info);
		//ASSERTE(std::is_same<T, typename Property::Type>::value, "Wrong type `T` for requested property");
		if (hash == Property::hash && std::strcmp(name, Property::name) == 0) {
			found = found || invoke_if(std::is_same<T, typename Property::Type>{}, prop_info, f);
			return;
		}
	});
	assert(found);
};

// ################################################################## IMPLEMENTATION (DATA DEFINITIONS)

#define POLYENGINE_RTTI(DERIVED_CLASS, BASE_CLASS, ...)                                                                                                                                                \
	struct CTTI : public RTTI                                                                                                                                                                          \
	{                                                                                                                                                                                                  \
		using RTTI::RTTI;                                                                                                                                                                              \
		static constexpr const char name[] = #DERIVED_CLASS;                                                                                                                                           \
		static constexpr const size_t hash = fnv1a(ct::strlen(name), name);                                                                                                                            \
		using BaseClass                    = BASE_CLASS;                                                                                                                                               \
		using ReflectedClass               = DERIVED_CLASS;                                                                                                                                            \
		using Properties                   = TypeList<__VA_ARGS__>;                                                                                                                                    \
		static const CTTI* Instance()                                                                                                                                                                  \
		{                                                                                                                                                                                              \
			static const CTTI type_info{parent_ctti<BaseClass>()};                                                                                                                                     \
			return &type_info;                                                                                                                                                                         \
		}                                                                                                                                                                                              \
		private: void assert_is_base()                                                                                                                                                                 \
		{                                                                                                                                                                                              \
			static_assert(                                                                                                                                                                             \
			    (std::is_base_of<BaseClass, ReflectedClass>::value && !std::is_same<BaseClass, ReflectedClass>::value) || std::is_same<BaseClass, void>::value,                                        \
			    "Wrong type(-s) passed to reflection macro. (Hint: `ReflectedClass` is not derived from `BaseClass`)"                                                                                  \
			);                                                                                                                                                                                         \
		}                                                                                                                                                                                              \
	};                                                                                                                                                                                                 \
	virtual const RTTI* PolymorphicTypeInfo() { return static_cast<const RTTI*>(CTTI::Instance()); };

#if defined(__GNUC__) && !defined(__clang__)

template<typename C, C... Chars> constexpr ct::string<Chars...> operator""_ctstr() { return {}; } //non-standard GNU extension (GCC, Clang with warning)

#define PROPERTY(VAR, FLAGS) Property<decltype(&ReflectedClass:: VAR), &ReflectedClass:: VAR, ct::strlen(#VAR), decltype(#VAR ## _ctstr)::chars, FLAGS>

#else

// requires C++17 for constexpr lambdas
// GCC does not support lambda expressions in template arguments due to a bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80488
#define PROPERTY(VAR, FLAGS)                                                                                                                                                                           \
Property<                                                                                                                                                                                              \
	decltype(&ReflectedClass:: VAR), &ReflectedClass:: VAR, ct::strlen(#VAR),                                                                                                                          \
	[]() constexpr {                                                                                                                                                                                   \
		struct chars_provider { const char* chars = #VAR; const size_t len = ct::strlen(#VAR); };                                                                                                      \
		return ct::build_string_non_static<chars_provider>::result{};                                                                                                                                  \
	}().chars,                                                                                                                                                                                         \
	FLAGS                                                                                                                                                                                              \
>

#endif //it is possible to make it portable using macros in a several ways; all of them incur some kind of limit, be it a maximum number of properties or a maximum length of a property name

#define DERIVED_FROM(CLASS) CLASS
#define BASELESS DERIVED_FROM(void)

// ################################################################## USAGE

// EXPANDED
struct TestClass00 {
	int num0;
	const char* str0;
	void* ptr0;


	struct CTTI : public RTTI {
		using RTTI::RTTI;
		static constexpr const char name[] = "TestClass00";
		static constexpr const size_t hash = fnv1a(ct::strlen(name), name);
		using BaseClass = void;
		using ReflectedClass = TestClass00;
		using Properties = TypeList<
			PROPERTY(num0, ePropertyFlag::NONE), //implementation is compiler-dependent for now (see the macro for more details)
			PROPERTY(str0, ePropertyFlag::NONE),
			PROPERTY(ptr0, ePropertyFlag::NONE)
		>;
		static const CTTI* Instance() { static const CTTI type_info{parent_ctti<BaseClass>()}; return &type_info; }
	};
	virtual const RTTI* PolymorphicTypeInfo() { return static_cast<const RTTI*>(CTTI::Instance()); };


	TestClass00() : num0(-8), str0("8"), ptr0(this) {}
	virtual ~TestClass00() = default;
};

struct TestClass01 : public TestClass00 {
	POLYENGINE_RTTI(TestClass01, DERIVED_FROM(TestClass00));

	virtual ~TestClass01() = default;
};

struct TestClass04 {
	int num4;
	const char* str4;
	POLYENGINE_RTTI(
		TestClass04, BASELESS,
		PROPERTY(num4, ePropertyFlag::NONE),
		PROPERTY(str4, ePropertyFlag::DONT_SERIALIZE)
	);
};

struct TestClass05 : public TestClass04 {
	float num4;
	POLYENGINE_RTTI(
		TestClass05, DERIVED_FROM(TestClass04),
		PROPERTY(num4, ePropertyFlag::EIGHT)
	);

	virtual ~TestClass05() = default;
};

// ################################################################## RUNTIME NAMES EXPERIMENT

template<typename... Ps> struct Properties : public TypeList<Ps...> {
	const char* names[sizeof...(Ps)];
	constexpr Properties(Ps... ps) : names{ps.name...} {}
};

template<typename... Ps> Properties<Ps...> constexpr make_properties(Ps... ps) { return {ps...}; }

template<typename PtrType, PtrType Ptr, size_t NameLen, uint64_t Hash, ePropertyFlag Flags>
struct PropertyRN {
	using Type = decltype(prop_type(Ptr));
	static constexpr PtrType ptr = Ptr;
	static constexpr size_t name_len = NameLen;
	static constexpr uint64_t hash = Hash;
	static constexpr auto flags = Flags;
	static Type get(decltype(base_type(Ptr))* base) { return base->*Ptr; }
	const char* name;
	constexpr PropertyRN(const char* name) : name(name) {}
};

struct TestRuntimeNames {
	int num0;
	const char* str0;
	void* ptr0;
	using ReflectedClass = TestRuntimeNames;
	static constexpr auto properties() -> decltype(
		std::make_tuple(
			PropertyRN<decltype(&ReflectedClass::num0), &ReflectedClass::num0, ct::strlen("num"), fnv1a(ct::strlen("num"), "num"), ePropertyFlag::NONE>{"num"},
			PropertyRN<decltype(&ReflectedClass::str0), &ReflectedClass::str0, ct::strlen("str"), fnv1a(ct::strlen("str"), "str"), ePropertyFlag::NONE>{"str"},
			PropertyRN<decltype(&ReflectedClass::ptr0), &ReflectedClass::ptr0, ct::strlen("ptr"), fnv1a(ct::strlen("ptr"), "ptr"), ePropertyFlag::NONE>{"ptr"}
		)
	) {
		return std::make_tuple(
			PropertyRN<decltype(&ReflectedClass::num0), &ReflectedClass::num0, ct::strlen("num"), fnv1a(ct::strlen("num"), "num"), ePropertyFlag::NONE>{"num"},
			PropertyRN<decltype(&ReflectedClass::str0), &ReflectedClass::str0, ct::strlen("str"), fnv1a(ct::strlen("str"), "str"), ePropertyFlag::NONE>{"str"},
			PropertyRN<decltype(&ReflectedClass::ptr0), &ReflectedClass::ptr0, ct::strlen("ptr"), fnv1a(ct::strlen("ptr"), "ptr"), ePropertyFlag::NONE>{"ptr"}
		);
	}
	using Properties = decltype(properties());
};

template<size_t N, typename Reflected>
const char* property_name() { return std::get<typename std::tuple_element<N, typename Reflected::Properties>::type>(Reflected::properties()).name; };

// ################################################################## IMPLEMENTATION (POLYMORPHISM)

template<typename To, typename From> To* polymorphic_cast(From* ptr) {
	static_assert(std::is_same<To,   typename To::  CTTI::ReflectedClass>::value, "Wrong type passed to reflection macro! (Hint: type `To`)");
	static_assert(std::is_same<From, typename From::CTTI::ReflectedClass>::value, "Wrong type passed to reflection macro! (Hint: type `From`)");
	auto* typeInfo = ptr->PolymorphicTypeInfo();
	do {
		if (typeInfo == To::CTTI::Instance()) {
			return static_cast<To*>(ptr);
		}
	} while ((typeInfo = typeInfo->baseTypeInfo));
	return nullptr;
};

template<typename CheckAgainst, typename Checked> bool instance_of(Checked* ptr) {
	static_assert(std::is_same<CheckAgainst, typename CheckAgainst::CTTI::ReflectedClass>::value, "Wrong type passed to reflection macro! (Hint: type `CheckAgainst`)");
	static_assert(std::is_same<Checked,      typename Checked::     CTTI::ReflectedClass>::value, "Wrong type passed to reflection macro! (Hint: type `Checked`)");
	return (char*)CheckAgainst::CTTI::Instance() == (char*)ptr->PolymorphicTypeInfo();
};

// ################################################################## TESTS

TEST_CASE("RTTI basics", "[RTTI]") {
	TestClass01* derived = new TestClass01();
	TestClass00* base = new TestClass00();
	TestClass00* poly = new TestClass01();

	CHECK(instance_of<TestClass01>(derived));
	CHECK(instance_of<TestClass00>(base));
	CHECK(instance_of<TestClass01>(poly));

	CHECK(polymorphic_cast<TestClass00>(derived) == static_cast<TestClass00*>(derived));
	CHECK(polymorphic_cast<TestClass01>(derived) == derived);

	CHECK(polymorphic_cast<TestClass00>(base) == base);
	CHECK(polymorphic_cast<TestClass01>(base) == nullptr);

	CHECK(polymorphic_cast<TestClass00>(poly) == poly);
	CHECK(polymorphic_cast<TestClass01>(poly) == static_cast<TestClass01*>(poly));

	printf("\nAll:\n");

	for_each_prop<TestClass00>([base](auto prop_info) {
		using Property = decltype(prop_info);
		printf("prop: %s, hash: %zu\n", Property::name, Property::hash);
		//printf("val: %s\n", base->*Property::ptr);
		//printf("val: %s\n", Property::get(base));
		//printf("val: %d\n", base->*Property::ptr);
		(void) (base->*Property::ptr);
	});

	printf("\nVal:\n");

	for_prop<TestClass00, int>("num0", [base](auto prop_info) {
		using Property = decltype(prop_info);
		printf("prop: %s, val: %d\n", Property::name, base->*Property::ptr);
	});

	for_prop<TestClass00, const char*>("str0", [base](auto prop_info) {
		using Property = decltype(prop_info);
		printf("prop: %s, val: %s\n", Property::name, base->*Property::ptr);
	});

	for_prop<TestClass00, void*>("ptr0", [base](auto prop_info) {
		using Property = decltype(prop_info);
		printf("prop: %s, val: %p\n", Property::name, base->*Property::ptr);
	});

	printf("\nRec:\n");

	TestClass05* five = new TestClass05();
	for_each_prop_recursive<TestClass05>([](auto prop_info) {
		using Property = decltype(prop_info);
		printf("prop: %s, flag_int: %d\n", Property::name, (int) Property::flags);
	});

	printf("rn: %s\n", property_name<0, TestRuntimeNames>());

	delete derived;
	delete base;
	delete poly;
	delete five;
}




// ################################################################## CONSTEXPR NO-MACRO TYPE NAME EXPERIMENT

#ifdef __clang__ //this particular implementation is Clang (and newer MSVC) only :(

struct name_view {
	const char* chars;
	size_t len;
};

template <typename T>
struct type_info
{
	struct cached
	{
		static constexpr name_view get_name()
		{
			constexpr auto name = __PRETTY_FUNCTION__;
			constexpr auto len = ct::strlen(__PRETTY_FUNCTION__);
			int64_t eq_idx = len;
			for (int64_t i = len - 1; i >= 0; i--)
			{
				if (name[i] == '=')
				{
					eq_idx = i;
					break;
				}
			}

			return name_view{name + eq_idx + 2, len - eq_idx - 3};
			//return {name, len};
		}

		static constexpr name_view name = get_name();
		struct chars_provider {
			static constexpr const char* chars = cached::name.chars;
			static constexpr size_t len = cached::name.len;
		};
	};
	using name = typename ct::build_string<typename cached::chars_provider>::result;
};

template<typename T> constexpr const name_view type_info<T>::cached::name;

#endif
