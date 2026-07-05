`timescale 1 ns/1 ps

module simple (input clk, output clk_out);
assign clk_out = clk;
initial begin
  $dumpvars;
  $dumpfile("out.vcd");
end
always @ (posedge clk) begin
  $display("Hello");
  //$finish;
end
endmodule
