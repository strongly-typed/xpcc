// coding: utf-8
/* Copyright (c) 2014, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the xpcc library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef XPCC_COROUTINE_HPP
#define XPCC_COROUTINE_HPP

#include "macros.hpp"
#include <xpcc/utils/arithmetic_traits.hpp>

namespace xpcc
{

namespace co
{

/// State of a coroutine.
enum State
{
	// reasons to stop
	Stop = 0,					///< Coroutine finished
	NestingError = (1 << 0),	///< Nested Coroutine has run out of nesting levels

	// reasons to wait
	WrongContext = (1 << 5),	///< Coroutine already running in a different context
	WrongState = (1 << 6),		///< Another coroutine of the same class is already running in this context

	// reasons to keep running
	Running = (1 << 7),			///< Coroutine is running
};

/// All Coroutines return an encapsulated result type.
template < typename T = bool >
struct Result
{
	uint8_t state;
	T result;
};

/**
 * An implementation of Coroutines which allow for nested calling.
 *
 * This base class and its macros allows you to implement and use several
 * coroutines in one class.
 * This allows you to modularize your code by placing it into its own coroutines
 * instead of the placing everything into one big method.
 * It also allows you to call and run coroutines within your coroutines,
 * so you can reuse their functionality.
 *
 * You are responsible to choosing the right nesting depth!
 * This class will guard itself against calling another coroutines at too
 * deep a nesting level and inform you gently of this by returning
 * `xpcc::co::NestingError` from your called `coroutine(ctx)`.
 * It is then up to you to recognise this in your program design
 * and increase the nesting depth or rethink your code.
 *
 * Be aware that only one coroutines of the same object can run at the
 * same time. Even if you call other coroutines, they will not allow you to
 * run them until the conflicting coroutines finished.
 *
 * Each coroutine requires a context pointer, which is used to ensure the
 * coroutine only executes in the context that it was originally started from.
 * If you call a coroutine in a different context, it will return
 * `xpcc::co::WrongContext`.
 * Similarly, if you call a different coroutine of the same class in
 * the same context, while another coroutine is running, it will return
 * `xpcc::co::WrongState`.
 * Using the `CO_CALL` macro, you can wait for these coroutine to become
 * available and then run them, so you usually do not need to worry
 * about those cases.
 *
 * You may exit the coroutine successfully by using `CO_EXIT_SUCCESS`.
 * This information is returned by the `CO_CALL` macro and can be used
 * to influence your program flow.
 * You may exit the coroutine unsuccessfully by using `CO_EXIT()`.
 * If the coroutine reaches `CO_END()` it will exit unsuccessfully
 * automatically.
 *
 * Note that you should call coroutines within a protothreads.
 * It is sufficient to use the `this` pointer of the class as context
 * when calling the coroutines.
 *
 * Here is a (slightly over-engineered) example:
 *
 * @code
 * #include <xpcc/architecture.hpp>
 * #include <xpcc/processing/protothread.hpp>
 * #include <xpcc/coroutine/coroutine.hpp>
 * #include <xpcc/processing/timeout.hpp>
 *
 * typedef GpioOutputB0 Led;
 *
 * class BlinkingLight : public xpcc::pt::Protothread, private xpcc::pt::NestedCoroutine<1>
 * {
 * public:
 *     bool
 *     run()
 *     {
 *         PT_BEGIN();
 *
 *         // set everything up
 *         Led::setOutput();
 *         Led::set();
 *
 *         while (true)
 *         {
 *             Led::set();
 *             PT_CALL(waitForTimer(this)))
 *
 *             Led::reset();
 *             PT_CALL(setTimer(this, 200));
 *
 *             PT_WAIT_UNTIL(timer.isExpired());
 *         }
 *
 *         PT_END();
 *     }
 *
 *     xpcc::co::Result<>
 *     waitForTimer(void *ctx)
 *     {
 *         CO_BEGIN(ctx);
 *
 *         // nested calling is allowed
 *         if (CO_CALL(setTimer(ctx, 100)))
 *         {
 *             CO_WAIT_UNTIL(timer.isExpired());
 *         }
 *
 *         // CO_RETURN is optional
 *
 *         CO_END();
 *     }
 *
 *     xpcc::co::Result<bool>
 *     setTimer(void *ctx, uint16_t timeout)
 *     {
 *         CO_BEGIN(ctx);
 *
 *         timer.restart(timeout);
 *
 *         if(timer.isRunning())
 *             CO_RETURN(true);
 *
 *         // clean up code goes here
 *
 *         CO_END();
 *     }
 *
 * private:
 *     xpcc::Timeout<> timer;
 * };
 *
 *
 * ...
 * BlinkingLight light;
 *
 * while (...) {
 *     light.run();
 * }
 * @endcode
 *
 * For other examples take a look in the `examples` folder in the XPCC
 * root folder.
 *
 * @ingroup	coroutine
 * @author	Niklas Hauser
 * @tparam	Depth	the nesting depth: the maximum of tasks that are called within tasks (should be < 128).
 */
template< uint8_t Depth >
class NestedCoroutine
{
protected:
	/// Used to store a protothread's position (what Dunkels calls a
	/// "local continuation").
	typedef uint16_t CoState;

	/// Construct a new nested protothread which will be stopped
	NestedCoroutine()
	:	coLevel(0), coContext(0)
	{
		this->stopCoroutine();
	}

public:
	/// Force the task to stop running at the current nesting level
	/// @warning	This will not allow the task to clean itself up!
	inline void
	stopCoroutine()
	{
		uint_fast8_t level = coLevel;
		while (level < Depth + 1)
		{
			coStateArray[level++] = CoStopped;
		}
		if (coLevel == 0)
			coContext = 0;
	}

	/// @return	`true` if a task is running at the current nesting level, else `false`
	bool ALWAYS_INLINE
	isCoroutineRunning() const
	{
		return !isStoppedCo();
	}

	/// @return the nesting depth in the current task, or -1 if called outside a task
	int8_t ALWAYS_INLINE
	getCoroutineDepth() const
	{
		return static_cast<int8_t>(coLevel) - 1;
	}

#ifdef __DOXYGEN__
	/**
	 * Run the coroutine.
	 *
	 * You need to implement this method in you subclass yourself.
	 *
	 * @return	>`NestingError` if still running, <=`NestingError` if it has finished.
	 */
	xpcc::co::Result< ReturnType >
	coroutine(void *ctx, ...);
#endif

protected:
	/// @internal
	/// @{

	/// increases nesting level, call this in the switch statement!
	/// @return current state before increasing nesting level
	CoState ALWAYS_INLINE
	pushCo()
	{
		return coStateArray[coLevel++];
	}

	/// always call this before returning from the run function!
	/// decreases nesting level
	void ALWAYS_INLINE
	popCo()
	{
		coLevel--;
	}

	// invalidates the parent nesting level
	// @warning	be aware in which nesting level you call this! (before popCo()!)
	void inline
	stopCo()
	{
		coStateArray[coLevel-1] = CoStopped;
		if (coLevel == 1)
			coContext = 0;
	}

	/// sets the state of the parent nesting level
	/// @warning	be aware in which nesting level you call this! (before popCo()!)
	void ALWAYS_INLINE
	setCo(CoState state)
	{
		coStateArray[coLevel-1] = state;
	}

	/// @return `true` if the nesting depth allows for another level.
	/// @warning	be aware in which nesting level you call this! (before pushCo()!)
	bool ALWAYS_INLINE
	nestingOkCo() const
	{
		return (coLevel < Depth + 1);
	}

	bool ALWAYS_INLINE
	isStoppedCo() const
	{
		return (coStateArray[coLevel] == CoStopped);
	}

	/// @return `true` if the task is called in the same context, or the context is not set
	///			`false` if the task is claimed by another context
	bool inline
	beginCo(void *ctx)
	{
		// only one comparison, if this task is called in the same context
		if (ctx == coContext)
			return true;

		// two comparisons + assignment, if this task is called for the first time
		if (coContext == 0)
		{
			coContext = ctx;
			return true;
		}

		return false;
	}
	/// @}

protected:
	static constexpr CoState CoStopped = static_cast<CoState>(0);
private:
	uint8_t coLevel;
	CoState coStateArray[Depth+1];
	void *coContext;
};

// ----------------------------------------------------------------------------
// we won't document the specialisation again
/// @cond
template <>
class NestedCoroutine<0>
{
protected:
	typedef uint16_t CoState;

	NestedCoroutine() :
		coState(CoStopped), coLevel(-1), coContext(0)
	{
	}

public:
	void ALWAYS_INLINE
	stopCoroutine()
	{
		this->stopCo();
	}

	bool ALWAYS_INLINE
	isCoroutineRunning() const
	{
		return !isStoppedCo();
	}

	int8_t ALWAYS_INLINE
	getCoroutineDepth() const
	{
		return coLevel;
	}

protected:
	/// @internal
	/// @{
	CoState ALWAYS_INLINE
	pushCo()
	{
		coLevel = 0;
		return coState;
	}

	void ALWAYS_INLINE
	popCo()
	{
		coLevel = -1;
	}

	void ALWAYS_INLINE
	stopCo()
	{
		coState = CoStopped;
		coContext = 0;
	}

	bool ALWAYS_INLINE
	nestingOkCo() const
	{
		return (coLevel != 0);
	}

	void ALWAYS_INLINE
	setCo(CoState state)
	{
		coState = state;
	}

	bool ALWAYS_INLINE
	isStoppedCo() const
	{
		return (coState == CoStopped);
	}

	bool inline
	beginCo(void *ctx)
	{
		if (ctx == coContext)
			return true;

		if (coContext == 0)
		{
			coContext = ctx;
			return true;
		}

		return false;
	}
	/// @}

protected:
	static constexpr CoState CoStopped = static_cast<CoState>(0);
private:
	CoState coState;
	int8_t coLevel;
	void *coContext;
};
/// @endcond

typedef NestedCoroutine<0> Coroutine;

} // namespace co

} // namespace xpcc

#endif // XPCC_COROUTINE_HPP
