/* proto_activities
 *
 * Copyright (c) 2022-2024, Framework Labs.
 */

#pragma once

/* Includes */

#include <stdbool.h>
#include <stdint.h> /* for uint16_t etc. */
#include <string.h> /* for memset */

/* Types */

typedef uint16_t pa_pc_t;
typedef int8_t pa_rc_t;
typedef uint64_t pa_time_t;

/* Constants */

#define PA_RC_WAIT ((pa_rc_t)-1)
#define PA_RC_DONE ((pa_rc_t)0)

/* Internals */

#define _pa_frame_name(nm) nm##_frame
#define _pa_frame_type(nm) struct _pa_frame_name(nm)
#define _pa_inst_name(nm) nm##_inst
#define _pa_inst_ptr(nm) &(pa_this->_pa_inst_name(nm))
#define _pa_call(nm, ...) nm(_pa_inst_ptr(nm), ##__VA_ARGS__)
#define _pa_call_as(nm, alias, ...) nm(_pa_inst_ptr(alias), ##__VA_ARGS__)
#define _pa_reset(inst) memset(inst, 0, sizeof(*inst));
#define _pa_abort(inst) _pa_reset(inst); *inst._pa_pc = 0xffff;

/* Context */

#define pa_ctx(vars...) vars
#define pa_ctx_tm(vars...) pa_ctx(pa_time_t _pa_time; vars)
#define pa_use(nm) _pa_frame_type(nm) _pa_inst_name(nm);
#define pa_use_as(nm, alias) _pa_frame_type(nm) _pa_inst_name(alias);
#define pa_self (*pa_this)

/* Activity */

#define pa_activity(nm, ctx, ...) \
    pa_activity_ctx(nm, ctx); \
    static pa_activity_def(nm, ##__VA_ARGS__)

#define pa_activity_ctx(nm, ...) \
    struct _pa_frame_name(nm) { \
        pa_pc_t _pa_pc; \
        __VA_ARGS__; \
    };

#define pa_activity_def(nm, ...) \
    pa_rc_t nm(_pa_frame_type(nm)* pa_this, ##__VA_ARGS__) { \
        switch (pa_this->_pa_pc) { \
            case 0: \
            case 0xffff:

#define pa_activity_end \
        } \
        pa_return; \
    }

#define pa_activity_sig(nm, ...) \
    extern pa_rc_t nm(_pa_frame_type(nm)* pa_this, ##__VA_ARGS__);

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

/* Define like this (Arduino) for your platform: #define pa_get_time_ms millis() */

#define pa_delay_ms(ms) \
    pa_self._pa_time = pa_get_time_ms + ms; \
    pa_mark_and_continue; \
    if (pa_get_time_ms < pa_self._pa_time) { \
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

#define pa_co(n) \
    memset(pa_this->_pa_co_rcs, -1, sizeof(pa_rc_t) * n); \
    pa_mark_and_continue; \
    { \
        uint8_t _pa_co_i = 0; \
        void* _pa_co_addrs[n]; \
        size_t _pa_co_szs[n];

#define _pa_with_templ(nm, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_addrs[_pa_co_i] = NULL; \
        ++_pa_co_i;

#define _pa_with_weak_templ(nm, alias, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_addrs[_pa_co_i] = _pa_inst_ptr(alias); \
        _pa_co_szs[_pa_co_i] = sizeof(_pa_frame_type(nm)); \
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
                bool _pa_is_strong = _pa_co_addrs[i] == NULL; \
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
            if (_pa_co_addrs[i]) { \
                memset(_pa_co_addrs[i], 0, _pa_co_szs[i]); \
                if (pa_this->_pa_co_rcs[i] == PA_RC_WAIT) { \
                    *(pa_pc_t*)(_pa_co_addrs[i]) = 0xffff; \
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

#define _pa_when_suspend_templ(cond, nm, call) \
    if (call == PA_RC_WAIT) { \
        pa_mark_and_wait \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                pa_wait; \
            } \
        } else { \
            pa_wait; \
        } \
    }

#define pa_when_suspend(cond, nm, ...) _pa_when_suspend_templ(cond, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_suspend_as(cond, nm, alias, ...) _pa_when_suspend_templ(cond, nm, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_after_abort_templ(ticks, nm, alias, call) \
    pa_self._pa_time = ticks; \
    _pa_when_abort_templ(--pa_self._pa_time == 0, nm, alias, call);

#define pa_after_abort(ticks, nm, ...) _pa_after_abort_templ(ticks, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_after_abort_as(ticks, nm, alias, ...) _pa_after_abort_templ(ticks, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_after_ms_abort_templ(ms, nm, alias, call) \
    pa_self._pa_time = pa_get_time_ms + ms; \
    _pa_when_abort_templ(pa_get_time_ms >= pa_self._pa_time, nm, alias, call);

#define pa_after_ms_abort(ms, nm, ...) _pa_after_ms_abort_templ(ms, nm, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_after_ms_abort_as(ms, nm, alias, ...) _pa_after_ms_abort_templ(ms, nm, alias, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define pa_after_s_abort(s, nm, ...) pa_after_ms_abort(s * 1000, nm, ##__VA_ARGS__)
#define pa_after_s_abort_as(s, nm, alias, ...) pa_after_ms_abort_as(s * 1000, nm, alias, ##__VA_ARGS__)

/* Trigger */

#define pa_init(nm) _pa_reset(&_pa_inst_name(nm));
#define pa_tick(nm, ...) nm(&_pa_inst_name(nm), ##__VA_ARGS__)

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
