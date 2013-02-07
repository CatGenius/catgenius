using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Text.RegularExpressions;

namespace llc
{
    struct def_t
    {
        public UInt16 value;
        public string source;
    }

    class Program
    {
        static Dictionary<string, def_t> defs = new Dictionary<string, def_t>();

        struct instruction_t
        {
            public byte opcode;
            public UInt16 operand;
        }

        static void ParseDefines(string source_path)
        {
            Regex re = new Regex("^\\s*#define\\s+([^\\s\\/]+)\\s+([0-9]+|0x[0-9A-Fa-f]+)\\b");
            TextReader tr = new StreamReader(source_path);
            MatchCollection matches;
            string line;
            string key;
            string value;
            def_t def;

            def.source = Path.GetFileName(source_path);

            while ((line = tr.ReadLine()) != null)
            {
                matches = re.Matches(line);
                if (matches.Count > 0)
                {
                    key = matches[0].Groups[1].Captures[0].Value;
                    value = matches[0].Groups[2].Captures[0].Value;

                    if (value.StartsWith("0x", StringComparison.CurrentCultureIgnoreCase))
                    {
                        def.value = Convert.ToUInt16(value.Substring(2), 16);
                        //Console.WriteLine("Convert [" + value + "] to [" + nvalue.ToString() + "]");
                    }
                    else
                    {
                        def.value = UInt16.Parse(value);
                    }

                    defs.Add(key, def);
                }
            }

            tr.Dispose();
        }

        static void C2LLP(string source_path, string dest_path)
        {
            TextReader tr = new StreamReader(source_path);
            TextWriter tw = new StreamWriter(dest_path);
            string line;
            string opcode;
            string operand;
            string last_line = "";
            string parsed_line = "";
            int pos;
            bool first_line = true;
            bool in_ifdef = false;

            // TBD: Handle ifdefs and whatnot
            // TBD: Handle comments
            while ((line = tr.ReadLine()) != null)
            {
                line = line.Replace('\t', ' ').Replace(" ", "");

                // Skip over conditional lines
                // TBD: Check defines.  For now we're just ignoring anything between #ifdef and #else or #endif
                if (in_ifdef)
                {
                    in_ifdef = line.StartsWith("#else") || line.StartsWith("#endif");
                    continue;                        
                }
                if (line.StartsWith("#ifdef"))
                {
                    in_ifdef = true;
                    continue;
                }

                if (parsed_line.Length == 0)
                {
                    if (line.StartsWith("conststructinstruction"))
                    {
                        pos = line.IndexOf('[');
                        if (first_line)
                            first_line = false;
                        else
                        {
                            if (last_line.Length > 0)
                            {
                                tw.WriteLine(last_line);
                                last_line = "";
                            }

                            tw.WriteLine();
                        }
                        last_line = line.Substring(22, pos - 22) + ":";
                        continue;
                    }

                    if (!line.StartsWith("{INS_")) continue;
                }

                parsed_line += line;
                pos = parsed_line.IndexOf('}');
                if (pos == -1) continue;

                parsed_line = parsed_line.Substring(5, pos-5);
                string[] s = parsed_line.Split(',');
                opcode = s[0];
                operand = s[1];

                if (operand.StartsWith(opcode + "_"))
                    operand = operand.Substring(opcode.Length + 1);
                else if ((opcode == "PUMP") || (opcode == "DRYER") || (opcode == "WATER"))
                    operand = (operand == "1") ? "ON" : "OFF";
                else if (opcode == "CALL")
                    operand = (operand.Split(')'))[1];
                else if (opcode == "WAITTIME")
                {
                    opcode = "DELAY";
                    operand = (double.Parse(operand) / 1000).ToString();
                }
                else if (opcode.StartsWith("WAIT"))
                {
                    if (opcode == "WAITWATER")
                    {
                        operand = (operand == "1") ? "WATER_HIGH" : "WATER_LOW";
                    }
                    else
                    {
                        operand = opcode.Substring(4);
                        if (operand == "DOSAGE") operand = "DOSE";
                    }
                    opcode = "WAIT";
                }
                else if (opcode == "START")
                    operand = operand.Replace("|", ", ");
                else if ((opcode == "RETURN") || (opcode == "END"))
                    operand = "";

                if (operand.Length == 0)
                    parsed_line = opcode;
                else
                    parsed_line = opcode.PadRight(16) + operand;

                if (last_line.Contains(":") && (opcode == "START"))
                {
                    // Combine label + start
                    if (operand.Contains("FLAGS_WETRUN") && operand.Contains("FLAGS_DRYRUN"))
                        last_line = last_line.PadRight(16) + "WET, DRY";
                    else if (operand.Contains("FLAGS_WETRUN"))
                        last_line = last_line.PadRight(16) + "WET";
                    else if (operand.Contains("FLAGS_DRYRUN"))
                        last_line = last_line.PadRight(16) + "DRY";
                }
                else if (last_line.StartsWith("DELAY ") && opcode.Equals("DELAY"))
                {
                    // Merge DELAYs together
                    last_line = "DELAY".PadRight(16) + (double.Parse(last_line.Substring(6).TrimStart()) + double.Parse(operand)).ToString();
                }
                else
                {
                    if (last_line.Length > 0)
                        tw.WriteLine(last_line);
                    last_line = parsed_line;
                }

                parsed_line = "";
            }

            if (last_line.Length > 0)
                tw.WriteLine(last_line);

            tr.Dispose();
            tw.Dispose();
        }

        static bool ResolveOpcode(string key, out byte opcode)
        {
            def_t def;

            if (!defs.TryGetValue(key, out def))
            {
                opcode = 0;
                return false;
            }

            opcode = (byte)def.value;
            return true;
        }

        static bool ResolveOperand(string key, out UInt16 operand)
        {
            def_t def;

            if (!defs.TryGetValue(key, out def))
            {
                operand = 0;
                return false;
            }

            operand = def.value;
            return true;
        }

        static void WriteInstruction(FileStream fo, ref UInt16 pc, instruction_t inst)
        {
            byte[] buf = {inst.opcode, (byte)((inst.operand & 0xFF00) >> 8), (byte)(inst.operand & 0xFF)};
            fo.Write(buf, 0, 3);
            pc += 3;
        }

        static void LogError(string source_path, UInt16 line_no, string line, string error)
        {
            Console.WriteLine(Path.GetFileName(source_path) + "(" + line_no + 1 + "): Error: " + error);
            Console.WriteLine(line);
        }

        static void LLP2BIN(string source_path, string dest_path)
        {
            Regex re_label = new Regex("^([A-Za-z0-9_\\-]+):\\s*(?:([A-Za-z0-9_\\-]+)(?:\\s*,\\s*){0,1})*$");
            Regex re_inst = new Regex("^([A-Za-z0-9_\\-]+)\\s*(?:([A-Za-z0-9_\\-\\.]+)(?:\\s*,\\s*){0,1})*$");
            Dictionary<string, UInt16> labels = new Dictionary<string, UInt16>();
            TextReader tr = new StreamReader(source_path);
            FileStream fo = new FileStream(dest_path, FileMode.Create);
            string all = tr.ReadToEnd();
            StringBuilder sb = new StringBuilder();
            int start, pos1, pos2;
            tr.Dispose();            
            
            //--------
            // Step 1: Remove /* ... */ comments
            //--------

            start = 0;
            while (true)
            {
                pos1 = all.IndexOf("/*", start);
                if (pos1 == -1)
                {
                    sb.Append(all.Substring(start));
                    break;
                }
                
                pos2 = all.IndexOf("*/", pos1 + 2);
                if (pos2 == -1)
                {
                    Console.WriteLine("Error: Unterminated comment line");
                    return;
                }

                sb.Append(all.Substring(start, pos1 - start));
                start = pos2 + 2;
            }
            all = sb.ToString();

            //--------
            // Step 2: Pre-scan for labels, uppercase everything, remove //-style comments
            //--------
            
            // TBD

            //--------
            // Step 3: Parse/translate instructions
            //--------

            UInt16 line_no;
            string line;
            string[] lines = all.Split('\n');
            Match match;

            byte INS_START;                 ResolveOpcode   ("INS_START",       out INS_START       );
            byte INS_END;                   ResolveOpcode   ("INS_END",         out INS_END         );
            byte INS_WAITTIME;              ResolveOpcode   ("INS_WAITTIME",    out INS_WAITTIME    );
            UInt16 FLAGS_DRYRUN;            ResolveOperand  ("FLAGS_DRYRUN",    out FLAGS_DRYRUN    );
            UInt16 FLAGS_WETRUN;            ResolveOperand  ("FLAGS_WETRUN",    out FLAGS_WETRUN    );

            instruction_t inst;
            string opcode;
            string operand;
            UInt16 pc;

            pc = 0;
            for (line_no = 0; line_no < lines.Length; line_no++)
            {
                // Uppercase & Remove whitespace
                // TBD: line_orig?
                line = lines[line_no].ToUpper().Trim();

                // Remove //-style comments
                if ((pos1 = line.IndexOf("//")) != -1) line = line.Substring(pos1).Trim();  // TBD: Test

                // Skip over empty lines
                if (line.Length == 0) continue;

                // Check for label
                if ((match = re_label.Match(line)).Length > 0)
                {
                    string label = match.Groups[1].Captures[0].Value;
                    UInt16 label_pc;

                    if (labels.TryGetValue(label, out label_pc))
                    {
                        LogError(source_path, line_no, line, "Label already defined @ pc=" + label_pc.ToString());
                        goto fail;
                    }

                    labels.Add(label, pc);
                    CaptureCollection args = match.Groups[2].Captures;
                    
                    if (args.Count > 1)
                    {
                        // If there are args, we have to write an INS_START instruction

                        inst.opcode = INS_START;
                        inst.operand = INS_END;

                        for (int i = 0; i < args.Count; i++)
                        {
                            if (args[i].Value == "WET")
                            {
                                inst.operand |= FLAGS_WETRUN;
                            }
                            else if (args[i].Value == "DRY")
                            {
                                inst.operand |= FLAGS_DRYRUN;
                            }
                        }

                        WriteInstruction(fo, ref pc, inst);
                    }

                    continue;
                }

                // Check for instructon + argument line
                if ((match = re_inst.Match(line)).Length > 0)
                {
                    opcode = match.Groups[1].Captures[0].Value;
                    CaptureCollection args = match.Groups[2].Captures;

                    // WAIT: Convert into WAIT##x instructions
                    if (opcode == "WAIT")
                    {
                        if (args.Count < 1)
                        {
                            LogError(source_path, line_no, line, "One or more operands required");
                            break;
                        }

                        char[] sep = {'_'};
                        for (int i = 0; i < args.Count; i++)
                        {
                            // Opcode will be WATER_ON for example, so translate into {INS_WAITWATER, WATER_ON (1)}
                            string[] a = args[i].Value.Split(sep, 2);

                            // Allow DOSE as well as DOSAGE
                            if (a[0] == "DOSE") a[0] = "DOSAGE";

                            if (!ResolveOpcode("INS_WAIT" + a[0], out inst.opcode))
                            {
                                LogError(source_path, line_no, line, "Unknown operand [" + args[i].Value + "]");
                                goto fail;
                            }

                            if (!ResolveOperand(args[i].Value, out inst.operand))
                            {
                                LogError(source_path, line_no, line, "Unknown operand value [" + args[i].Value + "]");
                                goto fail;
                            }

                            WriteInstruction(fo, ref pc, inst);                            
                        }

                        continue;
                    }

                    // DELAY: Convert to INS_WAITTIME, Convert from s to ms, break up into multiple 65535ms instructions
                    if (opcode == "DELAY")
                    {
                        inst.opcode = INS_WAITTIME;
                        if (args.Count != 1)
                        {
                            LogError(source_path, line_no, line, ((args.Count == 0) ? "Operand required" : "Too many operands"));
                            break;
                        }

                        double delay = double.Parse(args[0].Value) * 1000;
                        if (delay > 65535)
                        {
                            inst.operand = 65535;
                            while (delay > inst.operand)
                            {
                                WriteInstruction(fo, ref pc, inst);
                                delay -= inst.operand;
                            }
                        }
                        inst.operand = (UInt16)delay;
                        WriteInstruction(fo, ref pc, inst);
                        continue;
                    }

                    // Look up opcode #define
                    if (!ResolveOpcode("INS_" + opcode, out inst.opcode))
                    {
                        LogError(source_path, line_no, line, "Unknown opcode");
                        break;
                    }

                    // END/RETURN: No operand
                    if ((opcode == "END") || (opcode == "RETURN"))
                    {
                        if (args.Count > 0)
                        {
                            LogError(source_path, line_no, line, "Operand not allowed");
                            break;
                        }

                        inst.operand = 0;
                        WriteInstruction(fo, ref pc, inst);
                        continue;
                    }

                    // All the rest of the opcodes require exactly one argument
                    if (args.Count != 1)
                    {
                        LogError(source_path, line_no, line, ((args.Count == 0) ? "Operand required" : "Too many operands"));
                        break;
                    }
                    operand = args[0].Value;

                    // Special case: CALL - We need to look up the label
                    if (opcode == "CALL")
                    {
                        UInt16 jump_pc;

                        if (!labels.TryGetValue(operand, out jump_pc))
                        {
                            LogError(source_path, line_no, line, "Label not found [" + operand + "]");
                            break;
                        }

                        inst.operand = jump_pc;
                        WriteInstruction(fo, ref pc, inst);
                        continue;
                    }

                    // AUTODOSE/SKIPIFDRY - Always a numeric value
                    if ((opcode == "AUTODOSE") || (opcode == "SKIPIFDRY"))
                    {
                        inst.operand = UInt16.Parse(operand);
                        WriteInstruction(fo, ref pc, inst);
                        continue;
                    }

                    // Everything else uses defines
                    if (ResolveOperand(opcode + "_" + operand, out inst.operand))
                    {
                        WriteInstruction(fo, ref pc, inst);
                        continue;
                    }
                    LogError(source_path, line_no, line, "Unknown operand value [" + operand + "] for opcode [" + opcode + "]");

                    break;
                }

                // Don't know what this line is
                LogError(source_path, line_no, line, "Malformed line");
                break;
            }

            Console.WriteLine("Compiled " + lines.Length + " lines into " + (pc/3) + " instructions (" + pc + " bytes)");

fail:
            fo.Dispose();
        }

        static void BIN2C(string source_path, string dest_path)
        {
            FileStream fi = new FileStream(source_path, FileMode.Open);
            TextWriter fo = new StreamWriter(dest_path);
            byte[] buf = new byte[3];
            instruction_t inst;
            string opcode;
            string operand;            
            UInt16 pc;
            char[] sep = {'_'};
            string[] a;

            byte INS_START;                 ResolveOpcode   ("INS_START",       out INS_START       );
            byte INS_END;                   ResolveOpcode   ("INS_END",         out INS_END         );
            UInt16 FLAGS_DRYRUN;            ResolveOperand  ("FLAGS_DRYRUN",    out FLAGS_DRYRUN    );
            UInt16 FLAGS_WETRUN;            ResolveOperand  ("FLAGS_WETRUN",    out FLAGS_WETRUN    );

            fo.WriteLine("#include \"" + Path.GetFileNameWithoutExtension(source_path) + ".h\"");
            fo.WriteLine();
            fo.WriteLine("const struct instruction clean_program = {");
            
            pc = 0;
            while (fi.Read(buf, 0, 3) == 3)
            {
                inst.opcode = buf[0];
                inst.operand = buf[1];
                inst.operand <<= 8;
                inst.operand |= buf[2];

                // Lookup opcode
                opcode = null;
                foreach (KeyValuePair<string, def_t> kvp in defs) if ((kvp.Value.source != null) && (kvp.Value.value == inst.opcode) && kvp.Key.StartsWith("INS_")) { opcode = kvp.Key; break; }
                if (opcode == null)
                {
                    Console.WriteLine("(pc=" + pc.ToString() + ") Error: Opcode not recognized [" + inst.opcode +"]");
                    goto fail;
                }

                // Lookup operand
                a = opcode.Split(sep, 2);
                if (inst.opcode == INS_START)
                {
                    operand = "";

                    // TBD: Support additional attribs here
                    if ((inst.operand & INS_END) > 0) operand += "INS_END | ";
                    if ((inst.operand & FLAGS_DRYRUN) > 0) operand += "FLAGS_DRYRUN | ";
                    if ((inst.operand & FLAGS_WETRUN) > 0) operand += "FLAGS_WETRUN | ";

                    if (operand.Length > 0) operand = operand.Substring(0, operand.Length - 3);
                }
                else
                {
                    operand = inst.operand.ToString();
                    foreach (KeyValuePair<string, def_t> kvp in defs) if ((kvp.Value.source != null) && (kvp.Value.value == inst.operand) && kvp.Key.StartsWith(a[1] + '_')) { operand = kvp.Key; break; }
                }

                fo.WriteLine("".PadRight(8) + ("/* " + pc.ToString().PadLeft(4, '0') + " */").PadRight(16) + ("{" + opcode + ",").PadRight(16) + operand + "},");
                
                pc += 3;
            }

            fo.WriteLine("".PadRight(24) + "{0x00,".PadRight(16) + "0x0000}");
            fo.WriteLine("};");

fail:
            fo.Dispose();
            fi.Dispose();
        }

        static void AddCustomDef(string key, UInt16 value)
        {
            def_t def;

            if (defs.ContainsKey(key)) return;

            def.source = null;
            def.value = value;
            defs.Add(key, def);
        }

        static void Main(string[] args)
        {
            string m_AppPath;
            string m_SoftwarePath;
            string m_CGPath;
            string m_CommonPath;

            m_AppPath = AppDomain.CurrentDomain.BaseDirectory;
            
            // Resolve paths
            Console.WriteLine("* Resolving paths");
            m_SoftwarePath = Path.GetFullPath(m_AppPath + "\\..\\..\\..\\..\\software");
            m_CGPath = m_SoftwarePath + "\\catgenius";
            m_CommonPath = m_SoftwarePath + "\\common";

            // Load defines
            Console.WriteLine("* Loading defines"); 
            ParseDefines(m_CGPath + "\\litterlanguage.h");
            ParseDefines(m_CommonPath + "\\catgenie120.h");

            // Set up some defs of our own
            AddCustomDef("PUMP_ON", 1);
            AddCustomDef("PUMP_OFF", 0);
            AddCustomDef("DRYER_ON", 1);
            AddCustomDef("DRYER_OFF", 0);

            // WAIT intructions
            AddCustomDef("WATER_ON", 1);
            AddCustomDef("WATER_OFF", 0);
            AddCustomDef("WATER_HIGH", 1);
            AddCustomDef("WATER_LOW", 0);
            AddCustomDef("DOSAGE", 0);
            AddCustomDef("DOSE", 0);

            // Convert C to LLP
            Console.WriteLine("* Converting C to LLP");
            C2LLP(m_CGPath + "\\romwashprogram.c", m_AppPath + "\\clean_default.llp");

            // Convert LLP to BIN
            Console.WriteLine("* Converting LLP to BIN");
            LLP2BIN(m_AppPath + "\\clean_default.llp", m_AppPath + "\\clean_default.bin");

            // Convert BIN to HEX?

            // Convert BIN to C
            Console.WriteLine("* Converting BIN to C");
            BIN2C(m_AppPath + "\\clean_default.bin", m_AppPath + "\\clean_default.c");

            /*
            foreach (KeyValuePair<string, UInt16> kvp in defs)
            {
                Console.WriteLine(kvp.Key + "=[" + kvp.Value + "]");
            }
             */

            //Console.ReadKey();
        }
    }
}
