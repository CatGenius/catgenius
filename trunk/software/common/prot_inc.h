/* Function and variable sharing macros */

#if defined(PUBLIC_FN) || defined(PUBLIC_VAR) || defined(PRIVATE_FN) || defined(PRIVATE_VAR)
#	undef PUBLIC_FN
#   undef PUBLIC_VAR
#	undef PRIVATE_FN
#   undef PRIVATE_VAR
#endif

#define PUBLIC_FN(x)		extern x
#define PUBLIC_VAR(x, y)	extern x
#define PRIVATE_FN(x)
#define PRIVATE_VAR(x, y)