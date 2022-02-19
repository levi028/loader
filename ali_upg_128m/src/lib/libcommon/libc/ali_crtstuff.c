/* In order to use C++ in ALi TDS, we MUST invoke all constructors for C++
 * file-scope static-storage objects. We define tds_do_global_ctors() to do
 * this, and it MUST be called before runing any C++ code.
 *
 * tds_do_global_dtors() define the routine to call all the global destructors,
 * but it's not needed in ALi TDS.
 */

/* Declare a pointer to void function type.  */
typedef void (*func_ptr) (void);

extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[]; 

/* Constructors are called in reverse order of the list. */
void tds_do_global_ctors(void)
{
	unsigned long nptrs = (unsigned long)__CTOR_LIST__[0];
	unsigned int i;
	
	if (nptrs == (unsigned long)-1)
	{
		for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++) ;
	}
    
	for (i = nptrs; i >= 1; i--)
	{
		__CTOR_LIST__[i] ();
	}
}



/* Destructors are called in forward order of the list. */
void tds_do_global_dtors(void)
{
	unsigned long nptrs = (unsigned long)__DTOR_LIST__[0];
	unsigned int i;
	
	if (nptrs == (unsigned long)-1)
	{
		for (nptrs = 0; __DTOR_LIST__[nptrs + 1] != 0; nptrs++) ;
	}
    
	for (i = 1; i <= nptrs; i++)
	{
		__DTOR_LIST__[i] ();
	}
}

