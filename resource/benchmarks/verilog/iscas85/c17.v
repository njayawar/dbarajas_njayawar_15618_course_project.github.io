module c17(s22gat,s23gat,s1gat,s2gat,s3gat,s6gat,s7gat);

  output s22gat;
  output s23gat;
  input s1gat;
  input s2gat;
  input s3gat;
  input s6gat;
  input s7gat;

  nand x0(s10gat,s1gat,s3gat);
  nand x1(s11gat,s3gat,s6gat);
  nand x2(s16gat,s2gat,s11gat);
  nand x3(s19gat,s11gat,s7gat);
  nand x4(s22gat,s10gat,s16gat);
  nand x5(s23gat,s16gat,s19gat);

endmodule
