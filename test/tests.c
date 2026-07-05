#include "unity.h"
#define OSI_IMPLEMENT_XSI
#include "osi.h"

void setUp(void) {}
void tearDown(void) {}

#define CHECK(x)                                                               \
  {                                                                            \
    t_osi_result result = x;                                                   \
    if (result.kind != OK) {                                                   \
      TEST_FAIL_MESSAGE(result.msg);                                           \
    }                                                                          \
  }

void test_xsi(void) {
  t_osi_vecval zero = {.aval = 0, .bval = 0};

  t_osi_vecval one = {.aval = 1, .bval = 0};

  t_xsi_simulator simulator;
  CHECK(osi_create_xsi_simulator("libxv_simulator_kernel.so", &simulator));

  t_simulation simulation;
  CHECK(osi_start_xsi_simulation(&simulator, "xsimk.so", &simulation, NULL, NULL));

  t_osi_port clk;
  CHECK(simulation.get_port_by_name(simulation.simulation_data, "clk", &clk));

  t_osi_port clk_out;
  t_osi_vecval clk_out_value;
  CHECK(simulation.get_port_by_name(simulation.simulation_data, "clk_out",
                                    &clk_out));

  for (int i = 0; i < 10; i++) {
    printf("i: %d\n", i);
    CHECK(simulation.put_value(simulation.simulation_data, clk, &zero));
    CHECK(simulation.get_value(simulation.simulation_data, clk_out,
                               &clk_out_value));
    printf("clk_out before: %d\n", clk_out_value.aval & 1);
    CHECK(simulation.evaluate(simulation.simulation_data));
    CHECK(simulation.get_value(simulation.simulation_data, clk_out,
                               &clk_out_value));
    printf("clk_out after-ish: %d\n", clk_out_value.aval & 1);
    CHECK(simulation.advance(simulation.simulation_data, 5));
    CHECK(simulation.get_value(simulation.simulation_data, clk_out,
                               &clk_out_value));
    printf("clk_out after: %d\n", clk_out_value.aval & 1);
    CHECK(simulation.put_value(simulation.simulation_data, clk, &one));
    CHECK(simulation.get_value(simulation.simulation_data, clk_out,
                               &clk_out_value));
    printf("clk_out before: %d\n", clk_out_value.aval & 1);
    CHECK(simulation.advance(simulation.simulation_data, 5));
    CHECK(simulation.get_value(simulation.simulation_data, clk_out,
                               &clk_out_value));
    printf("clk_out after: %d\n", clk_out_value.aval & 1);
  }
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_xsi);
  return UNITY_END();
}