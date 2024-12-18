/* proto_activities
 *
 * Copyright (c) 2022-2024, Framework Labs.
 */

#pragma once

/* Mode */

/* #define PA_PREFER_C to use C meta model over CPP for smaller code sizes */
#if defined(__cplusplus) && __has_include(<functional>) && !defined(PA_PREFER_C)
#define _PA_ENABLE_CPP
#endif

/* Includes */

#include <stdbool.h>
#include <stdint.h> /* for uint16_t etc. */
#include <string.h> /* for memset */
#ifdef _PA_ENABLE_CPP
#include <functional> /* for std::function */
#include <type_traits> /* for std::true_type, std::void_t, std::enable_if etc. */
#endif

/* Types */

typedef uint16_t pa_pc_t;
typedef int8_t pa_rc_t;
typedef uint32_t pa_time_t;

/* Constants */

#define PA_RC_WAIT ((pa_rc_t)-1)
#define PA_RC_DONE ((pa_rc_t)0)

/* Internals */

#define _pa_frame_name(nm) nm##_frame
#define _pa_frame_type(nm) struct _pa_frame_name(nm)
#define _pa_inst_name(nm) nm##_inst
#define _pa_inst_ptr(nm) &(pa_this->_pa_inst_name(nm))
#define _pa_call(nm, ...) nm(_pa_inst_ptr(nm), pa_current_time_ms, ##__VA_ARGS__)
#define _pa_call_as(nm, alias, ...) nm(_pa_inst_ptr(alias), pa_current_time_ms, ##__VA_ARGS__)
#ifndef _PA_ENABLE_CPP
#define _pa_reset(inst) memset(inst, 0, sizeof(*inst));
#define _pa_abort(inst) _pa_reset(inst); *inst._pa_pc = 0xffff;
#define _pa_static static
#define _pa_extern extern
#else
#define _pa_reset(inst) (inst)->reset();
#define _pa_abort(inst) _pa_reset(inst); (inst)->_pa_pc = 0xffff;
#define _pa_static
#define _pa_extern
#define _pa_has_field_definer(field) \
    namespace proto_activities { namespace internal { \
        template< class... > using void_t = void; \
        template <typename T, typename = void> \
        struct has_field_##field : std::false_type {}; \
        template <typename T> \
        struct has_field_##field<T, void_t<decltype(std::declval<T>().field)>> : std::true_type {}; \
    } }
#define _pa_has_field(ty, field) proto_activities::internal::has_field_##field<ty>::value
#endif

/* Context */

#define pa_ctx(vars...) vars
#define pa_ctx_tm(vars...) pa_ctx(pa_time_t _pa_time; vars)
#ifndef _PA_ENABLE_CPP
#define pa_use(nm) _pa_frame_type(nm) _pa_inst_name(nm);
#define pa_use_as(nm, alias) _pa_frame_type(nm) _pa_inst_name(alias);
#else
#define pa_use(nm) _pa_frame_type(nm) _pa_inst_name(nm){};
#define pa_use_ns(ns, nm) _pa_frame_type(ns::nm) _pa_inst_name(nm){};
#define pa_use_as(nm, alias) _pa_frame_type(nm) _pa_inst_name(alias){};
#define pa_use_as_ns(ns, nm, alias) _pa_frame_type(ns::nm) _pa_inst_name(alias){};
#endif
#define pa_self (*pa_this)

/* Activity */

#define pa_activity(nm, ctx, ...) \
    pa_activity_ctx(nm, ctx); \
    _pa_static pa_activity_def(nm, ##__VA_ARGS__)

#ifndef _PA_ENABLE_CPP
#define pa_activity_ctx(nm, ...) \
    struct _pa_frame_name(nm) { \
        pa_pc_t _pa_pc; \
        __VA_ARGS__; \
    };
#else
namespace proto_activities { namespace internal {
    struct AnyFrame {
        virtual ~AnyFrame() = default;
        virtual void reset() = 0;
        pa_pc_t _pa_pc{};
    };
} }
#define pa_activity_ctx(nm, ...) \
    struct _pa_frame_name(nm) final : proto_activities::internal::AnyFrame { \
        void reset() final { \
            *this = _pa_frame_name(nm){}; \
        } \
        __VA_ARGS__; \
    };
#endif

#define pa_activity_ctx_tm(nm, vars...) pa_activity_ctx(nm, pa_ctx_tm(vars))

#define pa_activity_def(nm, ...) \
    pa_rc_t nm(_pa_frame_type(nm)* pa_this, pa_time_t pa_current_time_ms, ##__VA_ARGS__) { \
        _pa_enter_invoke(_pa_frame_name(nm)); \
        switch (pa_this->_pa_pc) { \
            case 0: \
            case 0xffff:

#define pa_activity_end \
        } \
        pa_return; \
    }

#define pa_activity_sig(nm, ...) \
    _pa_extern pa_rc_t nm(_pa_frame_type(nm)* pa_this, pa_time_t pa_current_time_ms, ##__VA_ARGS__);

#define pa_activity_decl(nm, ctx, ...) \
    pa_activity_ctx(nm, ctx); \
    pa_activity_sig(nm, ##__VA_ARGS__);

#define pa_return \
    _pa_reset(pa_this); \
    return PA_RC_DONE;

/* Expert API */

#define pa_wait \
    return PA_RC_WAIT;

#define pa_mark_and_wait \
    pa_this->_pa_pc = __LINE__; pa_wait; case __LINE__:

#define pa_mark_and_wait2 \
    pa_this->_pa_pc = __LINE__ | 0x8000; pa_wait; case __LINE__ | 0x8000:

#define pa_mark_and_continue \
    pa_this->_pa_pc = __LINE__; case __LINE__:

/* Await */

#define pa_await(cond) \
    pa_mark_and_wait; \
    if (!(cond)) { \
        pa_wait; \
    }

/* Delay */

#define pa_delay(ticks) \
    pa_self._pa_time = ticks; \
    pa_mark_and_continue; \
    if (pa_self._pa_time-- > 0) { \
        pa_wait; \
    }

#define pa_delay_ms(ms) \
    pa_self._pa_time = pa_current_time_ms; \
    pa_mark_and_continue; \
    if (pa_current_time_ms - pa_self._pa_time < ms) { \
        pa_wait; \
    }

#define pa_delay_s(s) pa_delay_ms(s * 1000)

/* Run */

#define pa_run(nm, ...) \
    pa_mark_and_continue; \
    if (_pa_call(nm, ##__VA_ARGS__) == PA_RC_WAIT) { \
        pa_wait; \
    }

#define pa_run_as(nm, alias, ...) \
    pa_mark_and_continue; \
    if (_pa_call_as(nm, alias, ##__VA_ARGS__) == PA_RC_WAIT) { \
        pa_wait; \
    }

/* Concurrency */

#define pa_co_res(n) \
    pa_rc_t _pa_co_rcs[n];

#ifndef _PA_ENABLE_CPP

#define _pa_co_def(n) \
    struct { \
        void* addrs[n]; \
        size_t szs[n]; \
    } _pa_co;

#define _pa_co_clr(i) _pa_co.addrs[i] = NULL;

#define _pa_co_set(i, obj, nm) \
    _pa_co.addrs[i] = obj; \
    _pa_co.szs[i] = sizeof(_pa_frame_type(nm));

#define _pa_co_is_strong(i) (_pa_co.addrs[i] == NULL)

#define _pa_co_reset(i) memset(_pa_co.addrs[i], 0, _pa_co.szs[i]);

#define _pa_co_abort(i) \
    _pa_co_reset(i); \
    *(pa_pc_t*)(_pa_co.addrs[i]) = 0xffff;

#else

#define _pa_co_def(n) \
    proto_activities::internal::AnyFrame* _pa_co[n];

#define _pa_co_clr(i) _pa_co[i] = nullptr;

#define _pa_co_set(i, obj, nm) _pa_co[i] = obj;

#define _pa_co_is_strong(i) (_pa_co[i] == nullptr)

#define _pa_co_reset(i) _pa_reset(_pa_co[i]);

#define _pa_co_abort(i) _pa_abort(_pa_co[i]);

#endif

#define pa_co(n) \
    memset(pa_this->_pa_co_rcs, -1, sizeof(pa_rc_t) * n); \
    pa_mark_and_continue; \
    { \
        uint8_t _pa_co_i = 0; \
        _pa_co_def(n);

#define _pa_with_templ(nm, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_clr(_pa_co_i); \
        ++_pa_co_i;

#define _pa_with_weak_templ(nm, alias, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_set(_pa_co_i, _pa_inst_ptr(alias), nm); \
        ++_pa_co_i;

#define pa_with(nm, ...) _pa_with_templ(nm, _pa_call(nm, ##__VA_ARGS__));
#define pa_with_as(nm, alias, ...) _pa_with_templ(nm, _pa_call_as(nm, alias, ##__VA_ARGS__));

#define pa_with_weak(nm, ...) _pa_with_weak_templ(nm, nm, _pa_call(nm, ##__VA_ARGS__));
#define pa_with_weak_as(nm, alias, ...) _pa_with_weak_templ(nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__));

#define pa_co_end \
        { \
            bool _pa_any_is_strong = false; \
            bool _pa_any_is_done = false; \
            for (uint8_t i = 0; i < _pa_co_i; ++i) { \
                bool _pa_is_strong = _pa_co_is_strong(i); \
                bool _pa_is_waiting = pa_this->_pa_co_rcs[i] == PA_RC_WAIT; \
                if (_pa_is_strong && _pa_is_waiting) { \
                    pa_wait; \
                } \
                _pa_any_is_strong |= _pa_is_strong; \
                _pa_any_is_done |= !_pa_is_waiting; \
            } \
            if (!_pa_any_is_strong && !_pa_any_is_done) { \
                pa_wait; \
            } \
        } \
        for (uint8_t i = 0; i < _pa_co_i; ++i) { \
            if (!_pa_co_is_strong(i)) { \
                if (pa_this->_pa_co_rcs[i] == PA_RC_WAIT) { \
                    _pa_co_abort(i); \
                } \
                else { \
                    _pa_co_reset(i); \
                } \
            } \
        } \
    }

/* Preemption */

#define pa_did_abort(nm) (*_pa_inst_ptr(nm)._pa_pc == 0xffff)

#define _pa_when_abort_templ(cond, nm, alias, call) \
    if (call == PA_RC_WAIT) { \
        pa_mark_and_wait; \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                pa_wait; \
            } \
        } else { \
            _pa_abort(_pa_inst_ptr(alias)); \
        } \
    }

#define pa_when_abort(cond, nm, ...) _pa_when_abort_templ(cond, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_abort_as(cond, nm, alias, ...) _pa_when_abort_templ(cond, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_when_reset_templ(cond, nm, alias, call) \
    if (call == PA_RC_WAIT) { \
        pa_mark_and_wait; \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                pa_wait; \
            } \
        } else { \
            _pa_abort(_pa_inst_ptr(alias)); \
            if (call == PA_RC_WAIT) { \
                pa_wait; \
            } \
        } \
    }

#define pa_when_reset(cond, nm, ...) _pa_when_reset_templ(cond, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_reset_as(cond, nm, alias, ...) _pa_when_reset_templ(cond, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_when_suspend_templ(cond, nm, alias, call) \
    if (call == PA_RC_WAIT) { \
        pa_mark_and_wait \
        if (!(cond)) { \
            _pa_susres_resume(_pa_frame_name(nm), alias); \
            if (call == PA_RC_WAIT) { \
                pa_wait; \
            } \
        } else { \
            _pa_susres_suspend(_pa_frame_name(nm), alias); \
            pa_wait; \
        } \
    }

#define pa_when_suspend(cond, nm, ...) _pa_when_suspend_templ(cond, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_suspend_as(cond, nm, alias, ...) _pa_when_suspend_templ(cond, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_after_abort_templ(ticks, nm, alias, call) \
    pa_self._pa_time = ticks; \
    _pa_when_abort_templ(--pa_self._pa_time == 0, nm, alias, call);

#define pa_after_abort(ticks, nm, ...) _pa_after_abort_templ(ticks, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_after_abort_as(ticks, nm, alias, ...) _pa_after_abort_templ(ticks, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_after_ms_abort_templ(ms, nm, alias, call) \
    pa_self._pa_time = pa_current_time_ms; \
    _pa_when_abort_templ(pa_current_time_ms - pa_self._pa_time >= ms, nm, alias, call);

#define pa_after_ms_abort(ms, nm, ...) _pa_after_ms_abort_templ(ms, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_after_ms_abort_as(ms, nm, alias, ...) _pa_after_ms_abort_templ(ms, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define pa_after_s_abort(s, nm, ...) pa_after_ms_abort(s * 1000, nm, ##__VA_ARGS__)
#define pa_after_s_abort_as(s, nm, alias, ...) pa_after_ms_abort_as(s * 1000, nm, alias, ##__VA_ARGS__)

/* Lifecycle */

#ifndef _PA_ENABLE_CPP

#define _pa_susres_suspend(nm, alias)
#define _pa_susres_resume(nm, alias)
#define _pa_enter_invoke(ty)

#else

namespace proto_activities { namespace internal {
    using Thunk = std::function<void()>;

    struct Defer {
        Defer& operator=(const Defer& other) {
            if (thunk) {
                thunk();
                thunk = nullptr;
            }
            return *this;
        }
        Thunk thunk;
    };

    struct SusRes {
        SusRes& operator=(const SusRes& other) {
            sus_thunk = nullptr;
            res_thunk = nullptr;
            did_suspend = false;
            return *this;
        }
        void suspend() {
            if (did_suspend) {
                return;
            }
            did_suspend = true;
            if (sus_thunk) {
                sus_thunk();
            }
        }
        void resume() {
            if (!did_suspend) {
                return;
            }
            did_suspend = false;
            if (res_thunk) {
                res_thunk();
            }
        }
        Thunk sus_thunk;
        Thunk res_thunk;
        bool did_suspend{};
    };

    struct Updatable {
        virtual ~Updatable() = default;
        virtual void update() = 0;
        Updatable* next{};
    };

    struct Enter {
        Enter& operator=(const Enter& other) {
            thunk = nullptr;
            return *this;
        }
        Enter& operator=(Thunk&& thunk_) {
            thunk = std::move(thunk_);
            thunk();
            return *this;
        }
        void add(Updatable* updatable) {
            updatable->next = head;
            head = updatable;
        }
        void invoke() {
            for (auto* updatable = head; updatable != nullptr; updatable = updatable->next) {
                updatable->update();
            }
            if (thunk) {
                thunk();
            }
        }
        Thunk thunk;
        Updatable* head{};
    };
} }

#define pa_defer_res proto_activities::internal::Defer _pa_defer{};
#define pa_defer pa_self._pa_defer.thunk = [=]()

_pa_has_field_definer(_pa_susres);
namespace proto_activities { namespace internal {
    template <typename T>
    auto invoke_suspend(T* frame) -> typename std::enable_if<_pa_has_field(T, _pa_susres)>::type {
        frame->_pa_susres.suspend();
    }
    template <typename T>
    auto invoke_suspend(T* frame) -> typename std::enable_if<!_pa_has_field(T, _pa_susres)>::type {}
    template <typename T>
    auto invoke_resume(T* frame) -> typename std::enable_if<_pa_has_field(T, _pa_susres)>::type {
        frame->_pa_susres.resume();
    }
    template <typename T>
    auto invoke_resume(T* frame) -> typename std::enable_if<!_pa_has_field(T, _pa_susres)>::type {}
} }
#if __cplusplus >= 201703L
#define _pa_susres_suspend(ty, alias) \
    if constexpr (_pa_has_field(ty, _pa_susres)) { proto_activities::internal::invoke_suspend<ty>(_pa_inst_ptr(alias)); }
#define _pa_susres_resume(ty, alias) \
    if constexpr (_pa_has_field(ty, _pa_susres)) { proto_activities::internal::invoke_resume<ty>(_pa_inst_ptr(alias)); }
#else
#define _pa_susres_suspend(ty, alias) proto_activities::internal::invoke_suspend<ty>(_pa_inst_ptr(alias));
#define _pa_susres_resume(ty, alias) proto_activities::internal::invoke_resume<ty>(_pa_inst_ptr(alias));
#endif

#define pa_susres_res proto_activities::internal::SusRes _pa_susres{};
#define pa_suspend pa_self._pa_susres.sus_thunk = [&]()
#define pa_resume pa_self._pa_susres.res_thunk = [&]()

_pa_has_field_definer(_pa_enter);
namespace proto_activities { namespace internal {
    template <typename T>
    auto invoke_enter(T& frame) -> typename std::enable_if<_pa_has_field(T, _pa_enter)>::type {
        frame._pa_enter.invoke();
    }
    template <typename T>
    auto invoke_enter(T& frame) -> typename std::enable_if<!_pa_has_field(T, _pa_enter)>::type {}
} }
#if __cplusplus >= 201703L
#define _pa_enter_invoke(ty) if constexpr (_pa_has_field(ty, _pa_enter)) { proto_activities::internal::invoke_enter<ty>(pa_self); }
#else
#define _pa_enter_invoke(ty) proto_activities::internal::invoke_enter<ty>(pa_self);
#endif

#define pa_enter_res proto_activities::internal::Enter _pa_enter{};
#define pa_enter pa_self._pa_enter = [&]()

#endif

/* Signals */

#ifdef _PA_ENABLE_CPP

namespace proto_activities {
    struct Signal final : internal::Updatable {
        Signal(internal::Enter& enter) {
            enter.add(this);
        }
        Signal(const Signal&) = delete;
        Signal& operator=(const Signal&) {
            is_present_ = false;
            return *this;
        }
        void emit() {
            is_present_ = true;
        }
        operator bool() const {
            return is_present_;
        }
        void update() final {
            is_present_ = false;
        }
    private:
        bool is_present_{};
    };

    template <typename T>
    struct ValSignal final : internal::Updatable {
        ValSignal(internal::Enter& enter) {
            enter.add(this);
        }
        ValSignal(const ValSignal&) = delete;
        ValSignal& operator=(const ValSignal&) {
            is_present_ = false;
            has_emitted_val_ = false;
            value_ = {};
            return *this;
        }
        void emit(T&& val) {
            is_present_ = true;
            has_emitted_val_ = true;
            value_ = std::move(val);
        }
        operator bool() const {
            return is_present_;
        }
        bool has_emitted_val() const {
            return has_emitted_val_;
        }
        const T& val() const {
            return value_;
        }
        void update() final {
            is_present_ = false;
        }
    private:
        bool is_present_{};
        bool has_emitted_val_{};
        T value_{};
    };
}

using pa_signal = proto_activities::Signal;
#define pa_signal_res pa_enter_res
#define pa_def_signal(sig) pa_signal sig{_pa_enter};
#define pa_emit(sig) sig.emit();

template <typename T>
using pa_val_signal = proto_activities::ValSignal<T>;
#define pa_def_val_signal(ty, sig) pa_val_signal<ty> sig{_pa_enter};
#define pa_emit_val(sig, val) sig.emit(val);

#endif

/* Trigger */

#define pa_init(nm) _pa_reset(&_pa_inst_name(nm));
#define pa_tick_tm(tm, nm, ...) nm(&_pa_inst_name(nm), tm, ##__VA_ARGS__)
#ifndef ARDUINO
#define pa_tick(nm, ...) pa_tick_tm(0, nm, ##__VA_ARGS__)
#else
#define pa_tick(nm, ...) pa_tick_tm(millis(), nm, ##__VA_ARGS__)
#endif

/* Convenience */

#define pa_end pa_activity_end

#define pa_pause pa_await (true);
#define pa_halt pa_await (false);

#define pa_await_immediate(cond) \
    if (!(cond)) { \
        pa_await (cond); \
    }

#define pa_repeat \
    while (true)

#define pa_always \
    pa_repeat {

#define pa_always_end \
        pa_pause; \
    }

#define pa_every(cond) \
    pa_repeat { \
        pa_await_immediate (cond);

#define pa_every_ms(ms) \
    pa_self._pa_time = pa_current_time_ms - ms; \
    pa_repeat { \
        pa_await_immediate (pa_current_time_ms - pa_self._pa_time >= ms); \
        pa_self._pa_time += ms;

#define pa_every_s(s) pa_every_ms(s * 1000)

#define pa_every_end \
        pa_pause; \
    }

#define _pa_whenever_templ(cond, nm, alias, call) \
    pa_repeat { \
        if (cond) {\
            _pa_when_abort_templ (!(cond), nm, alias, call); \
        } else { \
            pa_mark_and_wait2; \
        } \
    }

#define pa_whenever(cond, nm, ...) _pa_whenever_templ(cond, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_whenever_as(cond, nm, alias, ...) _pa_whenever_templ(cond, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))
