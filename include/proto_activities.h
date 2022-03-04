/* proto_activities
 *
 * Copyright (c) 2022, Framework Labs.
 */

/* Includes */

#include <stdbool.h>
#include <stdint.h> // for uint16_t
#include <string.h> // for memset

/* Types */

typedef uint16_t pa_pc_t;
typedef int8_t pa_rc_t;

/* Constants */

#define PA_RC_WAIT ((pa_rc_t)-1)
#define PA_RC_DONE ((pa_rc_t)0)

/* Internals */

#define _pa_frame_name(nm) nm##_frame
#define _pa_frame_type(nm) struct _pa_frame_name(nm)
#define _pa_inst_name(nm) nm##_inst
#define _pa_call(nm, ...) nm(&(pa_this->_pa_inst_name(nm)), ##__VA_ARGS__)
#define _pa_call_as(nm, alias, ...) nm(&(pa_this->_pa_inst_name(alias)), ##__VA_ARGS__)

/* Context */

#define pa_ctx(vars...) vars
#define pa_use(nm) _pa_frame_type(nm) _pa_inst_name(nm);
#define pa_use_as(nm, alias) _pa_frame_type(nm) _pa_inst_name(alias);
#define pa_self (*pa_this)

/* Activity */

#define pa_activity(nm, ctx, ...) \
    struct _pa_frame_name(nm) { \
        pa_pc_t _pa_pc; \
        ctx; \
    }; \
    static pa_rc_t nm(_pa_frame_type(nm)* pa_this, ##__VA_ARGS__) { \
        switch (pa_this->_pa_pc) { \
            case 0:

#define pa_activity_end \
        } \
        memset(pa_this, 0, sizeof(*pa_this)); \
        return PA_RC_DONE; \
    }

#define pa_end pa_activity_end

/* Moduls */

#define pa_activity_ctx(nm, ...) \
    struct _pa_frame_name(nm) { \
        pa_pc_t _pa_pc; \
        __VA_ARGS__; \
    }; \

#define pa_activity_sig(nm, ...) \
    extern pa_rc_t nm(_pa_frame_type(nm)* pa_this, ##__VA_ARGS__);

#define pa_activity_decl(nm, ctx, ...) \
    pa_activity_ctx(nm, ctx); \
    pa_activity_sig(nm, ##__VA_ARGS__);

#define pa_activity_def(nm, ...) \
    pa_rc_t nm(_pa_frame_type(nm)* pa_this, ##__VA_ARGS__) { \
        switch (pa_this->_pa_pc) { \
            case 0:

/* Await */

#define pa_await(cond) \
    pa_this->_pa_pc = __LINE__; return PA_RC_WAIT; case __LINE__: \
    if (!(cond)) { \
        return PA_RC_WAIT; \
    }

/* Run */

#define pa_run(nm, ...) pa_this->_pa_pc = __LINE__; case __LINE__: \
    if (_pa_call(nm, ##__VA_ARGS__) == PA_RC_WAIT) { \
        return PA_RC_WAIT; \
    }

#define pa_run_as(nm, alias, ...) pa_this->_pa_pc = __LINE__; case __LINE__: \
    if (_pa_call_as(nm, alias, ##__VA_ARGS__) == PA_RC_WAIT) { \
        return PA_RC_WAIT; \
    }

/* Concurrency */

#define pa_co_res(n) \
    pa_rc_t _pa_co_rcs[n];

#define pa_co(n) \
    memset(pa_this->_pa_co_rcs, -1, sizeof(pa_rc_t) * n); \
    pa_this->_pa_pc = __LINE__; case __LINE__: \
    { \
        int _pa_co_i = 0; \
        bool _pa_co_weaks[n]; \
        void* _pa_co_addrs[n]; \
        size_t _pa_co_szs[n];

#define _pa_with_templ(nm, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_weaks[_pa_co_i] = false; \
        _pa_co_addrs[_pa_co_i] = NULL; \
        ++_pa_co_i;

#define _pa_with_weak_templ(nm, alias, call) \
        if (pa_this->_pa_co_rcs[_pa_co_i] == PA_RC_WAIT) { \
            pa_this->_pa_co_rcs[_pa_co_i] = call; \
        } \
        _pa_co_weaks[_pa_co_i] = true; \
        _pa_co_addrs[_pa_co_i] = &(pa_this->_pa_inst_name(alias)); \
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
            for (int i = 0; i < _pa_co_i; ++i) { \
                bool _pa_is_strong = !_pa_co_weaks[i]; \
                bool _pa_is_waiting = pa_this->_pa_co_rcs[i] == PA_RC_WAIT; \
                if (_pa_is_strong && _pa_is_waiting) { \
                    return PA_RC_WAIT; \
                } \
                _pa_any_is_strong |= _pa_is_strong; \
                _pa_any_is_done |= !_pa_is_waiting; \
            } \
            if (!_pa_any_is_strong && !_pa_any_is_done) { \
                return PA_RC_WAIT; \
            } \
        } \
        for (int i = 0; i < _pa_co_i; ++i) { \
            if (_pa_co_addrs[i]) { \
                memset(_pa_co_addrs[i], 0, _pa_co_szs[i]); \
            } \
        } \
    }

/* Preemption */

#define _pa_when_abort_templ(cond, nm, call) \
    if (call == PA_RC_WAIT) { \
        pa_this->_pa_pc = __LINE__; return PA_RC_WAIT; case __LINE__: \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                return PA_RC_WAIT; \
            } \
        } else { \
            memset(&(pa_this->_pa_inst_name(nm)), 0, sizeof(_pa_frame_type(nm))); \
        } \
    }

#define pa_when_abort(cond, nm, ...) _pa_when_abort_templ(cond, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_abort_as(cond, nm, alias, ...) _pa_when_abort_templ(cond, nm, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_when_reset_templ(cond, nm, call) \
    if (call == PA_RC_WAIT) { \
        pa_this->_pa_pc = __LINE__; return PA_RC_WAIT; case __LINE__: \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                return PA_RC_WAIT; \
            } \
        } else { \
            memset(&(pa_this->_pa_inst_name(nm)), 0, sizeof(_pa_frame_type(nm))); \
            if (call == PA_RC_WAIT) { \
                return PA_RC_WAIT; \
            } \
        } \
    }

#define pa_when_reset(cond, nm, ...) _pa_when_reset_templ(cond, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_reset_as(cond, nm, alias, ...) _pa_when_reset_templ(cond, nm, _pa_call_as(nm, alias, ##__VA_ARGS__))

#define _pa_when_suspend_templ(cond, nm, call) \
    if (call == PA_RC_WAIT) { \
        pa_this->_pa_pc = __LINE__; return PA_RC_WAIT; case __LINE__: \
        if (!(cond)) { \
            if (call == PA_RC_WAIT) { \
                return PA_RC_WAIT; \
            } \
        } else { \
            return PA_RC_WAIT; \
        } \
    }

#define pa_when_suspend(cond, nm, ...) _pa_when_suspend_templ(cond, nm, _pa_call(nm, ##__VA_ARGS__))
#define pa_when_suspend_as(cond, nm, alias, ...) _pa_when_suspend_templ(cond, nm, _pa_call_as(nm, alias, ##__VA_ARGS__))

/* Trigger */

#define pa_init(nm) memset(&_pa_inst_name(nm), 0, sizeof(_pa_frame_type(nm)));
#define pa_tick(nm, ...) nm(&_pa_inst_name(nm), ##__VA_ARGS__)

/* Convenience */

#define pa_pause pa_await (true);
#define pa_halt pa_await (false);

#define pa_always \
    while (true) {

#define pa_always_end \
        pa_pause; \
    }

#define pa_every(cond) \
    while (true) { \
        if (!(cond)) { \
            pa_await (cond); \
        }

#define pa_every_end \
        pa_pause; \
    }
