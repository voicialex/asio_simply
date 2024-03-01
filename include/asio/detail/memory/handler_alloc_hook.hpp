#ifndef ASIO_HANDLER_ALLOC_HOOK_HPP
#define ASIO_HANDLER_ALLOC_HOOK_HPP

#include "asio/detail/config.hpp"
#include <cstddef>

#include "asio/detail/push_options.hpp"

namespace asio {

/// Default allocation function for handlers.
/**
 * Asynchronous operations may need to allocate temporary objects. Since
 * asynchronous operations have a handler function object, these temporary
 * objects can be said to be associated with the handler.
 *
 * Implement asio_handler_allocate and asio_handler_deallocate for your own
 * handlers to provide custom allocation for these temporary objects.
 *
 * The default implementation of these allocation hooks uses <tt>::operator
 * new</tt> and <tt>::operator delete</tt>.
 *
 * @note All temporary objects associated with a handler will be deallocated
 * before the upcall to the handler is performed. This allows the same memory to
 * be reused for a subsequent asynchronous operation initiated by the handler.
 *
 * @par Example
 * @code
 * class my_handler;
 *
 * void* asio_handler_allocate(std::size_t size, my_handler* context)
 * {
 *   return ::operator new(size);
 * }
 *
 * void asio_handler_deallocate(void* pointer, std::size_t size,
 *     my_handler* context)
 * {
 *   ::operator delete(pointer);
 * }
 * @endcode
 */
ASIO_DECL void* asio_handler_allocate(
    std::size_t size, ...);

/// Default deallocation function for handlers.
/**
 * Implement asio_handler_allocate and asio_handler_deallocate for your own
 * handlers to provide custom allocation for the associated temporary objects.
 *
 * The default implementation of these allocation hooks uses <tt>::operator
 * new</tt> and <tt>::operator delete</tt>.
 *
 * @sa asio_handler_allocate.
 */
ASIO_DECL void asio_handler_deallocate(
    void* pointer, std::size_t size, ...);

} // namespace asio

#include "asio/detail/pop_options.hpp"

#if defined(ASIO_HEADER_ONLY)
# include "asio/detail/memory/handler_alloc_hook.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_HANDLER_ALLOC_HOOK_HPP
