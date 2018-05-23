#include <base/std_def.h>
C_EXPORT mod_name_decoration_t	 mod_name_deco_type = MOD_NAME_DECO;
C_EXPORT int _fltused = 1;
unsigned int C_API_FUNC _DllMainCRTStartup(unsigned int *prev, unsigned int *cur, unsigned int *xx)
{

	return 1;
}