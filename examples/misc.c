/* misc.c */

/* Includes */

#include <proto_activities.h>

#include <stdio.h>
#include <assert.h>

/* Helper functions */

static char* int_to_str(uint16_t val) {
    static char buf[8];
    snprintf(buf, sizeof(buf), "%u", val);
    return buf;
}

/* Activities */

pa_activity (Counter, pa_ctx(uint16_t i), uint16_t n, const char* prefix) {
    assert(pa_self.i == 0);
    
    pa_self.i = n;
    while (pa_self.i-- > 0) {
        pa_pause;
        printf("%s %d\n", prefix, pa_self.i);
    }
} pa_activity_end;

pa_activity (Generator, pa_ctx(uint16_t i), uint16_t* valp) {
    assert(pa_self.i == 0);
    
    pa_self.i = 0;
    pa_always {
        *valp = pa_self.i++;
    } pa_always_end;
} pa_activity_end;

pa_activity (Printer, pa_ctx(), const char* prefix, const char* str) {
    pa_always {
        printf("%s %s\n", prefix, str);
    } pa_always_end;
} pa_activity_end;

pa_activity (SubActivity, pa_ctx(pa_use(Printer); pa_use(Counter)), uint16_t val) {
    pa_when_abort (val >= 10, Printer, "abort", ".");
    printf("#\n");
    
    pa_when_abort (val > 100, Counter, 10, "X");
    printf("!\n");
} pa_activity_end;

pa_activity (CountAndPrint, pa_ctx(pa_co_res(2); pa_use(Printer); pa_use(Generator)), uint16_t* i) {
    pa_co(2) {
        pa_with (Generator, i);
        pa_with (Printer, "reset", int_to_str(*i));
    } pa_co_end;
} pa_activity_end;

pa_activity (ResetActivity, pa_ctx(uint16_t i; pa_use(CountAndPrint))) {
    pa_when_reset (pa_self.i >= 3, CountAndPrint, &pa_self.i);
} pa_activity_end;

pa_activity (SuspendActivity, pa_ctx(pa_use(Printer)), uint16_t i) {
    pa_when_suspend (i % 2 == 0, Printer, "suspend", int_to_str(i));
} pa_activity_end;

pa_activity (Main, pa_ctx(pa_co_res(5); uint16_t i;
                          pa_use(Counter); pa_use_as(Counter, Counter_1);
                          pa_use(Generator); pa_use(Printer); pa_use(SubActivity);
                          pa_use(ResetActivity); pa_use(SuspendActivity)))
{
    assert(pa_self.i == 0);
    
    printf("First sequential\n");
    pa_run (Counter, 10, "A");
    
    printf("Second sequential\n");
    pa_run_as (Counter, Counter_1, 5, "B");
    
    printf("Concurrent\n");
    pa_co(2) {
        pa_with_weak (Counter, 10, "A");
        pa_with_as (Counter, Counter_1, 5, "B");
    } pa_co_end;
    
    printf("Concurrent weak only\n");
    pa_co(2) {
        pa_with_weak (Counter, 10, "A");
        pa_with_weak_as (Counter, Counter_1, 5, "B");
    } pa_co_end;

    printf("Redo first sequential\n");
    pa_run (Counter, 10, "A");
    
    printf("Redo second sequential\n");
    pa_run_as (Counter, Counter_1, 5, "B");
    
    printf("Preempt\n");
    pa_co(5) {
        pa_with_weak (Generator, &pa_self.i);
        pa_with_weak (Printer, "pre", int_to_str(pa_self.i));
        pa_with (SubActivity, pa_self.i);
        pa_with_weak (ResetActivity);
        pa_with_weak (SuspendActivity, pa_self.i);
    } pa_co_end;
    
    printf("Done\n");
} pa_activity_end;
   
/* Driver */

pa_use(Main);

int main(int argc, const char * argv[]) {
    
    /* Tick until done. */
    while (pa_tick(Main) == PA_RC_WAIT) {
    }
    
    return 0;
}
