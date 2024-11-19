Here is a verilog parser and verilog-to-iscas converter script for you to use. Both are written in python and should be fairly well documented.

1. convert_verilog_multimodule_to_iscas.py - This is a script I developed for my research work. It converts a verilog module (which might contain sub-modules) into a flattened ISCAS .bench file. For example, if we start with this verilog file:

module myCircuit(a, b, c);
    output c;
    input [3:0] a, b;
    wire and_a, or_b;

    and4 g1(and_a, a[0], a[1], a[2], a[3]);
    or4 g2(or_b, b[0], b[1], b[2], b[3]);
    xor2 g3(c, and_a, or_b);
endmodule

We run this command to convert it to iscas:

./convert_verilog_multimodule_to_iscas.py test.v test.bench myCircuit

This is the ISCAS output stored in test.bench:
# myCircuit
# 2 inputs
# 1 outputs
INPUT(a[3])
INPUT(a[2])
INPUT(a[1])
INPUT(a[0])
INPUT(b[3])
INPUT(b[2])
INPUT(b[1])
INPUT(b[0])
OUTPUT(c)

# To properly handle 1'b0 / 1'b1, we generate these constants here
ZERO = XOR(b[0], b[0])
ONE = XNOR(b[0], b[0])
# Don't worry if you don't use them, feel free to delete these lines

and_a = AND(a[0], a[1], a[2], a[3])
or_b = OR(b[0], b[1], b[2], b[3])
c = XOR(and_a, or_b)

Let me know if you have any questions or especially if you run into a verilog file that causes the parser (used by this conversion script) crashes!




2. VerilogParser.py - This is a Python module that does a decent job of parsing all the odd syntaxes of the verilog language. You use it like this:

    import VerilogParser
    modules = VerilogParser.parse("my_file.v")
    # modify modules as you like
    VerilogParser.write_verilog(modules, "my_output_file.v")

The modules variable returned from the parse() function is a list of modules, with one item for each module found in the original verilog file. You can iterate over all the modules found like this:

for module in modules:
        # do whatever with the module variable

Each module object is a dictionary (associative array / map / hash table) with the following keys. Each key points to a list of corresponding items. Keys and their corresponding items are shown below:

module['name']
        Just a string telling the name of this module
        For example: "TestController"

module['ports']
        A list of the input and output connections used in the module instantiation line.
        For example: ['sum', 'Co', 'A', 'B', "Cin']

module['inputs']
        A list of the inputs to the module. Each input is a pair of (net_name, bus_info). If it's not a bus input, bus_info is just the empty string "". For a bus input, bus_info will be something like ' [7:0]'. Yes, with the space at the start, I'm still working to improve that part of the parser.
        For example: [ ('a', ' [3:0]'), ('b', ' [3:0]') ]

module['outputs']
        A list of the outputs from the modules. Format is exactly the same as the input list.

module['wires']
        A list of the declared wires in the module. Format is exactly the same as the input and output lists.

module['assigns']
        The assigns in the module. Since these are of the format "assign lhs = rhs;" each assign is a pair of pairs, like this:
        ( ('net_a', '[3:0]'), ('some_other_net', '[3:0]') )

module['gate_pos']
        A list of gates instantiated with positional arguments:
                fa fulladder(s, co, a, b, cin); // like this one
        Each gate is a 3-tuple like this (for and2 gate above):
                ('fa', 'fulladder', {0: 's', 1: 'co', 2: 'a', 3: 'b', 4: 'cin'})
        Notice that the third part of the tuple is a dictionary that maps from integer indexes into the name of the net.

module['gate_named']
        A list of gates instantiated with named arguments:
                fa fulladder(.sum(s), .cout(co), .x(a), .y(b), .cin(cin));
        Same first two parts of the tuple, but the keys of the dictionary are now the actual internal names of the gate connections:
                ('fa', 'fulladder', {'sum': 's', 'cout': 'co', 'x': 'a', 'y': 'b', 'cin': 'cin'})

There are some other functions in the VerilogParser module that might be useful to modify your modules, such as:

convert_to_named_ports() // very useful, should run this first
convert_to_scan() // might not be 100% working, beware
convert_to_no_buses() // expand all buses into individual nets, might break things

Hopefully that makes sense, let me know if you need any help with it and I'll do the best I can.

-Matthew - mbeckler at cmu dot edu
