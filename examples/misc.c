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
    assert(self->i == 0);
    
    self->i = n;
    while (self->i-- > 0) {
        pa_await (true);
        printf("%s %d\n", prefix, self->i);
    }
} pa_end;

pa_activity (Generator, pa_ctx(uint16_t i), uint16_t* valp) {
    assert(self->i == 0);
    
    self->i = 0;
    while (true) {
        *valp = self->i++;
        pa_await (true);
    }
} pa_end;

pa_activity (Printer, pa_ctx(), const char* str) {
    while (true) {
        printf("%s\n", str);
        pa_await (true);
    }
} pa_end;

pa_activity (SubActivity, pa_ctx(pa_use(Printer); pa_use(Counter)), uint16_t val) {
    pa_when_abort (val >= 10, Printer, ".");
    printf("#\n");
    
    pa_when_abort (val > 100, Counter, 10, "X");
    printf("!\n");
} pa_end;

pa_activity (Main, pa_ctx(pa_codef(4); uint16_t i;
                          pa_use(Counter); pa_use_as(Counter, Counter_1);
                          pa_use(Generator); pa_use(Printer); pa_use(SubActivity)))
{
    assert(self->i == 0);
    
    printf("First sequential\n");
    pa_run (Counter, 10, "A");
    
    printf("Second sequential\n");
    pa_run_as (Counter, Counter_1, 5, "B");
    
    printf("Concurrent\n");
    pa_cobegin(2) {
        pa_with_weak (Counter, 10, "A");
        pa_with_as (Counter, Counter_1, 5, "B");
    } pa_coend;
    
    printf("Concurrent weak only\n");
    pa_cobegin(2) {
        pa_with_weak (Counter, 10, "A");
        pa_with_weak_as (Counter, Counter_1, 5, "B");
    } pa_coend;

    printf("Redo first sequential\n");
    pa_run (Counter, 10, "A");
    
    printf("Redo second sequential\n");
    pa_run_as (Counter, Counter_1, 5, "B");
    
    printf("Preempt\n");
    pa_cobegin(3) {
        pa_with_weak (Generator, &self->i);
        pa_with_weak (Printer, int_to_str(self->i));
        pa_with (SubActivity, self->i);
    } pa_coend;
    
    printf("Done\n");
} pa_end;
   
/* Driver */

pa_use(Main);

int main(int argc, const char * argv[]) {
    
    /* Tick until done. */
    while (pa_tick(Main) == PA_RC_WAIT) {
    }
    
    return 0;
}
