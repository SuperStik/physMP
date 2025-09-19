#ifndef OBJC_MACROS_H
#define OBJC_MACROS_H 1

#ifdef __OBJC__
# define ARP_PUSH() @autoreleasepool {
# define ARP_POP() }
#else
# define ARP_PUSH()
# define ARP_POP()
#endif

#endif /* OBJC_MACROS_H */
