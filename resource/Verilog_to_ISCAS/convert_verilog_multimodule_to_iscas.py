#!/usr/bin/env python
# Short script to demo the VerilogParser module
# by converting a multi-module design from verilog to ISCAS.
#
# Version 1.02 - Last update November 29, 2011.
# Matthew Beckler - mbeckler at cmu dot edu
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

import sys, re
import VerilogParser

def get_module_by_type(type):
    mod = None
    for module in modules:
        if module['name'] == type:
            mod = module
    return mod

def bus_range(bus_range_string):
    a, b = bus_range_string.strip(" []").split(":")
    a = int(a)
    b = int(b)
    if a < b:
        return range(a, b + 1)
    else:
        return range(a, b - 1, -1)

def get_io_tuple(list_of_tuples, name):
    for tuple in list_of_tuples:
        if tuple[0] == name:
            return tuple
    return None

# intersting recursive function to dig through the modules
def handle_module(fid, type, prefix):
    mod = get_module_by_type(type)
    if not mod:
        print "Unable to find module '%s', things are messed up! Returning from handle_module()..." % type
        return

    # handle module-level assigns
    for dest, src in mod['assigns']:
        #print "dest:", dest
        #print "src:", src
        if dest[1] != "": # bus
            dest_range = bus_range(dest[1])
            #print "src[0]:", src[0]
            if "{" in src[0]:
                #print "this src is a concatenation"
                #print src[0]
                inside = re.search("\{\s*([^\}]*?)\s*\}", src[0]).group(1)
                cons = map(str.strip, inside.split(","))
                i = 0
                for con in cons:
                    if ":" in con:
                        #print "this con is a bus"
                        net, net_range = re.search("\s*([^\s]+)\s*\[\s*([^\s]+)\s*\]", con).groups()
                        src_range = bus_range(net_range)
                        for j in src_range:
                            #print "%s = BUFF(%s)" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + net + "[" + str(j) + "]")
                            fid.write("%s = BUFF(%s)\n" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + net + "[" + str(j) + "]"))
                            i += 1
                    else:
                        #print "this con is not a bus"
                        #print "%s = BUFF(%s)" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + con)
                        fid.write("%s = BUFF(%s)\n" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + con))
                        i += 1
            else:
                #print "this src is just a bus"
                src_range = bus_range(src[1])
                assert len(dest_range) == len(src_range)
                for i in range(len(dest_range)):
                    #print "%s = BUFF(%s)" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + src[0] + "[" + str(src_range[i]) + "]")
                    fid.write("%s = BUFF(%s)\n" % (prefix + dest[0] + "[" + str(dest_range[i]) + "]", prefix + src[0] + "[" + str(src_range[i]) + "]"))
        else:
            fid.write("%s = BUFF(%s)\n" % (prefix + dest[0], prefix + src[0]))

    the_list = []
    for inst in mod['gate_pos']:
        the_list.append( (inst, "pos") )
    for inst in mod['gate_named']:
        the_list.append( (inst, "named") )

    base_gates = []
    for inst, type in the_list:
        #print inst, type
        if inst[0] in module_names:
            fid.write("#--- Recursing into module '%s' named '%s' ---\n" % (inst[0], prefix + inst[1]))
            print "  Recursing into module '%s' named '%s'" % (inst[0], prefix + inst[1])

            # we need to add renaming buffers so the names don't get messed up
            recurse_mod = get_module_by_type(inst[0])
            for inside in inst[2]:
                outside = inst[2][inside]

                # if inside in [inp[0] for inp in recurse_mod['inputs']]:
                if outside in [inp[0] for inp in recurse_mod['inputs']]:
                    # dealing with an input to this module we're about to recurse into
                    # inside_input_tuple = get_io_tuple(recurse_mod['inputs'], inside)
                    inside_input_tuple = get_io_tuple(recurse_mod['inputs'], outside)
                    if inside_input_tuple[1] != "":
                        # this is a bus!
                        this_bus_range = bus_range(inside_input_tuple[1])
                        if "{" in outside: # using the stupid {concatenation syntax} in this instantiations:
                            # we have multiple outside nets (which could be buses or just single wires) to connect to our single inside bus name
                            #print "inside: '%s'" % inside
                            #print "outside: '%s'" % outside
                            cons_raw = re.search("\{\s*([^\}]*?)\s*\}", outside).group(1)
                            cons = map(str.strip, cons_raw.split(","))
                            i = 0
                            for con in cons:
                                #print con
                                if ":" in con:
                                    #print "  this con is a bus"
                                    net, con_range_raw = re.search("\s*([^\s]+)\s*\[\s*([^\s]+)\s*\]", con).groups()
                                    con_range = bus_range(con_range_raw)
                                    for j in con_range:
                                        fid.write("%s = BUFF(%s)\n" % (prefix + inst[1] + "\\" + inside + "[" + str(this_bus_range[i]) + "]", prefix + net + "[" + str(j) + "]"))
                                        i += 1
                                else:
                                    #print "  this con is not a bus"
                                    fid.write("%s = BUFF(%s)\n" % (prefix + inst[1] + "\\" + inside + "[" + str(this_bus_range[i]) + "]", prefix + con))
                                    i += 1

                        elif ":" in outside: # but, the outside name is just a range of its net, like this: fancymodule M1(.inputbus(somebus[5:0]), blah, blah)
                            out_net, out_bus_range = re.search("\s*([^\s]+)\s*\[\s*([^\s]+)\s*\]", outside).groups()
                            out_bus_range_true = bus_range(out_bus_range)
                            if len(this_bus_range) != len(out_bus_range_true):
                                print "Error! The size of the buses that are to be connected together differ! '%s' vs '%s'" % (str(this_bus_range), str(out_bus_range_true))
                                print "  inside: '%s'" % inside
                                print "  outside: '%s'" % outside
                                sys.exit(1)
                            for i in range(len(this_bus_range)):
                                fid.write("%s = BUFF(%s)\n" % (prefix + inst[1] + "\\" + inside + "[" + str(this_bus_range[i]) + "]", prefix + out_net + "[" + str(out_bus_range_true[i]) + "]"))
                        else:
                            for i in this_bus_range:
                                fid.write("%s = BUFF(%s)\n" % (prefix + inst[1] + "\\" + inside + "[" + str(i) + "]", prefix + outside + "[" + str(i) + "]"))
                    else:
                        # not a bus
                        fid.write("%s = BUFF(%s)\n" % (prefix + inst[1] + "\\" + inside, prefix + outside))
                else:
                    # dealing with an output from this module we're about to recurse into
                    inside_output_tuple = get_io_tuple(recurse_mod['outputs'], inside)
                    if inside_output_tuple[1] != "":
                        # this is a bus!
                        this_bus_range = bus_range(inside_output_tuple[1])
                        if ":" in outside: # but, the outside name is just a range of its net, like this: fancymodule M1(.outputbus(somebus[5:0]), blah, blah)
                            out_net, out_bus_range = re.search("\s*([^\s]+)\s*\[\s*([^\s]+)\s*\]", outside).groups()
                            out_bus_range_true = bus_range(out_bus_range)
                            if len(this_bus_range) != len(out_bus_range_true):
                                print "Error! The size of the buses that are to be connected together differ! '%s' vs '%s'" % (str(this_bus_range), str(out_bus_range_true))
                                print "  inside:", inside
                                print "  outside:", outside
                                sys.exit(1)
                            for i in range(len(this_bus_range)):
                                fid.write("%s = BUFF(%s)\n" % (prefix + out_net + "[" + str(out_bus_range_true[i]) + "]", prefix + inst[1] + "\\" + inside + "[" + str(this_bus_range[i]) + "]"))
                        else:
                            for i in this_bus_range:
                                fid.write("%s = BUFF(%s)\n" % (prefix + outside + "[" + str(i) + "]", prefix + inst[1] + "\\" + inside + "[" + str(i) + "]"))
                    else:
                        fid.write("%s = BUFF(%s)\n" % (prefix + outside, prefix + inst[1] + "\\" + inside))

            fid.write("\n")
            handle_module(fid, inst[0], prefix + inst[1] + "\\")
        else:
            #print "   %s is not in our module list, must be a regular gate" % inst[0]
            base_gates.append( ( inst, type) )

    for gate, type in base_gates:
        iscas_type = re.search("([^\d]+).*", gate[0]).group(1).upper()
        if iscas_type == "INV":
            iscas_type = "NOT"
        if iscas_type == "BUF":
            iscas_type = "BUFF"
        if type == "pos":
            inputs = ", ".join([prefix + gate[2][x] for x in range(1, len(gate[2]))])
            #fid.write("# %s - %s\n" % (str(gate), str(type)))
            fid.write("%s = %s(%s)\n" % (prefix + gate[2][0], iscas_type, inputs))
        else:
            inputs = ", ".join([prefix + gate[2][x] for x in gate[2].keys() if x != 'Y'])
            #fid.write("# %s - %s\n" % (str(gate), str(type)))
            fid.write("%s = %s(%s)\n" % (prefix + gate[2]['Y'], iscas_type, inputs))
        
if len(sys.argv) < 4:
    print "Usage: {0} input output top_level_module_name".format(sys.argv[0])
    sys.exit(1)

filename_input = sys.argv[1]
filename_output = sys.argv[2]
top_name = sys.argv[3]

modules = VerilogParser.parse(filename_input)
# each module is a dictionary with these keys:
#   'name'          A string of the module's name
#   'ports'         A list of the input and output nets, so we can make sure to maintain their order
#   'inputs'        A list of (bits, name) tuples, where bits is either "" (non-bus) or something like '3:0' for a bus.
#   'outputs'       Same as above
#   'wires'         Same as above
#   'assigns'       A list of (dest, src) tuples
#   'gate_pos'      Gates using positional notation like "not mynot(out, in);"
#                       Gate format: ('type', 'name', {0: 'netQ', 1: 'netD', 2: 'netClk'}
#   'gate_named'    Gates using named notation like "not mynot(.A(in), .Y(out));"
#                       Gate format: ('type', 'name', {'q': 'netQ', 'd': 'netD', 'clk': 'netClk'}

fid = open(filename_output, "w")
top_module = None
module_names = []
for module in modules:
    module_names.append(module['name'])
    if module['name'] == top_name:
        top_module = module
if not top_module:
    print "Unable to find top-level module from the name you specified, '%s'. Here is the list of modules in that verilog file:" % top_name
    print "    " + ", ".join(module_names)
    sys.exit(1)

# we have identified the top-level module, top_module
fid.write("# %s\n" % top_name)
fid.write("# %s inputs\n" % len(top_module['inputs']))
fid.write("# %s outputs\n" % len(top_module['outputs']))
fid.write("\n")

# deal with the circuit inputs and outputs
io_list = []
for i in top_module['inputs']:
    io_list.append((i, "INPUT"))
for o in top_module['outputs']:
    io_list.append((o, "OUTPUT"))

for io, type in io_list:
    if io[1] == '': # not a bus, just a net
        fid.write("%s(%s)\n" % (type, io[0]))
    else: # a bus
        for i in bus_range(io[1]):
            fid.write("%s(%s[%d])\n" % (type, io[0], i))
fid.write("\n")


# to properly handle the use of the verilog constants 1'b0 and 1'b1, we use XOR/XNOR gates to generate these constants
net_name, net_bus = top_module['inputs'][-1]
if net_bus == "":
    net = net_name
else:
    this_range = bus_range(net_bus)
    net = net_name + "[" + str(this_range[-1]) + "]"

fid.write("# To properly handle 1'b0 / 1'b1, we generate these constants here\n")
fid.write("ZERO = XOR(%s, %s)\n" % (net, net))
fid.write("ONE = XNOR(%s, %s)\n" % (net, net))
fid.write("\n")


# deal with gates and module instantion
handle_module(fid, top_name, "")

fid.close()

