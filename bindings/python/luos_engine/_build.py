import sys
from pathlib import Path
from cffi import FFI

# Regenerate _enum_values.py from C headers on every build.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from tools.gen_enums import main as _gen_enums
_gen_enums()
sys.path.pop(0)

ffibuilder = FFI()

ffibuilder.cdef("""
float cmult(int int_param, float float_param);

#define MAX_DATA_MSG_SIZE 128
#define MAX_ALIAS_SIZE 16

typedef enum {
    SUCCEED = 0,
    PROHIBITED = 1,
    FAILED = 0xFF
} error_return_t;

typedef struct { ...; } header_t;
typedef struct { ...; } msg_t;

// revision_t: matches the named-field half of the engine's packed
// union (struct_luos.h). Size is 3; the sibling unmap[3] is not
// exposed because Python doesn't need the raw-byte view.
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t build;
} revision_t;

// service_t is opaque; we only pass pointers.
typedef struct service_t service_t;

typedef void (*SERVICE_CB)(service_t *, const msg_t *);

void Luos_Init(void);
void Luos_Loop(void);
service_t *Luos_CreateService(SERVICE_CB cb, uint8_t type,
                              const char *alias, revision_t revision);
void Luos_ServicesClear(void);
error_return_t Luos_SendMsg(service_t *service, msg_t *msg);
uint32_t Luos_GetSystick(void);
bool Luos_IsDetected(void);
void Luos_Detect(service_t *service);
uint16_t Luos_NbrAvailableMsg(void);

// Accessors for service_t's opaque fields (defined in set_source).
uint16_t service_id(service_t *s);
uint16_t service_type(service_t *s);

// Peek helpers from Task 5 — keep these.
uint16_t peek_target(const void *header_bytes);
uint16_t peek_source(const void *header_bytes);
uint8_t  peek_cmd(const void *header_bytes);
uint16_t peek_size(const void *header_bytes);
uint8_t  peek_target_mode(const void *header_bytes);
uint8_t  peek_config(const void *header_bytes);

typedef enum {
    SERVICEID,
    SERVICEIDACK,
    TYPE,
    BROADCAST,
    TOPIC,
    NODEID,
    NODEIDACK
} target_mode_t;

typedef enum { CLEAR, SERVICE, NODE } entry_mode_t;

typedef int luos_type_t;

typedef struct { ...; } routing_table_t;

typedef struct {
    uint16_t result_nbr;
    routing_table_t *result_table[...];
} search_result_t;

search_result_t *RTFilter_Reset(search_result_t *result);
search_result_t *RTFilter_Type(search_result_t *result, luos_type_t type);
search_result_t *RTFilter_Alias(search_result_t *result, char *alias);

// Helpers to read opaque routing_table_t entries.
uint8_t  rtb_mode(const routing_table_t *e);
uint16_t rtb_service_id(const routing_table_t *e);
uint16_t rtb_service_type(const routing_table_t *e);
void     rtb_service_alias(const routing_table_t *e, char *out16);
""")

# Discover repo root and engine include dirs at build time.
_BUILD_DIR = Path(__file__).resolve().parent.parent  # bindings/python/
_REPO_ROOT = _BUILD_DIR.parent.parent

_INCLUDE_DIRS = [
    str(_REPO_ROOT / "engine" / "core" / "inc"),
    str(_REPO_ROOT / "engine"),
    str(_REPO_ROOT / "engine" / "OD"),
    str(_REPO_ROOT / "engine" / "IO" / "inc"),
]

_extra_link_args = []
if sys.platform == "darwin":
    _extra_link_args = ["-undefined", "dynamic_lookup"]
elif sys.platform.startswith("linux"):
    _extra_link_args = ["-Wl,--unresolved-symbols=ignore-in-object-files"]

ffibuilder.set_source(
    "luos_engine._luos_cffi",
    """
    #include <string.h>
    #include "luos_engine.h"

    // Peek helpers: take 7 raw bytes, interpret them as the real
    // engine's header_t, and return individual fields. Used by
    // test_header_layout to verify Python-side packing matches the
    // compiler's bitfield layout.
    uint16_t peek_target(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.target;
    }
    uint16_t peek_source(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.source;
    }
    uint8_t peek_cmd(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.cmd;
    }
    uint16_t peek_size(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.size;
    }
    uint8_t peek_target_mode(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.target_mode;
    }
    uint8_t peek_config(const void *header_bytes) {
        header_t h; memcpy(&h, header_bytes, sizeof(h)); return h.config;
    }
    uint16_t service_id(service_t *s) { return s->id; }
    uint16_t service_type(service_t *s) { return s->type; }
    uint8_t rtb_mode(const routing_table_t *e) { return e->mode; }
    uint16_t rtb_service_id(const routing_table_t *e) { return e->id; }
    uint16_t rtb_service_type(const routing_table_t *e) { return e->type; }
    void rtb_service_alias(const routing_table_t *e, char *out16) {
        memcpy(out16, e->alias, MAX_ALIAS_SIZE);
    }
    """,
    include_dirs=_INCLUDE_DIRS,
    extra_link_args=_extra_link_args,
)

if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
