#pragma once

#include <atomic>
#include <cstdint>

#ifdef AX_DEBUG
#include <mutex>
#include <string>
#include <typeinfo>
#include <unordered_map>

#include "AxionEngine/Source/core/Logging.h"
#endif

namespace Axion {

	// ----- RefTracker -----
	#ifdef AX_DEBUG
	class RefTracker {
	public:

		static void track(const void* ptr, const std::string& typeName) {
			std::lock_guard<std::mutex> lock(s_mutex);
		}

		static void untrack(const void* ptr) {
			std::lock_guard<std::mutex> lock(s_mutex);
			s_liveObjects.erase(ptr);
		}

		static void dump() {
			std::lock_guard<std::mutex> lock(s_mutex);
			if (s_liveObjects.empty()) {
				AX_CORE_LOG_INFO("RefTracker: Zero live references. No leaks detected!");
				return;
			}

			AX_CORE_LOG_ERROR("RefTracker: Detected {0} Memory Leaks!", s_liveObjects.size());
			for (const auto& kv : s_liveObjects) {
				AX_CORE_LOG_ERROR("  -> Leak: [{0}] at address {1}", kv.second, kv.first);
			}
		}

	private:

		inline static std::unordered_map<const void*, std::string> s_liveObjects;
		inline static std::mutex s_mutex;

	};
	#endif



	// ----- RefCounted -----
	class RefCounted {
	public:

		virtual ~RefCounted() {
			#ifdef AX_DEBUG
			RefTracker::untrack(this);
			#endif
		}

		void incRefCount() const { ++m_refCount; }
		void decRefCount() const {
			if (--m_refCount == 0) {
				delete this;
			}
		}

		uint32_t getRefCount() const { return m_refCount.load(); }

	private:

		mutable std::atomic<uint32_t> m_refCount{ 0 };

	};



	// ----- Ref -----
	template<typename T>
	class Ref {
	public:

		constexpr Ref() : m_ptr(nullptr) {}
		constexpr Ref(std::nullptr_t) : m_ptr(nullptr) {}

		Ref(T* ptr) : m_ptr(ptr) {
			#ifdef AX_DEBUG
			if (m_ptr && m_ptr->getRefCount() == 0) RefTracker::track(m_ptr, typeid(T).name());
			#endif
			incRef();
		}

		Ref(const Ref<T>& other) : m_ptr(other.m_ptr) { incRef(); }

		template<typename U>
		Ref(const Ref<U>& other) : m_ptr((T*)other.get()) { incRef(); }

		Ref& operator=(const Ref<T>& other) {
			if (this != &other) {
				decRef();
				m_ptr = other.m_ptr;
				incRef();
			}
			return *this;
		}

		Ref(Ref<T>&& other) noexcept : m_ptr(other.m_ptr) {
			other.m_ptr = nullptr;
		}

		Ref& operator=(Ref<T>&& other) noexcept {
			if (this != &other) {
				decRef();
				m_ptr = other.m_ptr;
				other.m_ptr = nullptr;
			}
			return *this;
		}

		Ref& operator=(std::nullptr_t) {
			decRef();
			m_ptr = nullptr;
			return *this;
		}

		~Ref() { decRef(); }

		operator bool() const { return m_ptr != nullptr; }
		T* operator->() const { return m_ptr; }
		T& operator*() const { return *m_ptr; }
		T* get() const { return m_ptr; }

		void reset(T* ptr = nullptr) {
			decRef();
			#ifdef AX_DEBUG
			if (ptr && ptr->getRefCount() == 0) RefTracker::track(ptr, typeid(T).name());
			#endif
			m_ptr = ptr;
			incRef();
		}

		template<typename U>
		Ref<U> as() const {
			return Ref<U>(dynamic_cast<U*>(m_ptr));
		}

		template<typename U>
		Ref<U> staticAs() const {
			return Ref<U>(static_cast<U*>(m_ptr));
		}

		bool operator==(const Ref<T>& other) const { return m_ptr == other.m_ptr; }
		bool operator!=(const Ref<T>& other) const { return !(*this == other); }
		bool operator==(std::nullptr_t) const { return m_ptr == nullptr; }
		bool operator!=(std::nullptr_t) const { return m_ptr != nullptr; }

	private:

		T* m_ptr = nullptr;

		void incRef() const { if (m_ptr) m_ptr->incRefCount(); }
		void decRef() const { if (m_ptr) m_ptr->decRefCount(); }

	};



	// ----- MakeRef -----
	template<typename T, typename... Args>
	Ref<T> MakeRef(Args&&... args) {
		return Ref<T>(new T(std::forward<Args>(args)...));
	}

}
