#ifndef __QUEUE_T_H__
#define __QUEUE_T_H__
// engler, cs140e: abuse the C preprocessor to 
// generate a brain-dead generic queue
// that supports LIFO (push), FIFO (append), 
// iteration and element count.     
//
// features:
//   - written so you have have more than one queue
//     type in the same lexical scope.
//   - the queue <next> pointer is in the client-defined 
//     structure.
//   - client does all allocation and deallocation.
//   - should work both in the kernel and on unix.
//
// see <test-queue.c> for example usage.
//
// TODO: 
//   - make an apply. 
//   - add some more tests to show how works.

// a slight sugaring of the full generator.
//  - "<pfx>q_t" used as the the queue type.
#define gen_queue(pfx, E_T, next_name)          \
    gen_queue(pfx, pfx ## q_t, E_T, next_name)

// the full queue generator.  
//
// client supplies:
//  - "<pfx>_": this prefix gets prepended to each method name.
//  - <Q_T>: the type name to use for the generated list.
//  - <E_T>: the element type name the list tracks. 
// - <next_name> is the next pointer name in <E_T>.
//
// generates the following methods:
//   - <pfx>_mk(): initialize the queue.
//   - <pfx>_append(Q_T *q,e): FIFO append <e> to <q>
//   - <pfx>_push(Q_T *q,e):  LIFO append <e> to <q>
//   - <pfx>_pop(Q_T *q):  pops element from front of <q>.
//   - <pfx>_first(q): returns first element or null if none.
//   - <pfx>_next(e): returns next element after <e> or null if none.
//   - other helpers.

#define gen_queue_decl(pfx, Q_T, E_T, next_name)    \
    typedef struct {                                \
        E_T *head, *tail;                           \
        unsigned cnt;                               \
    } Q_T;


#define gen_queue_fn(pfx, Q_T, E_T, next_name)                  \
    /* initialize Q_T queue. */                                 \
    static inline Q_T pfx ## _mk(void) {                        \
        return (Q_T){0};                                        \
    }                                                           \
                                                                \
    /* used for iteration. */                                   \
    static E_T *pfx ## _start(Q_T *q) { return q->head; }       \
    static E_T *pfx ## _first(Q_T *q) { return q->head; }       \
    static E_T *pfx ## _last(Q_T *q)  { return q->tail; }       \
    static E_T *pfx ## _next(E_T *e)  { return e->next_name; }  \
    static unsigned pfx ## _nelem(Q_T *q) { return q->cnt; }    \
                                                        \
    static int pfx ## _empty(Q_T *q)  {                 \
        if(q->head)                                     \
            return 0;                                   \
        assert(pfx ## _nelem(q) == 0);                  \
        demand(!q->tail, invalid Q);                    \
        return 1;                                       \
    }                                                   \
                                                        \
    /* remove from front of list. */                    \
    static E_T *pfx ## _pop(Q_T *q) {                   \
        demand(q, bad input);                           \
        E_T *e = q->head;                               \
        if(!e) {                                        \
            assert(pfx ## _empty(q));                   \
            return 0;                                   \
        }                                               \
        q->cnt--;                                       \
        q->head = e->next_name;                         \
        if(!q->head)                                    \
            q->tail = 0;                                \
        return e;                                       \
    }                                                   \
                                                        \
    /* insert at tail (FIFO) */                    \
    static void pfx ## _append(Q_T *q, E_T *e) {        \
        e->next_name = 0;                                   \
        q->cnt++;                                           \
        if(!q->tail)                                        \
            q->head = q->tail = e;                          \
        else {                                              \
            q->tail->next_name = e;                         \
            q->tail = e;                                    \
        }                                                   \
    }                                                       \
                                                            \
    /* insert at head (LIFO) */                             \
    static void pfx ## _push(Q_T *q, E_T *e) {              \
        q->cnt++;                                           \
        e->next_name = q->head;                             \
        q->head = e;                                        \
        if(!q->tail)                                        \
            q->tail = e;                                    \
    }                                                       \
                                                            \
    /* insert <e_new> after <e>: <e>=<null> means put at head. */   \
    static void pfx ## _insert_after(Q_T *q, E_T *e, E_T *e_new) {  \
        if(!e)                                                      \
            pfx ## _push(q,e_new);                                  \
        else if(q->tail == e)                                       \
            pfx ## _append(q,e_new);                                \
        else {                                                      \
            q->cnt++;                                               \
            e_new->next_name = e->next_name;                        \
            e->next_name = e_new;                                   \
        }                                                           \
    }

#define gen_queue_full(pfx, Q_T, E_T, next_name)    \
    gen_queue_decl(pfx,Q_T,E_T,next_name)           \
    gen_queue_fn(pfx,Q_T,E_T,next_name)

#endif
