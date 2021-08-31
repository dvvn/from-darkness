#pragma once

#define CHEAT_NETVARS_UPDATING

#if defined(_DEBUG) || defined(CHEAT_NETVARS_UPDATING)
#define CHEAT_NETVARS_RESOLVE_TYPE
#endif
#if 0
#define CHEAT_NETVARS_DUMP_STATIC_OFFSET
#endif
#if defined(CHEAT_GUI_TEST) || (defined(CHEAT_NETVARS_DUMP_STATIC_OFFSET) && !defined(_DEBUG))
#define CHEAT_NETVARS_DUMPER_DISABLED
#endif
