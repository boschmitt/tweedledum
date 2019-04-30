/*
** This header must not contain header guards (like <assert.h> must not).
** Each time it is invoked, it redefines the macros EXTERN, INITIALIZE
** based on whether macro DEFINE_VARIABLES is currently defined.
*/
#undef EXTERN
#undef INITIALIZE

#ifdef DEFINE_VARIABLES
#define EXTERN              /* nothing */
#define INITIALIZE(...)     = __VA_ARGS__
#else
#define EXTERN              extern
#define INITIALIZE(...)     /* nothing */
#endif /* DEFINE_VARIABLES */