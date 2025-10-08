#ifndef CPPHEADER_H
#define CPPHEADER_H 1

#ifdef __cplusplus
# define C_BEGIN extern "C" {
# define C_END }
#else
# define C_BEGIN
# define C_END
#endif

#endif /* CPPHEADER_H */
