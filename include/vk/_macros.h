#ifndef MACROS_H_
#define MACROS_H_

#include "_pch.h"

//#define NDEBUG

	#ifndef NDEBUG

		#if !defined(__PRETTY_FUNCTION__) && !defined(__GNUC__) && !defined(__GNUG__)
			#define __PRETTY_FUNCTION__ __FUNCSIG__
		#endif

		#ifdef WIN32

			#define RED_COLOR			FOREGROUND_RED
			#define GREEN_COLOR		FOREGROUND_GREEN
			#define BLUE_COLOR		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
			#define YELLOW_COLOR	(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)

		#else

			#define RESET_COLOR   "\x1B[0m"
			#define RED_COLOR     "\x1B[31m"
			#define GREEN_COLOR		"\x1B[32m"
			#define BLUE_COLOR		"\x1B[36m"
			#define YELLOW_COLOR  "\x1B[33m"

		#endif

		#define DEBUG_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, BLUE_COLOR,		"\n\n",	##__VA_ARGS__);
		#define TRACE_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, BLUE_COLOR,		"\n",		##__VA_ARGS__);
		#define INFO_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, GREEN_COLOR,		"\n\n",	##__VA_ARGS__);
		#define WARN_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, YELLOW_COLOR,	"\n\n",	##__VA_ARGS__);
		#define ERROR_LOG(message, ...)						vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__);
		#define CRITICAL_ERROR_LOG(message, ...)	vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__);
		#define FATAL_ERROR_LOG(message, ...)			vk::debug::COLOR_PRINTF(message, RED_COLOR, 		"\n\n",	##__VA_ARGS__); std::abort();

		#define ASSERT(x, message)								if(!(x)) vk::debug::PRINT_ERR(message)
		#define ASSERT_VK(x, message)							ASSERT((x) == VK_SUCCESS, (message));
		#define ASSERT_FATAL(x, message)					ASSERT(x, message); if(!(x)) { vk::debug::PRINT_STACKTRACE(); std::abort(); }

		#define ASSERT_ENUMS(...)									vk::debug::ENUMS_ASSERT<__VA_ARGS__>();
		#define ASSERT_STATIC_VAR(variable)				vk::debug::STATIC_VAR_MACRO_ASSERT(variable); static_assert(variable, "static variable " #variable " not valid or defined correctly!");

		#ifndef ENABLE_VK_VARS_ASSERT
			#error "Add compile def ENABLE_VK_VARS_ASSERT in cmake and use macro ASSERT_VK_STATIC_VARS_FUNC() in the main.cpp to assert initialization of vulkan app static variables."
		#else
			#define ASSERT_VK_STATIC_VARS()      		ASSERT_STATIC_VAR(vk::Attachment::s_attCount)				\
																							ASSERT_STATIC_VAR(vk::RenderPass::s_subpassCount)   \
																							ASSERT_STATIC_VAR(vk::RenderPass::s_spDepCount)     \
																							ASSERT_STATIC_VAR(vk::Buffer		::s_bufferCount)    \
																							ASSERT_STATIC_VAR(vk::Buffer		::s_ubcCount)				\
																							ASSERT_STATIC_VAR(vk::Descriptor::s_setLayoutCount)	\
																							ASSERT_STATIC_VAR(vk::Model			::s_modelCount)

			#define ASSERT_VK_STATIC_VARS_FUNC()		static void VK_STATIC_VARS_ASSERT() { ASSERT_VK_STATIC_VARS() }
		#endif

		#define ERROR_VK(message)									ERROR_LOG(message);
		#define WARN_VK(message)									WARN_LOG(message);
		#define TRACE_VK(message)									TRACE_LOG(message);

	#else

		#define DEBUG_LOG(message, ...);
		#define TRACE_LOG(message, ...);
		#define INFO_LOG(message, ...);
		#define WARN_LOG(message, ...);
		#define ERROR_LOG(message, ...);
		#define CRITICAL_ERROR_LOG(message, ...);
		#define FATAL_ERROR_LOG(message, ...) std::abort();

		#define ASSERT(x, message);
		#define ASSERT_VK(x, message);
		#define ASSERT_FATAL(x, message, ...);
		#define ASSERT_ENUMS(...);
		#define ASSERT_STATIC_VAR(member);
		#define ASSERT_VK_STATIC_VARS();
		#define ASSERT_VK_STATIC_VARS_FUNC();

		#define ERROR_VK(message);
		#define WARN_VK(message);
		#define TRACE_VK(message);

	#endif

	#define STACK_ONLY(Type, ...)																															\
		template<typename TElement, std::size_t>																								\
		friend class vk::Array;																																	\
		private:          																																			\
			Type() = default;																																			\
		public:																																									\
			inline static auto create() noexcept { ASSERT_ENUMS(__VA_ARGS__) return Type(); }

	// @todo: figure out a way to make binding ids as const class members/enums,
	// 				while still can set all bindings in a function, so they'll be accessible outside class
	// 				and their values be used as indices for bindings array
	// @todo: make sure __COUNTER__ will not cause ODR violation in various scenarios
	#define BEGIN_DESC_SET_LAYOUT_BINDING_STRUCT(category)																					\
    struct category##_layout_binding																															\
		{                                                     																				\
			category##_layout_binding& operator=(const category##_layout_binding&) = delete;						\
			private:          																																					\
				category##_layout_binding() { set_bindings(); }																						\
			public:																																											\
				inline static auto create() noexcept { return category##_layout_binding(); }              \
			private:                                                                         						\
				using StageFlag = vk::shader::StageFlag;          																				\
      	using DescType	= vk::descriptor::Type;            																				\
      	using Desc			= vk::Descriptor;                             														\
			private:																																										\
				inline static constexpr uint16_t s_bindCountOffset = __COUNTER__ + 1;											\
			public:                                                                         						\
				inline auto get_bindings() noexcept { return m_bindings; }																\
			private:                                                                    								\
        template<uint16_t bindingIdx>                                       											\
				inline void set_binding(                                        													\
					const VkDescriptorType		&_type,                                  											\
					const VkShaderStageFlags	&_stage                               												\
				) noexcept                              																									\
				{                                                               													\
					m_bindings[bindingIdx] = Desc::createSetLayoutBinding<bindingIdx>(_type,	_stage);			\
				}																																													\
				inline void set_bindings() noexcept                          															\
				{

	#define LAYOUT_BINDING_UNIFORM_BUFFER(_id, _stage)                                        			\
					const uint16_t _id = (__COUNTER__ - s_bindCountOffset);																	\
					set_binding<_id>(DescType::UNIFORM_BUFFER,					_stage);
	#define LAYOUT_BINDING_COMBINED_IMAGE_SAMPLER(_id, _stage)																			\
					const uint16_t _id = (__COUNTER__ - s_bindCountOffset);																	\
					set_binding<_id>(DescType::COMBINED_IMAGE_SAMPLER,	_stage);
	#define LAYOUT_BINDING_STORAGE_BUFFER(_id, _stage)																							\
					const uint16_t _id	= (__COUNTER__ - s_bindCountOffset);																\
					set_binding<_id>(DescType::STORAGE_BUFFER,	_stage);

	#define END_DESC_SET_LAYOUT_BINDING_STRUCT()																										\
    		}                                          																								\
			private:																																										\
				inline static constexpr const uint16_t s_bindingCount = __COUNTER__ - s_bindCountOffset;	\
				vk::Array<VkDescriptorSetLayoutBinding, s_bindingCount> m_bindings;												\
		};

	#define USE_DESC_SET_LAYOUT_BINDING_STRUCT(wrapper, category)	\
		private: using category##_LAYOUT_BINDING = wrapper::category##_layout_binding;

	#define GET_DESC_SET_LAYOUT_BINDINGS(category) \
		category##_LAYOUT_BINDING::create().get_bindings();

#endif