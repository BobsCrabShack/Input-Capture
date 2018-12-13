#pragma once
#include "TypeList_Detail.h"
#include "TypeList_Detail_SetOps.h"
#include "TypeList_Detail_IndexOps.h"

namespace t_list
{
	// type_list - Represents a compile time list of types
	// All type_list operations that work on the type_list give a new type_list and do not modify the current instance
	template <typename... Ts> 
	struct type_list
	{
		// Alias for type of current type_list
		using type                                          = type_list<Ts...>;

		// Number of occurrences of T in Ts
		template<typename T>
		static constexpr std::size_t count                  = (static_cast<std::size_t>(std::is_same_v<T, Ts>) + ...);
		// True if Ts contains one or more instances of T
		template<typename T>
		static constexpr bool        contains               = detail::contains_v<T, Ts...>;
		// True if Ts contains exactly one instance of T
		template<typename T>
		static constexpr bool        contains_unique        = count<T> == 1;
		// True if all Ts are same as all Args (order matters)
		template<typename... Args>
		static constexpr bool        is_same                = std::is_same_v<type, type_list<Args...>>;
		// True if all Ts are templates of TemplateOf
		template <template<class...> class TemplateOf>
		static constexpr bool        all_template_of_type_v = detail::is_template_of_type_v<TemplateOf, Ts...>;

		// Number of types in list
		static constexpr std::size_t n_types                = sizeof... (Ts);
		// True if list is empty
		static constexpr bool        empty                  = n_types == 0;
		// True if there are no duplicate types in list
		static constexpr bool        is_unique              = detail::all_true_v<contains_unique<Ts>...>;
		// True if all Ts in list are storable types
		static constexpr bool        all_storable           = std::conjunction_v<detail::is_storable<Ts>...>;

		// Rebind Ts... to another template in the form of TTo<Ts...>
		template <template<class...> class TTo>
		using rebind                            = detail::rebind_t<type, TTo>;
		// Apply Ts... to one or more templates in the form of TTo_First<TTo_Rest<Ts>>...>
		template <template<class...> class TTo_First, template<class...> class... TTo_Rest>
		using apply                             = detail::apply_unary_t  <type, TTo_First, TTo_Rest...>;
		// Apply Ts... and Ts... from TList to one or more templates in the form of TTo_First<TTo_Rest<Ts>>...>
		template <class TList, template<class...> class TTo_First, template<class...> class... TTo_Rest>
		using apply_binary                      = detail::apply_binary_t <type, TList, TTo_First, TTo_Rest...>;

		// Remove all elements from list
		using clear                             = type_list<>;
		// Reverse all Ts in list
		using reverse                           = detail::reverse_t<type>;
		// Remove duplicates from list, list ends in a reversed state
		using unique                            = std::conditional_t<is_unique, type, detail::type_list_unique<Ts...>>;

		// Extract type at idx
		template <std::size_t I>
		using extract                           = detail::extract_t<I, Ts...>;
		// Erase type at idx
		template <std::size_t I>
		using erase                             = detail::erase_t  <I, Ts...>;

		// Acess first type in list
		using front                             = detail::front_t<type>;
		// Add Args to front of list
		template <typename... Args>
		using append_front                      = type_list<Args..., Ts...>;
		// Append T to front of list if T does not exist in Ts...
		template <typename T>
		using append_front_unique               = std::conditional_t<contains<T>, type, type_list<T, Ts...>>;
		// Append Args to front of list if Predicate<Args>::value == true
		template <template <typename> class Predicate, typename... Args>
		using append_conditional_front          = detail::append_conditional_front_t<Predicate, type, Args...>;
		// Remove first element in list
		using pop_front                         = detail::pop_front_t<type>;

		// Access last type in list
		using back                              = detail::back_t     <type>;
		// Append Args to end of list
		template <typename... Args>
		using append                            = type_list<Ts..., Args...>;
		// Append T to end of list if T does not exist in Ts...
		template <typename T>
		using append_unique                     = std::conditional_t<contains<T>, type, type_list<Ts..., T>>;
		// Append Args to end of list if Predicate<Args>::value == true
		template <template <typename> class Predicate, typename... Args>
		using append_conditional                = detail::append_conditional_t<Predicate, type, Args...>;
		// Remove last element in list
		using pop_back                          = detail::pop_back_t<type>;

		// Append all TLists to current list of types
		template <typename... TLists>
		using append_lists                      = detail::type_list_cat_t <type, TLists...>;
		// Append all types in TLists to current list of types if Predicate<T>::value == true, list ends in a reversed state
		template <template <typename> class Predicate, class... TLists>
		using append_lists_conditional          = detail::type_list_cat_conditional_t<Predicate, type, TLists...>;

		// Remove all elements where Predicate<Ts>::value is false
		template <template <typename> class Predicate>
		using filter                            = detail::filter_t       <Predicate, Ts...>;
		// New typeList containing all Ts where Predicate<T, TList>::value is true
		template <template <typename, typename> class Predicate, typename TList>
		using filter_binary                     = detail::filter_binary_t<Predicate, type, TList>;

		// All set operations result in a unique type_list
		// Computes union between type_list<Ts...> and TList
		template<typename TList>
		using setop_union                       = detail::union_t               <type, TList>;
		// Computes intersection between type_list<Ts...> and TList
		template<typename TList>
		using setop_intersection                = detail::intersection_t        <type, TList>;
		// Computes difference between type_list<Ts...> and TList
		template<typename TList>
		using setop_difference                  = detail::difference_t          <type, TList>;
		// Computes symmetric difference between type_list<Ts...> and TList
		template<typename TList>
		using setop_symmetric_difference        = detail::symmetric_difference_t<type, TList>;
		// Computes cartesian product between type_list<Ts...> and TList
		template <typename TList>
		using setop_cartesian_product           = detail::cartesian_product_t   <type, TList>;
		// True if type_list<Ts...> is a subset of TList
		template<typename TList>
		static constexpr bool is_subset         = (n_types == setop_intersection<TList>::n_types);
		// True if type_list<Ts...> is a subset of TList
		template<typename TList>
		static constexpr bool is_equivalent_set = detail::is_equivalent_v<type, TList>;

		// True if Predicate<Ts>:value is true for all Ts in list
		template <template <typename> class Predicate>
		static constexpr bool        all_match_predicate()
		{
			return std::conjunction_v<Predicate<Ts>...>;
		}
		// True if Predicate<Ts, Args>::value... for all Ts and Args is true
		template <template <typename, typename> class Predicate, typename... Args>
		static constexpr bool        all_match_predicate()
		{
			if constexpr (sizeof... (Args) == n_types)
				return std::conjunction_v<Predicate<Ts, Args>...>;

			return false;
		}
		// Returns true if all Ts are convertible to Args
		template <typename... Args>
		static constexpr bool        is_convertible()
		{
			if constexpr (sizeof... (Args) == n_types)
				return std::conjunction_v<std::is_convertible<std::decay_t<Ts>, std::decay_t<Args>>...>;

			return false;
		}
		// Returns true if all Ts in TList convertible to Ts
		template <typename TList>
		static constexpr bool        is_convertible_list ()
		{
			return detail::is_convertible_list_v<type, TList>;
		}

		// Returns total size required to store all the types in the type_list
		static constexpr std::size_t total_size()
		{
			static_assert(all_storable, "Error: cannot determine total size of type_list if all types are not storable");
			return (sizeof(Ts) + ...);
		}
		// Returns sizeof smallest type in type_list
		static constexpr std::size_t type_min_size()
		{
			static_assert(all_storable, "Error: cannot determine sizeof min_type in type_list if all types are not storable");
			return sizeof(detail::smallest_type_t<Ts...>);
		}
		// Returns sizeof largest type in type_list
		static constexpr std::size_t type_max_size()
		{
			static_assert(all_storable, "Error: cannot determine sizeof max_type in type_list if all types are not storable");
			return sizeof(detail::largest_type_t<Ts...>);
		}
	};
}