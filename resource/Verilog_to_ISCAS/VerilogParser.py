#!/usr/bin/env python
#
# Verilog Parser - Reads and parses a verilog file. Stores the design info
# in a dictionary-per-module, with a few fields within, such as for inputs,
# outputs, wires, assigns, and both named- and positionally-instantiated 
# gates/modules. Please let me know if you run across a verilog file that
# doesn't parse properly!
#
# TODO list:
#   Standardize handling of bus vs not-bus, perhaps default to buses, and offer a conversion function to switch to not-bus?
#
# Usage:
# import VerilogParser
# my_modules_list = VerilogParser.parse("file.v")
#
# Look at the end of this file for more usage examples
#
# Version 1.05 - Last modified November 29, 2011
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

import sys, re, operator

primitive_gates = ["and", "nand", "or", "nor", "xor", "xnor", "xorp", "xnorp", "not", "inv", "buf", "buff"]
primitive_gates_ports = ["Y", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V"]

def _parse_iow(line):
    """ Parses an input, output, or wire line. Returns a list of (net_name, size), where size is like "[7:0]" or "[0:7]" (we don't care about the order) for a bus, or "" for a single net."""
    if re.search("^\w+\s*\[", line):
        # bus definition
        raw_bits, raw_names = re.search("\w+\s*\[([^\[]*)\]\s*(.*)\s*$", line).groups()
        names = map(str.strip, raw_names.strip().split(",")) # the names of the buses involved in this definition
        retval = []
        for name in names:
            retval.append( (name, " [%s]" % raw_bits) )
        return retval
    else:
        # single definition
        raw_names = re.search("\w+\s*(.*)\s*$", line).group(1)
        names = map(str.strip, raw_names.strip().split(","))
        retval = [(n, "") for n in names]
        return retval

def _parse_module(line):
    """ Parses a module definition line, returns the module name and a list of the module ports. """
    mod_name, mod_ports = re.search("module\s*([^\s\(]*)\s*\(\s*(.*)\s*\).*", line).groups()
    mod_ports.strip() # if there are trailing spaces inside the parentheses, this will help clean them out
    mod_ports = map(str.strip, mod_ports.split(","))
    return mod_name, mod_ports

def _parse_assign(line):
    """ Parses an "assign" line. Returns a list of (lhs, rhs) tuples. """
    assign, remainder = re.search("(assign)\s+(.*)", line).groups()
    assigns = []
    while len(remainder) > 0:
        lhs, rest = re.search("([^=]*?)\s*=\s*(.*)", remainder).groups()

        if ":" in lhs: # bus definition
            lhs = re.search("(.*)\s*\[(.*)\]", lhs).groups()
        else:
            lhs = (lhs, "")

        if rest.startswith("{"):
            # using concatenation
            rhs, remainder = re.search("(\{\s*[^\}]*\s*\}),?\s*(.*)", rest).groups()
            rhs = (rhs, "")
        else:
            # just a regular net
            rhs, remainder = re.search("([^,]*),?\s*(.*)", rest).groups()

            if ":" in rhs: # bus definition
                rhs = re.search("(.*)\s*\[(.*)\]", rhs).groups()
            else:
                rhs = (rhs, "")

        #print "   '%s' = '%s'" % (lhs, rhs)
        assigns.append( (lhs, rhs) )

    return assigns

def _parse_instantiation(line):
    """ Parses an instantation line (module or gate). Returns a list of (type, instantiation name, io list). """
    # There can be multiple instantiations per line, unfortunately:
    # nand2 Xo2( .A(NotA), .B(B), .Y(line2) ), Xo3( .A(NotB), .B(A), .Y(line3) ), Xo4( .A(line2), .B(line3), .Y(Y) )
    type, remainder = re.search("([^\s]+)\s+(.*)", line).groups()
    instantiations = []
    while len(remainder) > 0:
        iolist = {}
        if re.search(".*\([^\)]*\(", remainder):
            # using named assignment
            iname, raw_iolist, remainder = re.search("([^\s\(]+)\s*\(\s*(.*?\)\s*?)\s*\)\s*,?\s*(.*)", remainder).groups()
            while len(raw_iolist) > 0:
                # we actually don't have to deal with concatenations here, since .A(foo) always catches foo correctly (as far as I have seen)
                name, net, raw_iolist = re.search("\.([^\)]*)\s*\(\s*([^\)]*)\s*\)\s*,?\s*(.*)", raw_iolist).groups()
                iolist[name] = net
        else:
            # using positional assignment
            iname, raw_iolist, remainder = re.search("([^\s\(]+)\s*\(\s*(.*?)\s*\)\s*,?\s*(.*)", remainder).groups()
            pos = 0
            while len(raw_iolist) > 0:
                if raw_iolist.startswith("{"):
                    # this one is a concatenation
                    net, raw_iolist = re.search("(\{.*?\})\s*,\s*(.*)", raw_iolist).groups()
                    iolist[pos] = net
                else:
                    # just a regular net
                    net, raw_iolist = re.search("([^,\s]*)\s*,?\s*(.*)", raw_iolist).groups()
                    iolist[pos] = net
                pos += 1

        instantiations.append( (type, iname, iolist) )

    return instantiations

def make_empty_module(name):
    this_mod = {}
    this_mod['name'] = name
    this_mod['ports'] = []
    this_mod['inputs'] = []
    this_mod['outputs'] = []
    this_mod['wires'] = []
    this_mod['assigns'] = []
    # these next two are for instantiations (gates or modules)
    # '_pos' is for unnamed instantiations, that are positional:
    #       "not1 mynot(out, in);"
    # '_named' is for .clk(foo) style named instantiations
    #       "not1 mynot(.A(in), .Y(out));"
    this_mod['gate_pos'] = []
    this_mod['gate_named'] = []
    return this_mod

def parse(filename, dump_cleaned = False):
    # read in the file
    verilog = []
    for line in file(filename):
        line = line.strip()
        if not line:
            continue
        if line.startswith("`"):
            continue
        if line.startswith("//"):
            continue
        if "//" in line:
            line = line[:line.find("//")]
        verilog.append(line)
    
    # get rid of multiple-line statements and comment lines
    verilog = " ".join([x for x in verilog if not x.startswith("//")]) # TODO do we still need this // check if we have probably handled it above?
    # get rid of multi-line comments
    verilog = re.sub("\/\*.*?\*\/", "", verilog)
    # split the text into statements
    verilog = verilog.split(";")
    verilog = map(str.strip, verilog)

    # since verilog doesn't require a semicolon after 'endmodule', it gets joined with the next line
    # so we have to split it up manually
    changed = True # since we are changing the verilog list, we want to restart the search each time we change something
    while changed:
        changed = False
        for ix in range(len(verilog)):
            if verilog[ix].startswith("endmodule"):
                # there are lots of lines that start with 'endmodule', but not all (like previously-converted ones) have a module (foo) part too
                match = re.search("module\s+(.*)", verilog[ix])
                if match:
                    nextline = match.group(1)
                    verilog[ix] = "endmodule"
                    if ix < len(verilog):
                        verilog.insert(ix + 1, nextline)
                    else:
                        verilog.append(nextline)
                    changed = True
                    break # start over in the outer while loop

    if dump_cleaned:
        # here we can dump the raw, cleaned verilog text to file, for debugging purposes:
        # sort of like looking at the output of a compiler's preprocessor
        with open(filename + ".cleaned.v", "w") as fid:
            fid.write("\n".join(verilog))

    modules = []
    this_mod = None

    for line in verilog:
        line = line.strip()
        if line.startswith("module") and this_mod == None:
            name, ports = _parse_module(line)
            this_mod = {}
            this_mod['name'] = name
            this_mod['ports'] = ports
            this_mod['inputs'] = []
            this_mod['outputs'] = []
            this_mod['wires'] = []
            this_mod['assigns'] = []
            # these next two are for instantiations (gates or modules)
            # '_pos' is for unnamed instantiations, that are positional:
            #       "not1 mynot(out, in);"
            # '_named' is for .clk(foo) style named instantiations
            #       "not1 mynot(.A(in), .Y(out));"
            this_mod['gate_pos'] = []
            this_mod['gate_named'] = []
        elif line.startswith("endmodule"):
            modules.append(this_mod)
            this_mod = None
        elif line.startswith("input"):
            list = _parse_iow(line)
            this_mod['inputs'].extend(list)
        elif line.startswith("output"):
            list = _parse_iow(line)
            this_mod['outputs'].extend(list)
        elif line.startswith("wire"):
            list = _parse_iow(line)
            this_mod['wires'].extend(list)
        elif line.startswith("assign"):
            list = _parse_assign(line)
            this_mod['assigns'].extend(list)
        else:
            instantiations = _parse_instantiation(line)
            for gtype, instantiation_name, iolist in instantiations:
                if 0 in iolist.keys():
                    # unnamed - positional instantiation
                    this_mod['gate_pos'].append( (gtype, instantiation_name, iolist) )
                else:
                    # named instantiation
                    this_mod['gate_named'].append( (gtype, instantiation_name, iolist) )

    return modules

def is_true_primitive(gate_name):
    return gate_name.lower() in primitive_gates
def is_primitive(gate_name):
    return reduce(operator.or_, [gate_name.lower().startswith(x) for x in primitive_gates])

def convert_primitives(modules):
    """ This function converts raw (true) primitives (ie and, buf, inv) into their cell library forms (and2/3/4, buf1, inv1, etc). Only adjusts named-position gates! """
    #print "--- Renaming primitives to cell library primitives ---"
    num_converted = 0
    for mod in modules:
        #print "--- Renaming primitives in module '%s' to cell library primitives ---" % mod['name']
        for gate in mod['gate_named']:
            if is_true_primitive(gate[0]):
                num_converted += 1
                if gate[0] in ["buf", "not", "inv", "buff"]:
                    # just append a 1 to the name
                    new_type = gate[0] + "1"
                else: # must be a larger gate like and, nand, or, nor, xor, etc - we should append the fan-in of the gate
                    new_type = gate[0] + str(len(gate[2]) - 1)
                new_gate = (new_type, gate[1], gate[2])
                mod['gate_named'].remove(gate)
                mod['gate_named'].insert(0, new_gate)
    return num_converted

def convert_to_scan(modules):
    """ This function converts a sequential (DFF-based) design to a "scan-based" design by removing the DFFs and making the DFF's IO lines into module IO as required. """
    # Rules for converting a DFF into extra inputs and outputs:
    # Input to DFF is called D, output is Q (per usual)
    # 
    # 1. Add Q to inputs
    # 2. If Q is in the outputs, remove it from the outputs
    # 3. If D is not in the inputs, add D to the outputs

    count = 0
    for mod in modules:

        new_gate_named = [] # This will be our new list of gate_named gates, minus all the DFFs
        # I guess we are only looking at gate_named for now...
        for gate in mod['gate_named']:
            if "latch" in gate[0]:
                print "Error: We don't know how to handle latches!"
                sys.exit(1)
            elif "dff" not in gate[0]:
                new_gate_named.append(gate)
            else:
                count += 1

                d_raw = gate[2]['d'] if ('d' in gate[2]) else gate[2]['D']
                d = (d_raw, "")
                # if D not in inputs: add D to outputs
                if d not in mod['inputs'] and d[0] != "1'b1" and d[0] != "1'b0" and d not in mod['outputs']:
                    #print "    D %s is not already an input, adding to outputs..." % str(d)
                    mod['outputs'].append(d)
                    mod['ports'].append(d_raw)


                # For cleanliness sake, remove new IO lines from wires
                if d in mod['wires']:
                    mod['wires'].remove(d)

                q_raw = gate[2]['q'] if ('q' in gate[2]) else gate[2]['Q']
                q = (q_raw, "")
                # Add Q to the inputs
                #print "    Q \"%s\" is being added to inputs..." % q
                mod['inputs'].append(q)
                mod['ports'].append(q_raw)

                # If Q in outputs, remove from outputs
                if q in mod['outputs']:
                    #print "    Q \"%s\" is in outputs, removing..." % q
                    mod['outputs'].remove(q)
                    mod['ports'].remove(q_raw)

                # For cleanliness sake, remove new IO lines from wires
                if q in mod['wires']:
                    mod['wires'].remove(q)

        mod['gate_named'] = new_gate_named

    #print "Removed %d DFFs" % count
    return count


def convert_to_named_ports(modules):
    """ This function converts the instantiations of gates and modules from positional to named. Modifies the module list in place! """
    # For this example, we want to convert all the gates in gate_pos to be in gate_named
    # We assume that the order of inputs is Y, A, B, C, D, E,...., that is, the output is first
    # We will only do this automatic conversion for the basic gates and, nand, or, nor, xor, xnor, inv, not, buf
    #print "--- Converting module gates to named-instantiation ---"
    num_pos = 0
    for mod in modules:
        #print "--- Converting module '%s' to named-instantiation ---" % mod['name']
        num_pos += len(mod['gate_pos'])
        while len(mod['gate_pos']) > 0:
            gate = mod['gate_pos'][0]
            #if reduce(operator.or_, [gate[0].startswith(x) for x in primitive_gates]):
            if is_primitive(gate[0]):
                # convert and add to gate_named - type and name are the same
                connections = {}
                for index, net in gate[2].iteritems():
                    connections[primitive_gates_ports[index]] = net
                new_gate = (gate[0], gate[1], connections)
                mod['gate_named'].append(new_gate)
                mod['gate_pos'].pop(0)
            else:
                #print "gate %s not an easy gate, skipping..." % gate[0]
                if gate[0] in [x['name'] for x in modules]:
                    thismod_index = [x['name'] for x in modules].index(gate[0])
                    #print "  ...but it's in our module list, at index %d, hurray!" % thismod_index
                    thismod = modules[[x['name'] for x in modules].index(gate[0])]
                    connections = {}
                    for index, net in gate[2].iteritems():
                        connections[thismod['ports'][index]] = net
                    new_gate = (gate[0], gate[1], connections)
                    mod['gate_named'].append(new_gate)
                    mod['gate_pos'].pop(0)
                else:
                    print "Gate '%s' is not a basic gate, but it's not in our module list, weird...exiting!"
                    sys.exit(1)
    return num_pos

def bus_range(bus_range_string):
    """ Creates an iterable range based on the bus string, something like " [2:0]". """
    s = bus_range_string.strip()
    s = s[1:-1]
    a, b = map(int, s.split(":"))
    if a < b:
        return range(a, b + 1)
    else:
        return range(a, b - 1, -1)

def convert_to_no_buses_iow(pair_list):
    new_items = []
    for name, bus in pair_list:
        if bus == "":
            if "[" in name:
                new_items.append( ("_".join(re.search("([^\[]+)\[([^\]]+)\]", name).groups()), "") )
            else:
                new_items.append( (name, "") )
        else:
            for i in bus_range(bus):
                new_items.append( (name + "_" + str(i), "") )
    return new_items

def regularize_assign_side(p):
    if p[1] == "":
        if "[" in p[0]:
            return "_".join(re.search("([^\[]+)\[([^\]]+)\]", p[0]).groups())
        else:
            return p[0]
    else:
        if "[" in p[0]:
            print "Weird, this assign is a bus but also has '[' in the net name, bailing...",  str(p)
            sys.exit(1)
        else:
            new_items = []
            for i in bus_range(p[1]):
                new_items.append(p[0] + "_" + str(i))
            return new_items

def convert_assignments(gates):
    new_gates = []
    for gtype, name, cons in gates:
        new_cons = {}
        for port_name, net in cons.iteritems():
            # super huge assumption here! we assume that there are no sub-modules since this un-busing messes up the instantiations involving buses - FIXME
            if "[" in net:
                new_cons[port_name] = "_".join(re.search("([^\[]+)\[([^\]]+)\]", net).groups())
            else:
                new_cons[port_name] = net
        new_gates.append( (gtype, name, new_cons) )
    return new_gates

def convert_to_no_buses(modules):
    """ This function goes through all the inputs, outputs, ports, assigns, wires, and named instantiations to "expand" the bus notation to make scan easier. """
    for mod in modules:

        mod['inputs'] = convert_to_no_buses_iow(mod['inputs'])
        mod['outputs'] = convert_to_no_buses_iow(mod['outputs'])
        mod['wires'] = convert_to_no_buses_iow(mod['wires'])

        new_pairs = []
        for lhs, rhs in mod['assigns']:
            lhs_items = regularize_assign_side(lhs)
            rhs_items = regularize_assign_side(rhs)
            if type(lhs_items) == type([]) and type(rhs_items) == type([]): # two buses
                if len(lhs_items) != len(rhs_items):
                    print "Error! We had an assign with two buses that are different sizes, bailing..."
                    print "'%s' = '%s'" % (str(lhs), str(rhs))
                    sys.exit(1)
                for i in range(len(lhs_items)):
                    new_pairs.append( ((lhs_items[i], ""), (rhs_items[i], "")) )
            elif type(lhs_items) == type("asdf") and type(rhs_items) == type("asdf"): # two non-buses
                new_pairs.append( ((lhs_items, ""), (rhs_items, "")) )
            else:
                print "Error! We had an assign with one bus and one non-bus, bailing..."
                print "'%s' = '%s'" % (str(lhs), str(rhs))
                sys.exit(1)
        mod['assigns'] = new_pairs

        # regenerate the ports from the new list of inputs and outputs
        mod['ports'] = []
        mod['ports'].extend(map(operator.itemgetter(0), mod['outputs']))
        mod['ports'].extend(map(operator.itemgetter(0), mod['inputs']))

        # deal with assignments
        mod['gate_named'] = convert_assignments(mod['gate_named'])
        mod['gate_pos'] = convert_assignments(mod['gate_pos'])


def convert_to_realistic_fanin(modules):
    """ This function converts all the by-name-instantiated gates to have realistic fan-in values. Modifies the module list in place! TODO is this working? """
    print "WARNING - This code is DUBIOUS at best, and probably NOT WORKING AT ALL."
    gate_sizes = ["and", "nand", "or", "nor", "xor", "xnor"]
    for mod in modules:
        print "--- Fixing fan-in for module '%s' ---" % mod['name']
        if len(mod['gate_pos']) > 0:
            print "Module '%s' has positionally-instantiated gates/modules! This function only works on named-instantiated gates."
            print "   Since this might not matter, we are continuing, but beware! Things might be messed up, or not fully converted."
        if len(mod['gate_named']) > 0:
            finished = False
            while not finished:
                finished = True
                for gate in mod['gate_named']:
                    gate_name_pure = None
                    for g in gate_sizes:
                        if gate[0].startswith(g):
                            gate_name_pure = g
                    print "gate_name_pure:", gate_name_pure
                    if gate_name_pure is not None:
                        print "we can convert this gate"
                        new_gates = []
                        inputs = [gate[2][x] for x in gate[2] if x not in ('Y', "Z", "ZN")]
                        print "  inputs:", inputs
                        if gate_name_pure in ("and", "or"):
                            if len(inputs) < 4:
                                print "  gate is small enough"
                            else:
                                print "  gate needs to be broken up for smaller fan-in"

                        mod['gate_named'].remove(gate)
                        mod['gate_named'].extend(new_gates)
                        finished = False
                    else:
                        # gate is not eligible to be converted, just skip it
                        pass


def write_verilog(modules, output_filename):
    """ Writes a list of modules back into a verilog file, with clean and consistent formatting. """
    # tmax has a line length limit of 50000 or something, but let's make nicely human readable files
    maxlen = 100
    fid = file(output_filename, "w")
    for m in modules:
        start = "module %s (" % m['name']
        fid.write(start)
        linelen = len(start)
        for name in m['ports'][:-1]: # save the last one since it doesn't get a comma after it
            if linelen + len(name) >= maxlen:
                fid.write("\n    ")
                linelen = 4
            fid.write(name + ", ")
            linelen += len(name) + 2
        # deal with the last arg here, different than the rest (no trailing comma)
        last_port = m['ports'][-1]
        if linelen + len(last_port) >= maxlen:
            fid.write("\n    ")
        fid.write(m['ports'][-1] + ");\n")

        fid.write("\n")

        for name, bus in m['inputs']:
            fid.write("    input%s %s;\n" % (bus, name))
        fid.write("\n")

        for name, bus in m['outputs']:
            fid.write("    output%s %s;\n" % (bus, name))
        fid.write("\n")

        if len(m['wires']) > 0:
            for name, bus in m['wires']:
                fid.write("    wire%s %s;\n" % (bus, name))
            fid.write("\n")

        if len(m['assigns']) > 0:
            for lhs, rhs in m['assigns']:
                fid.write("    assign %s = %s;\n" % ("".join(lhs), "".join(rhs)))
            fid.write("\n")


        if len(m['gate_pos']) > 0:
            for gp in m['gate_pos']:
                cons = []
                for index in range(len(gp[2])):
                    cons.append(gp[2][index])
                fid.write("    %s %s(%s);\n" % (gp[0], gp[1], ", ".join(cons)))
            fid.write("\n")

        if len(m['gate_named']) > 0:
            for gn in m['gate_named']:
                cons = []
                # we want to put the output connections first, for increased compatability with Osei's v2i script
                if is_primitive(gn[0]):
                    if "Y" in gn[2]:
                        cons.append(".Y(%s)" % (gn[2]['Y']))
                    if "Z" in gn[2]:
                        cons.append(".Z(%s)" % (gn[2]['Z']))
                    if "ZN" in gn[2]:
                        cons.append(".ZN(%s)" % (gn[2]['ZN']))
                    for c in gn[2]:
                        if c not in ("Y", "Z", "ZN"):
                            cons.append(".%s(%s)" % (c, gn[2][c]))
                else: # this is a module instantiation
                    the_mod = None
                    for mod in modules:
                        if mod['name'] == gn[0]:
                            the_mod = mod
                            break
                    if not the_mod:
                        print "weird, a non-primitive module wasn't found: '%s'" % gn[0]
                        sys.exit(1)
                    for out in the_mod['outputs']:
                        cons.append(".%s(%s)" % (out[0], gn[2][out[0]]))
                    for inp in the_mod['inputs']:
                        cons.append(".%s(%s)" % (inp[0], gn[2][inp[0]]))
                fid.write("    %s %s(%s);\n" % (gn[0], gn[1], ", ".join(cons)))
            fid.write("\n")

        fid.write("endmodule /* %s */\n\n" % m['name'])
    fid.close()

def main():
    import sys
    if len(sys.argv) < 2:
        print "Usage: %s verilog_filename" % sys.argv[0]
        print "Running this script will parse a verilog file, report any errors it encounters during parsing, print out the internal representation of the modules, and then write it out to a different filename."
        sys.exit(1)
    filename = sys.argv[1]
    
    modules = parse(filename)

    for m in modules:
        print m['name']
        print m['ports']
        print m['inputs']
        print m['outputs']
        print m['wires']
        print m['assigns']
        print m['gate_pos']
        print m['gate_named']
        print ""


    write_verilog(modules, filename + ".output.v")



if __name__ == "__main__":
    main()
