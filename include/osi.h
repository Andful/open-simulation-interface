#include <assert.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef OSI_IMPLEMENT_XSI
#include <xsi.h>
#endif

/**
 * Error Kinds:
 * * OK: everything went ok
 * * NOT_IMPLEMENTED_ERROR: implementation is missing for some reason
 * * OSI_ERROR: OSI specific error.
 * * SIM_ERROR: Simulation specific error.
 * * DL_ERRPR: Error due to dynamic linking.
 */
typedef enum {
  OK,
  NOT_IMPLEMENTED_ERROR,
  OSI_ERROR,
  SIM_ERROR,
  DL_ERROR,
} t_osi_error_kind;

typedef enum {
  INPUT,
  OUTPUT,
  INOUT
} t_osi_port_direction;

typedef struct {
  t_osi_error_kind kind;
  const char* msg;
  const void* arg;
} t_osi_result;

typedef struct {
  t_osi_result (*callback)(void*);
  void* data;
} t_osi_callback;

typedef struct {
  uint32_t aval;
  uint32_t bval;
} t_osi_vecval;

typedef void* t_osi_simulation_data;
typedef void* t_osi_port; // The pointer may be treated as an integer, so NULL
                          // may not mean what it means

typedef t_osi_result (*t_fp_osi_get_number_top_ports)(t_osi_simulation_data,
                                                      uint32_t*);
typedef t_osi_result (*t_fp_osi_get_time_precision)(t_osi_simulation_data,
                                                    uint32_t*);
typedef t_osi_result (*t_fp_osi_get_port_direction)(t_osi_simulation_data,
                                                    t_osi_port,
                                                    t_osi_port_direction*);
typedef t_osi_result (*t_fp_osi_get_port_width)(t_osi_simulation_data,
                                                const t_osi_port, uint32_t*);
typedef t_osi_result (*t_fp_osi_get_port_by_name)(t_osi_simulation_data,
                                                  const char*, t_osi_port*);
typedef t_osi_result (*t_fp_osi_get_port_by_index)(t_osi_simulation_data,
                                                   uint32_t, t_osi_port*);
typedef t_osi_result (*t_fp_osi_get_port_name)(t_osi_simulation_data,
                                               const t_osi_port, const char**);
typedef t_osi_result (*t_fp_osi_get_port_index)(t_osi_simulation_data,
                                                const t_osi_port, uint32_t*);
typedef t_osi_result (*t_fp_osi_put_value)(t_osi_simulation_data, t_osi_port,
                                           const t_osi_vecval*);
typedef t_osi_result (*t_fp_osi_get_value)(t_osi_simulation_data,
                                           const t_osi_port, t_osi_vecval*);
typedef t_osi_result (*t_fp_osi_advance)(t_osi_simulation_data, uint64_t);
typedef t_osi_result (*t_fp_osi_evaluate)(t_osi_simulation_data);
typedef t_osi_result (*t_fp_osi_get_time)(t_osi_simulation_data, uint64_t*);
typedef t_osi_result (*t_fp_osi_close)(t_osi_simulation_data);

typedef struct {
  t_fp_osi_get_number_top_ports get_number_top_ports;
  t_fp_osi_get_time_precision get_time_precision;
  t_fp_osi_get_port_direction get_port_direction;
  t_fp_osi_get_port_width get_port_width;
  t_fp_osi_get_port_by_name get_port_by_name;
  t_fp_osi_get_port_by_index get_port_by_index;
  t_fp_osi_get_port_name get_port_name;
  t_fp_osi_get_port_index get_port_index;
  t_fp_osi_put_value put_value;
  t_fp_osi_get_value get_value;
  t_fp_osi_advance advance;
  t_fp_osi_evaluate evaluate;
  t_fp_osi_get_time get_time;
  t_fp_osi_close close;
  t_osi_simulation_data simulation_data;
} t_osi_simulation;

#ifndef OSI_IMPLEMENT_XSI
typedef void t_osi_xsi_simulator;
#else
typedef struct {
  t_fp_xsi_get_port_number xsi_get_port_number;
  t_fp_xsi_get_port_name xsi_get_port_name;
  t_fp_xsi_put_value xsi_put_value;
  t_fp_xsi_get_value xsi_get_value;
  t_fp_xsi_run xsi_run;
  t_fp_xsi_restart xsi_restart;
  t_fp_xsi_get_int xsi_get_int;
  t_fp_xsi_get_int_port xsi_get_int_port;
  t_fp_xsi_get_str_port xsi_get_str_port;
  t_fp_xsi_get_status xsi_get_status;
  t_fp_xsi_get_error_info xsi_get_error_info;
  t_fp_xsi_close xsi_close;
  t_fp_xsi_trace_all xsi_trace_all;
  t_fp_xsi_get_time xsi_get_time;

  void* simkernel_library;
} t_osi_xsi_simulator;

typedef struct {
  t_osi_xsi_simulator* simulator;
  xsiHandle handle;
} t_osi_xsi_simulation_data;

t_osi_result
osi_xsi_impl_get_number_top_ports(t_osi_simulation_data simulation_data,
                                  uint32_t* num_ports) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;
  int32_t num =
      simulator->xsi_get_int(xsi_simulation_data->handle, xsiNumTopPorts);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  *num_ports = (uint32_t)num;

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_get_port_index(t_osi_simulation_data simulation_data,
                                         t_osi_port port, uint32_t* index) {
  uint32_t num_ports = 0;
  t_osi_result result =
      osi_xsi_impl_get_number_top_ports(simulation_data, &num_ports);

  if (result.kind != OK) {
    return result;
  }

  *index = (uint32_t)(uintptr_t)port;

  if (*index >= num_ports) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = "no such port",
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result
osi_xsi_impl_get_time_precision(t_osi_simulation_data simulation_data,
                                uint32_t* num_ports) {
  (void)(simulation_data); // Unused
  (void)(num_ports);       // Unused
  return (t_osi_result){
      .kind = NOT_IMPLEMENTED_ERROR,
      .msg = "not implemented",
      .arg = NULL,
  };
}

t_osi_result
osi_xsi_impl_get_port_direction(t_osi_simulation_data simulation_data,
                                t_osi_port port,
                                t_osi_port_direction* port_kind) {
  uint32_t index = 0;
  t_osi_result result =
      osi_xsi_impl_get_port_index(simulation_data, port, &index);

  if (result.kind != OK) {
    return result;
  }

  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  uint32_t direction =
      simulator->xsi_get_int_port(handle, index, xsiDirectionTopPort);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  if (direction == xsiInputPort) {
    *port_kind = INPUT;
  } else if (direction == xsiOutputPort) {
    *port_kind = OUTPUT;
  } else {
    *port_kind = INOUT;
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_get_port_width(t_osi_simulation_data simulation_data,
                                         t_osi_port port,
                                         uint32_t* port_width) {
  uint32_t index = 0;
  t_osi_result result =
      osi_xsi_impl_get_port_index(simulation_data, port, &index);

  if (result.kind != OK) {
    return result;
  }

  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  *port_width = simulator->xsi_get_int_port(handle, index, xsiHDLValueSize);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result
osi_xsi_impl_get_port_by_name(t_osi_simulation_data simulation_data,
                              const char* port_name, t_osi_port* port) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  int32_t index = simulator->xsi_get_port_number(handle, port_name);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  *port = (t_osi_port)(uintptr_t)index;

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result
osi_xsi_impl_get_port_by_index(t_osi_simulation_data simulation_data,
                               uint32_t index, t_osi_port* port) {
  uint32_t num_ports = 0;
  t_osi_result result =
      osi_xsi_impl_get_number_top_ports(simulation_data, &num_ports);

  if (result.kind != OK) {
    return result;
  }

  if (index >= num_ports) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = "no such port",
        .arg = NULL,
    };
  }

  *port = (t_osi_port)(uintptr_t)index;

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_get_port_name(t_osi_simulation_data simulation_data,
                                        t_osi_port port, const char** name) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  uint32_t index = 0;
  t_osi_result result =
      osi_xsi_impl_get_port_index(simulation_data, port, &index);

  if (result.kind != OK) {
    return result;
  }

  *name = simulator->xsi_get_port_name(handle, index);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  if (!*name) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = "port name is null",
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_put_value(t_osi_simulation_data simulation_data,
                                    t_osi_port port,
                                    const t_osi_vecval* value) {
  uint32_t index;
  t_osi_result result =
      osi_xsi_impl_get_port_index(simulation_data, port, &index);
  if (result.kind != OK) {
    return result;
  }

  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  // I think it is fine to discard the const qualifier
  simulator->xsi_put_value(handle, (int32_t)index, (t_osi_vecval*)value);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_get_value(t_osi_simulation_data simulation_data,
                                    const t_osi_port port,
                                    t_osi_vecval* value) {
  uint32_t index;
  t_osi_result result =
      osi_xsi_impl_get_port_index(simulation_data, port, &index);
  if (result.kind != OK) {
    return result;
  }

  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  simulator->xsi_get_value(handle, (int32_t)index, value);

  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_advance(t_osi_simulation_data simulation_data,
                                  uint64_t delta_time) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;

  simulator->xsi_run(handle, (int64_t)delta_time);
  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_evaluate(t_osi_simulation_data simulation_data) {
  return osi_xsi_impl_advance(simulation_data, 0);
}

t_osi_result osi_xsi_impl_close(t_osi_simulation_data simulation_data) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;

  xsi_simulation_data->simulator->xsi_close(xsi_simulation_data->handle);
  free(xsi_simulation_data);

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}

t_osi_result osi_xsi_impl_get_time(t_osi_simulation_data simulation_data,
                                   uint64_t* time) {
  t_osi_xsi_simulation_data* xsi_simulation_data =
      (t_osi_xsi_simulation_data*)simulation_data;
  t_osi_xsi_simulator* simulator = xsi_simulation_data->simulator;
  xsiHandle handle = xsi_simulation_data->handle;
  *time = xsi_simulation_data->simulator->xsi_get_time(handle);
  if (simulator->xsi_get_status(handle) != xsiNormal) {
    return (t_osi_result){
        .kind = SIM_ERROR,
        .msg = simulator->xsi_get_error_info(handle),
        .arg = NULL,
    };
  }
  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
}
#endif

bool osi_xsi_implemented(void) {
#ifdef OSI_IMPLEMENT_XSI
  return true;
#else
  return false;
#endif
}

t_osi_result osi_create_xsi_simulator(const char* simkernel_libname,
                                      t_osi_xsi_simulator* simulator) {
  assert(simkernel_libname);
  assert(simulator);
#ifndef OSI_IMPLEMENT_XSI
  return (t_osi_result){
      .kind = NOT_IMPLEMENTED_ERROR,
      .msg = "Not implemented because OSI_IMPLEMENT_XSI is not defined",
      .arg = NULL,
  };
#else
  simulator->simkernel_library = dlopen(simkernel_libname, RTLD_LAZY | RTLD_GLOBAL);
  char* err = dlerror();
  if (err) {
    return (t_osi_result){.kind = DL_ERROR, .msg = err, .arg = NULL};
  }

#define OSI_LOAD_XSI_FUNC(x)                                                   \
  *(void**)(&simulator->x) = dlsym(simulator->simkernel_library, #x);                     \
  err = dlerror();                                                             \
  if (err) {                                                                   \
    return (t_osi_result){.kind = DL_ERROR, .msg = err, .arg = NULL};          \
  }

  OSI_LOAD_XSI_FUNC(xsi_get_port_number);
  OSI_LOAD_XSI_FUNC(xsi_get_port_name);
  OSI_LOAD_XSI_FUNC(xsi_put_value);
  OSI_LOAD_XSI_FUNC(xsi_get_value);
  OSI_LOAD_XSI_FUNC(xsi_run);
  OSI_LOAD_XSI_FUNC(xsi_restart);
  OSI_LOAD_XSI_FUNC(xsi_get_int);
  OSI_LOAD_XSI_FUNC(xsi_get_int_port);
  OSI_LOAD_XSI_FUNC(xsi_get_str_port);
  OSI_LOAD_XSI_FUNC(xsi_get_status);
  OSI_LOAD_XSI_FUNC(xsi_get_error_info);
  OSI_LOAD_XSI_FUNC(xsi_close);
  OSI_LOAD_XSI_FUNC(xsi_trace_all);
  OSI_LOAD_XSI_FUNC(xsi_get_time);

#undef OSI_LOAD_XSI_FUNC

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
#endif
}

t_osi_result osi_destroy_xsi_simulator(t_osi_xsi_simulator* simulator) {
#ifndef OSI_IMPLEMENT_XSI
  (void)simulator;
  return (t_osi_result){
      .kind = NOT_IMPLEMENTED_ERROR,
      .msg = "Not implemented because OSI_IMPLEMENT_XSI is not defined",
      .arg = NULL,
  };
#else
  if (dlclose(simulator->simkernel_library)) {
    char* msg = dlerror();
    return (t_osi_result){
        .kind = DL_ERROR,
        .msg = msg,
        .arg = NULL,
    };
  }

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
#endif
}

t_osi_result osi_start_xsi_simulation(t_osi_xsi_simulator* simulator,
                                      const char* design_libname,
                                      t_osi_simulation* simulation,
                                      const char* log_file,
                                      const char* wdb_file) {
  assert(simulator);
  assert(design_libname);
  assert(simulation);
#ifndef OSI_IMPLEMENT_XSI
  (void)log_file;
  (void)wdb_file;
  return (t_osi_result){
      .kind = NOT_IMPLEMENTED_ERROR,
      .msg = "Not implemented because OSI_IMPLEMENT_XSI is not defined",
      .arg = NULL,
  };
#else
  dlerror(); // Clear errors

  void* design_dl = dlopen(design_libname, RTLD_LAZY | RTLD_GLOBAL);

  char* err = dlerror();
  if (err) {
    return (t_osi_result){
        .kind = DL_ERROR,
        .msg = err,
        .arg = NULL,
    };
  }

  t_fp_xsi_open xsi_open;
  *(void**)(&xsi_open) = dlsym(design_dl, "xsi_open");
  err = dlerror();
  if (err) {
    return (t_osi_result){
        .kind = DL_ERROR,
        .msg = err,
        .arg = NULL,
    };
  }

  s_xsi_setup_info info;
  // memset is apparently insecure
  for (size_t i = 0; i < sizeof(info); i++) {
    ((uint8_t*)&info)[i] = 0;
  }
  info.logFileName = (char*)log_file; // I think conversion to char* is fine
  info.wdbFileName = (char*)wdb_file; // I think conversion to char* is fine
  xsiHandle handle = xsi_open(&info);

  t_osi_xsi_simulation_data* xsi_simulation_data =
      malloc(sizeof(t_osi_xsi_simulation_data));
  xsi_simulation_data->simulator = simulator;
  xsi_simulation_data->handle = handle;

  simulation->get_number_top_ports = osi_xsi_impl_get_number_top_ports;
  simulation->get_time_precision = osi_xsi_impl_get_time_precision;
  simulation->get_port_direction = osi_xsi_impl_get_port_direction;
  simulation->get_port_width = osi_xsi_impl_get_port_width;
  simulation->get_port_by_name = osi_xsi_impl_get_port_by_name;
  simulation->get_port_by_index = osi_xsi_impl_get_port_by_index;
  simulation->get_port_name = osi_xsi_impl_get_port_name;
  simulation->get_port_index = osi_xsi_impl_get_port_index;
  simulation->put_value = osi_xsi_impl_put_value;
  simulation->get_value = osi_xsi_impl_get_value;
  simulation->advance = osi_xsi_impl_advance;
  simulation->evaluate = osi_xsi_impl_evaluate;
  simulation->get_time = osi_xsi_impl_get_time;
  simulation->close = osi_xsi_impl_close;
  simulation->simulation_data = (void*)xsi_simulation_data;

  return (t_osi_result){
      .kind = OK,
      .msg = NULL,
      .arg = NULL,
  };
#endif
}
