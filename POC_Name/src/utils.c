#include "module1.h"
/* Shared error-code messages used by the CLI and audit log. */
const char *tc_error(int code){switch(code){case TC_OK:return "Success";case TC_INVALID:return "Invalid input";case TC_DUPLICATE:return "Subscriber ID already exists";case TC_MISSING:return "Subscriber not found";case TC_NOMEM:return "Memory allocation failed";case TC_IO:return "File or internal error";case TC_DENIED:return "Permission denied";default:return "Unknown error";}}

