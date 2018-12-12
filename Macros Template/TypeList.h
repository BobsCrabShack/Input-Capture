#pragma once
#include "TypeList_Detail.h"

namespace t_list
{
	// type_list - Represents a compile time list of types
	// All type_list operations that work on the type_list give a new type_list and do not modify the current instance
	template <typename... Ts> 
	struct type_list
	{
		// Number of occurrences of T in Ts
		template<typename T>
		static constexpr std::size_t count                  = (static_cast<std::size_t>(std::is_same_v<T, Ts>) + ...);
		// True if Ts contains one or more instances of T
		template<typename T>
		static constexpr bool        contains               = t_list::detail::contains_v<T, Ts...>;
		// True if Ts contains exactly one instance of T
		template<typename T>
		static constexpr bool        contains_unique        = count<T> == 1;
		// True if all Ts are same as all Args
		template<typename... Args>
		static constexpr bool        same                   = std::is_same_v<type_list<Args...>, type_list<Ts...>>;
		// True if all Ts are templates of TemplateOf
		template <template<class...> class TemplateOf>
		static constexpr bool        all_template_of_type_v = t_list::detail::is_template_of_type_v<TemplateOf, Ts...>;

		// Number of types in list
		static constexpr std::size_t n_types                = sizeof... (Ts);
		// True if list is empty
		static constexpr bool        empty                  = n_types == 0;
		// True if there are no duplicate types in list
		static constexpr bool        is_unique              = t_list::detail::all_true_v<contains_unique<Ts>...>;
		// True if all Ts in list are storable types
		static constexpr bool        all_storable           = std::conjunction_v<t_list::detail::is_storable<Ts>...>;

		// Rebind Ts... to another template in the form of TTo<Ts...>
		template <template<class...> class TTo>
		using rebind                           = t_list::detail::rebind_t<type_list<Ts...>, TTo>;
		// Apply Ts... to one or more templates in the form of TTo_First<TTo_Rest<Ts>>...>
		template <template<class...> class TTo_First, template<class...> class... TTo_Rest>
		using apply                            = t_list::detail::apply_t<type_list<Ts...>, TTo_First, TTo_Rest...>;

		// Reverse all Ts in list
		using reverse                          = t_list::detail::reverse_t<type_list<Ts...>>;
		// Remove duplicates from list
		using unique                           = std::conditional_t<is_unique, type_list<Ts...>, t_list::detail::type_list_unique<Ts...>>;

		// Extract type at idx
		template <std::size_t idx>
		using extract                          = t_list::detail::extract_t<idx, Ts...>;
		// Extract type at idx
		template <std::size_t idx>
		using erase                            = t_list::detail::erase_t  <idx, Ts...>;

		// Acess first type in list
		using front                            = t_list::detail::front_t    <type_list<Ts...>>;
		// Add Args to front of list
		template <typename... Args>
		using append_front                     = type_list<Args..., Ts...>;
		// Append Args to front of list if Predicate<Args>::value == true
		template <template <typename> class Predicate, typename... Args>
		using append_conditional_front         = t_list::detail::append_conditional_front_t<Predicate, type_list<Ts...>, Args...>;
		// Remove first element in list
		using pop_front                        = t_list::detail::pop_front_t<type_list<Ts...>>;

		// Access last type in list
		using back                             = t_list::detail::back_t     <type_list<Ts...>>;
		// Append Args to end of list
		template <typename... Args>
		using append                           = type_list<Ts..., Args...>;
		// Append Args to end of list if Predicate<Args>::value == true
		template <template <typename> class Predicate, typename... Args>
		using append_conditional               = t_list::detail::append_conditional_t<Predicate, type_list<Ts...>, Args...>;

		// Append all TLists to current list of types
		template <typename... TLists>
		using append_lists                     = t_list::detail::type_list_cat_t <type_list<Ts...>, TLists...>;
		// Append all types in TLists to current list of types if Predicate<T>::value == true
		template <template <typename> class Predicate, class... TLists>
		using append_lists_conditional         = t_list::detail::type_list_cat_conditional_t<Predicate, type_list<Ts...>, TLists...>;
		// Remove all elements from list
		using clear                            = type_list<>;

		// Remove all elements where Predicate<Ts>::value is false
		template <template <typename> class Predicate>
		using filter                          = t_list::detail::type_list_filter_t<Predicate, Ts...>;

		// Computes union between type_list<Ts...> and TList
		template<typename TList>
		using setop_union                     = append_lists                          <type_list<Ts...>, TList>;		
		// Computes intersection between type_list<Ts...> and TList
		template<typename TList>
		using setop_intersection              = t_list::detail::intersection_t        <type_list<Ts...>, TList>;
		// Computes difference between type_list<Ts...> and TList
		template<typename TList>
		using setop_difference                = t_list::detail::difference_t          <type_list<Ts...>, TList>;
		// Computes symmetric difference between type_list<Ts...> and TList
		template<typename TList>
		using setop_symmetric_difference      = t_list::detail::symmetric_difference_t<type_list<Ts...>, TList>;
		// Computes cartesian product between type_list<Ts...> and TList
		template <typename TList>
		using setop_cartesian_product         = t_list::detail::cartesian_product_t   <type_list<Ts...>, TList>;
		// True if type_list<Ts...> is a subset of TList
		template<typename TList>
		static constexpr bool setop_is_subset = std::is_same_v<type_list<Ts...>, setop_intersection<TList>>;

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
		static constexpr bool        convertible()
		{
			if constexpr (sizeof... (Args) == n_types)
				return std::conjunction_v<std::is_convertible<std::decay_t<Ts>, std::decay_t<Args>>...>;

			return false;
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
			return sizeof(t_list::detail::smallest_type_t<Ts...>);
		}
		// Returns sizeof largest type in type_list
		static constexpr std::size_t type_max_size()
		{
			static_assert(all_storable, "Error: cannot determine sizeof max_type in type_list if all types are not storable");
			return sizeof(t_list::detail::largest_type_t<Ts...>);
		}
	};
}